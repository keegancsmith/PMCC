CC=gcc

CFLAGS=-Wall -O3 -fopenmp `mpicc -showme:compile`
CLIBS=-lm

all: filter_mp filter_mpi_basic filter_mpi_neigh filter_mpi_ghost gen

filter_mp: filter_mp.c image.o
	$(CC) $(CFLAGS) -o filter_mp filter_mp.c image.o $(CLIBS)

filter_mpi_basic: filter_mpi_basic.c filter_mpi.o image.o
	$(CC) $(CFLAGS) -o filter_mpi_basic filter_mpi_basic.c filter_mpi.o image.o $(CLIBS) `mpicc -showme:link`

filter_mpi_neigh: filter_mpi_neigh.c filter_mpi.o image.o
	$(CC) $(CFLAGS) -o filter_mpi_neigh filter_mpi_neigh.c filter_mpi.o image.o $(CLIBS) `mpicc -showme:link`

filter_mpi_ghost: filter_mpi_ghost.c filter_mpi.o image.o
	$(CC) $(CFLAGS) -o filter_mpi_ghost filter_mpi_ghost.c filter_mpi.o image.o $(CLIBS) `mpicc -showme:link`

gen: gen.c image.o
	$(CC) $(CFLAGS) -o gen gen.c image.o $(CLIBS)

filter_mpi.o: filter_mpi.c filter_mpi.h
	$(CC) $(CFLAGS) -c filter_mpi.c

image.o: image.c image.h
	$(CC) $(CFLAGS) -c image.c

clean:
	rm filter_mp filter_mpi_basic filter_mpi_ghost filter_mpi_neigh gen *.o

check-syntax:
	$(CC) $(CFLAGS) -Wextra -Wno-sign-compare -fsyntax-only $(CHK_SOURCES)
