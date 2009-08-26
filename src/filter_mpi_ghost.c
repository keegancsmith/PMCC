/* Use ghost cells. */

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
        buf[0] = img_y1;
        buf[1] = img_y2;
        MPI_Send(buf, 2, MPI_INT, i, 0, MPI_COMM_WORLD);

        for (c = 0; c < 3; c++)
            MPI_Send(image[c] + orig_y1 * width, (orig_y2 - orig_y1) * width,
                     MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
    }


    // Send
    int orig_y2 = min(rows, height);
    int orig_y2_ghost = max(orig_y2 - f->height / 2, 0);
    if (orig_y2 != orig_y2_ghost && workers > 1) {
        LOG("Sending south ghost cell (%d, %d)", orig_y2_ghost, orig_y2);
        int offset = orig_y2_ghost * width;
        int size = (orig_y2 - orig_y2_ghost) * width;
        for (c = 0; c < 3; c++)
            MPI_Send(image[c] + offset, size, MPI_UNSIGNED_CHAR,
                     1, 0, MPI_COMM_WORLD);
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
    int orig_y2 = buf[1];
    int work_size = (orig_y2 - orig_y1) * width;

    if (orig_y1 == height) {
        job_t job = {
            0,
            0, 0,
            0, 0, 0, 0,
            0, 0
        };
        return job;
    }

    // Get img_y
    MPI_Recv(buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, 0);
    int img_y1 = buf[0];
    int img_y2 = buf[1];
    int image_size = (img_y2 - img_y1) * width;

    // Get y
    int y1 = f->height / 2;
    int y2 = y1 + (orig_y2 - orig_y1);

    unsigned char ** image = calloc(3, sizeof(unsigned char*));
    for (c = 0; c < 3; c++) {
        image[c] = calloc(image_size, sizeof(unsigned char));
        MPI_Recv(image[c] + y1 * width, work_size, MPI_UNSIGNED_CHAR,
                 0, 0, MPI_COMM_WORLD, 0);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    // Fetch north ghost cell
    int offset = 0;
    int size = (orig_y1 - img_y1) * width;
    LOG("Fetching north ghost cell (%d, %d)", img_y1, orig_y1);
    assert(orig_y1 != img_y1);
    for (c = 0; c < 3; c++)
        MPI_Recv(image[c] + offset, size, MPI_UNSIGNED_CHAR,
                 rank - 1, 0, MPI_COMM_WORLD, 0);

    // South cells
    if (orig_y2 != img_y2) {
        // Send
        int y2_ghost = max(y2 - f->height / 2, 0);
        offset = y2_ghost * width;
        size = (y2 - y2_ghost) * width;
        LOG("Sending south ghost cell (%d, %d)",
            orig_y2 - y2 + y2_ghost, orig_y2);
        for (c = 0; c < 3; c++)
            MPI_Send(image[c] + offset, size, MPI_UNSIGNED_CHAR,
                     rank + 1, 0, MPI_COMM_WORLD);

        // Fetch
        offset = y2 * width;
        size = (img_y2 - orig_y2) * width;
        LOG("Fetching south ghost cell (%d, %d) %d %d", orig_y2, img_y2);
        for (c = 0; c < 3; c++)
            MPI_Recv(image[c] + offset, size, MPI_UNSIGNED_CHAR,
                     rank + 1, 0, MPI_COMM_WORLD, 0);
    }

    // Send north if not sending to master
    if (rank > 1) {
        int y1_ghost = min(y1 + f->height / 2, y2);
        offset = y1 * width;
        size = (y1_ghost - y1) * width;
        LOG("Sending north ghost cell (%d, %d)",
            orig_y1, orig_y1 - y1 + y1_ghost);
        for (c = 0; c < 3; c++)
            MPI_Send(image[c] + offset, size, MPI_UNSIGNED_CHAR,
                     rank - 1, 0, MPI_COMM_WORLD);
    }



    job_t job = {
        image,
        width, img_y2 - img_y1,
        0, y1, width, y2,
        0, orig_y1
    };

    return job;
}

int main(int argc, char ** argv) {
    return mpi_main(argc, argv, send_jobs, get_job);
}
