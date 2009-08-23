#ifndef _FILTER_MPI_H
#define _FILTER_MPI_H


typedef struct {
    unsigned char ** image;
    int width, height;

    // Box to work on in image
    int x1, y1, x2, y2;

    int orig_x1, orig_y1;
} job_t;

typedef struct {
    float ** filter;
    int width, height;
} filter_t;


void LOG(const char* format, ...);


int mpi_main(int argc, char ** argv,
             job_t (send_jobs)(filter_t *, unsigned char **, int, int),
             job_t (get_job)(filter_t *));

#endif
