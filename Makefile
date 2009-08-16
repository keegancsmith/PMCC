CC=g++

CFLAGS=-Wall -O3 `Magick++-config --cppflags --cxxflags --ldflags`
CLIBS=-lm `Magick++-config --libs`

filter_mp: filter_mp.cpp
	c++ $(CFLAGS) -o filter_mp filter_mp.cpp $(CLIBS)

check-syntax:
	$(CC) $(CFLAGS) -Wextra -Wno-sign-compare -fsyntax-only $(CHK_SOURCES)
