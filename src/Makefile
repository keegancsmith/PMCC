CC=gcc

CFLAGS=-Wall -O3 -fopenmp `mpicc -showme:compile`
CLIBS=-lm

all: filter_mp #filter_mpi

filter_mp: filter_mp.c image.o
	$(CC) $(CFLAGS) -o filter_mp filter_mp.c image.o $(CLIBS)

filter_mpi: filter_mpi.cpp image.o
	$(CC) $(CFLAGS) -o filter_mpi filter_mpi.cpp image.o $(CLIBS) `mpicc -showme:link`

image.o: image.c image.h
	$(CC) $(CFLAGS) -c image.c

clean:
	rm filter_mp filter_mpi *.o

check-syntax:
	$(CC) $(CFLAGS) -Wextra -Wno-sign-compare -fsyntax-only $(CHK_SOURCES)
