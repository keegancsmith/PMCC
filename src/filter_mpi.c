#include "image.h"
#include "filter_mpi.h"

#include <mpi.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void LOG(const char* format, ...) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    fprintf(stderr, "%d : ", rank);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
    fflush(stderr);
}


filter_t get_filter() {
    filter_t filter;
    int buf[2];
    int h;

    MPI_Bcast(buf, 2, MPI_INT, 0, MPI_COMM_WORLD);
    filter.width  = buf[0];
    filter.height = buf[1];

    filter.filter = calloc(filter.height, sizeof(float*));
    for (h = 0; h < filter.height; h++) {
        filter.filter[h] = calloc(filter.width, sizeof(float));
        MPI_Bcast(filter.filter[h], filter.width, MPI_FLOAT,
                  0, MPI_COMM_WORLD);
    }

    return filter;
}


void send_filter(const filter_t * filter) {
    int h;
    int buf[2] = {filter->width, filter->height};
    MPI_Bcast(buf, 2, MPI_INT, 0, MPI_COMM_WORLD);

    for (h = 0; h < filter->height; h++)
        MPI_Bcast(filter->filter[h], filter->width, MPI_FLOAT,
                  0, MPI_COMM_WORLD);
}


unsigned char * filter_channel(const job_t * job,
                               const filter_t * filter,
                               int channel) {
    int offset_r = filter->height / 2;
    int offset_c = filter->width / 2;
    int size = job->width * job->height;
    const unsigned char * pixels = job->image[channel];
    unsigned char * result = calloc(size, sizeof(unsigned char));

    int h, w, r, c;

    for (h = job->y1; h < job->y2; h++) {
        for (w = job->x1; w < job->x2; w++) {
            float val = 0;
            for (r = 0; r < filter->height; r++) {
                for (c = 0; c < filter->width; c++) {
                    int x = w + c - offset_c;
                    int y = h + r - offset_r;
                    int i = y * job->width + x;
                    if (!(i < 0 || i >= size))
                        val += (pixels[i] * filter->filter[r][c]);
                }
            }
            result[h*job->width + w] = (unsigned char)(val + 0.5);
        }
    }

    return result;
}


unsigned char ** do_job(const job_t * job, const filter_t * filter) {
    LOG("Doing job (%d, %d, %d, %d) for (%d, %d)",
        job->x1, job->y1, job->x2, job->y2,
        job->orig_x1, job->orig_y1);

    unsigned char ** result = calloc(3, sizeof(unsigned char*));

    int c;
    for (c = 0; c < 3; c++)
        result[c] = filter_channel(job, filter, c);
    return result;
}


void send_result(const job_t * job, unsigned char ** result) {
    int w = job->x2 - job->x1;
    int h = job->y2 - job->y1;
    int dimensions[4] = {job->orig_x1,     job->orig_y1,
                         job->orig_x1 + w, job->orig_y1 + h};
    MPI_Send(dimensions, 4, MPI_INT, 0, 0, MPI_COMM_WORLD);

    int y, c;
    for (c = 0; c < 3; c++)
        for (y = job->y1; y < job->y2; y++)
            MPI_Send(result[c] + y * job->width + job->x1, w,
                     MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
}


void fetch_result(unsigned char ** result, int worker, int width) {
    int dimensions[4];
    MPI_Recv(dimensions, 4, MPI_INT, worker, 0, MPI_COMM_WORLD, 0);
    int x = dimensions[0];
    int w = dimensions[2] - dimensions[0];
    int c, y;
    for (c = 0; c < 3; c++)
        for (y = dimensions[1]; y < dimensions[3]; y++)
            MPI_Recv(result[c] + y * width + x, w, MPI_UNSIGNED_CHAR,
                     worker, 0, MPI_COMM_WORLD, 0);
    LOG("Fetched %d", worker);
}


void fetch_results(unsigned char ** result, int width) {
    int workers, i;
    MPI_Comm_size(MPI_COMM_WORLD, &workers);
    for (i = 1; i < workers; i++)
        fetch_result(result, i, width);
}

int mpi_main(int argc, char **argv,
             job_t (send_jobs)(filter_t *, unsigned char **, int, int),
             job_t (get_job)(filter_t *)) {
    int rank, image_width, image_height;
    filter_t filter;
    job_t job;
    double starttime;
    unsigned char ** result;

    MPI_Init(&argc, &argv);

    starttime = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Init
    if (rank == 0) {
        // Read in image and filter and send to workers
        if (argc < 3 || argc > 4) {
            fprintf(stderr, "USAGE: %s filter.txt input.pnm [output.pnm]\n",
                    argv[1]);
            return 1;
        }

        const char * filter_path = argv[1];
        const char * image_path  = argv[2];
        const char * output_path = argv[argc == 3 ? 2 : 3];

        unsigned char ** image;

        filter.filter = read_filter(filter_path, &filter.width, &filter.height);
        image = read_image(image_path, &image_width, &image_height);

        send_filter(&filter);
        job = send_jobs(&filter, image, image_width, image_height);

        result = do_job(&job, &filter);

        LOG("Fetching results.");
        fetch_results(result, image_width);

        LOG("Writing output");
        write_image(output_path, result, job.width, job.height);
    } else {
        // Receive filter and job
        filter = get_filter();
        job = get_job(&filter);

        result = do_job(&job, &filter);

        LOG("Sending results");
        send_result(&job, result);
    }

    // Cleanup
    /* free_filter(filter.filter, filter.height); */
    /* free_image(job.image); */
    /* free_image(result); */

    starttime = MPI_Wtime() - starttime;
    if (rank == 0)
        printf("%f\n", starttime);

    MPI_Finalize();

    return 0;
}
