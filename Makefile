CC=g++

CFLAGS=-Wall -O3 -fopenmp
CLIBS=-lm

filter_mp: filter_mp.cpp
	c++ $(CFLAGS) -o filter_mp filter_mp.cpp $(CLIBS)

check-syntax:
	$(CC) $(CFLAGS) -Wextra -Wno-sign-compare -fsyntax-only $(CHK_SOURCES)
