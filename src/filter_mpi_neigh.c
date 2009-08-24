/* Slightly smarter implementation for distributing image. Only send what each
 * image needs */

#include "filter_mpi.h"

#include <mpi.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


inline int max(int a, int b) {
    return a < b ? b : a;
}

inline int min(int a, int b) {
    return a < b ? a : b;
}


job_t send_jobs(filter_t * f, unsigned char ** image, int width, int height) {
    int c, i;

    // Dimensions
    int dim[2] = {width, height};
    MPI_Bcast(dim, 2, MPI_INT, 0, MPI_COMM_WORLD);

    // Get number of workers
    int workers;
    MPI_Comm_size(MPI_COMM_WORLD, &workers);

    // Divide rows up. Cant be smaller than half size of filter
    int rows = (height + workers - 1) / workers;
    rows = max(rows, f->height / 2);

    // Send jobs
    for (i = 1; i < workers; i++) {
        int orig_y1 = min(rows * i, height);
        int orig_y2 = min(orig_y1 + rows, height);

        int buf[2] = {orig_y1, orig_y2};
        MPI_Send(buf, 2, MPI_INT, i, 0, MPI_COMM_WORLD);

        // Does not use node
        if (orig_y1 == height)
            continue;

        int img_y1 = max(orig_y1 - f->height / 2, 0);
        int img_y2 = min(orig_y2 + f->height / 2, height);
        int img_height = img_y2 - img_y1;
        MPI_Send(&img_height, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

        int y1 = f->height / 2;
        int y2 = y1 + (orig_y2 - orig_y1);

        buf[0] = y1; buf[1] = y2;
        MPI_Send(buf, 2, MPI_INT, i, 0, MPI_COMM_WORLD);

        for (c = 0; c < 3; c++)
            MPI_Send(image[c] + img_y1 * width, img_height * width,
                     MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
    }

    // Master job
    job_t job = {
        image,
        width, height,
        0, 0, width, rows,
        0, 0
    };

    return job;
}


job_t get_job(filter_t * f) {
    int buf[2];
    int c;
    int width, height;

    // Get global dimensions
    MPI_Bcast(buf, 2, MPI_INT, 0, MPI_COMM_WORLD);
    width  = buf[0];
    height = buf[1];

    // Get orig_y
    MPI_Recv(buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, 0);
    int orig_y1 = buf[0];

    if (orig_y1 == height) {
        job_t job = {
            0,
            0, 0,
            0, 0, 0, 0,
            0, 0
        };
        return job;
    }

    int img_height;
    MPI_Recv(&img_height, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, 0);
    int image_size = img_height * width;
    
    // Get y
    MPI_Recv(buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, 0);
    int y1 = buf[0];
    int y2 = buf[1];

    unsigned char ** image = calloc(3, sizeof(unsigned char*));
    for (c = 0; c < 3; c++) {
        image[c] = calloc(image_size, sizeof(unsigned char));
        MPI_Recv(image[c], image_size, MPI_UNSIGNED_CHAR,
                 0, 0, MPI_COMM_WORLD, 0);
    }

    job_t job = {
        image,
        width, img_height,
        0, y1, width, y2,
        0, orig_y1
    };

    return job;
}

int main(int argc, char ** argv) {
    return mpi_main(argc, argv, send_jobs, get_job);
}
