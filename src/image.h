#ifndef IMAGE_H
#define IMAGE_H

#include <string>

// Filter
float ** read_filter(std::string path, int & W, int & H);
void free_filter(float ** filter, int H);

// Image
unsigned char ** read_image(std::string path, int & width, int & height);
void write_image(std::string path, unsigned char ** pixels,
                 int width, int height);
void free_image(unsigned char ** pixels);

#endif
