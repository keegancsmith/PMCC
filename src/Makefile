CC=g++

CFLAGS=-Wall -O3 -fopenmp
CLIBS=-lm

all: filter_mp

filter_mp: filter_mp.cpp image.o
	c++ $(CFLAGS) -o filter_mp filter_mp.cpp image.o $(CLIBS)

image.o: image.cpp image.h
	c++ $(CFLAGS) -c image.cpp $(CLIBS)

check-syntax:
	$(CC) $(CFLAGS) -Wextra -Wno-sign-compare -fsyntax-only $(CHK_SOURCES)
