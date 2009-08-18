CC=g++

CFLAGS=-Wall -O3 -fopenmp `mpic++ -showme:compile`
CLIBS=-lm

all: filter_mp filter_mpi

filter_mp: filter_mp.cpp image.o
	$(CC) $(CFLAGS) -o filter_mp filter_mp.cpp image.o $(CLIBS)

filter_mpi: filter_mpi.cpp image.o
	$(CC) $(CFLAGS) -o filter_mpi filter_mpi.cpp image.o $(CLIBS) `mpic++ -showme:link`

image.o: image.cpp image.h
	$(CC) $(CFLAGS) -c image.cpp $(CLIBS)

clean:
	rm filter_mp filter_mpi

check-syntax:
	$(CC) $(CFLAGS) -Wextra -Wno-sign-compare -fsyntax-only $(CHK_SOURCES)
