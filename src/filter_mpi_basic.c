/* Basic implementation for distributing image. Just broadcast image to
 * everyone and evenly divide work by row. */

#include "filter_mpi.h"

#include <mpi.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

job_t send_jobs(filter_t * f, unsigned char ** image, int width, int height) {
    int dimensions[2] = {width, height};
    int image_size = width * height;
    int c;

    // Broadcast image
    MPI_Bcast(dimensions, 2, MPI_INT, 0, MPI_COMM_WORLD);
    for (c = 0; c < 3; c++)
        MPI_Bcast(image[c], image_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);


    // Master job
    int workers;
    MPI_Comm_size(MPI_COMM_WORLD, &workers);
    int rows = (height + workers - 1) / workers;

    job_t job = {
        image,
        width, height,
        0, 0, width, rows,
        0, 0
    };

    return job;
}


job_t get_job(filter_t * f) {
    job_t job;
    int dimensions[2];
    int image_size;
    int c;

    // Broadcast image
    MPI_Bcast(dimensions, 2, MPI_INT, 0, MPI_COMM_WORLD);
    job.width  = dimensions[0];
    job.height = dimensions[1];
    image_size = job.width * job.height;

    job.image = calloc(3, sizeof(unsigned char*));
    for (c = 0; c < 3; c++) {
        job.image[c] = calloc(image_size, sizeof(unsigned char));
        MPI_Bcast(job.image[c], image_size, MPI_UNSIGNED_CHAR, 0,
                  MPI_COMM_WORLD);

    }

    // Calculate job
    int workers, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &workers);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int rows = (job.height + workers - 1) / workers;
    int start = rows * rank;

    if (start >= job.height) {
        start = job.height;
        rows = 0;
    }

    job.x1 = 0;
    job.x2 = job.width;
    job.y1 = start;
    job.y2 = start + rows;

    job.orig_x1 = 0;
    job.orig_y1 = start;

    return job;
}

int main(int argc, char ** argv) {
    return mpi_main(argc, argv, send_jobs, get_job);
}
