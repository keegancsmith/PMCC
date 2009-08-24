#!/bin/sh

for i in basic neigh ghost; do
    /CHPC/usr/local/mvapich/bin/mpicc -O3 filter_mpi_$i.c image.c filter_mpi.c -o filter_mpi_$i
done