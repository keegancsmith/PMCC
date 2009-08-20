#ifndef IMAGE_H
#define IMAGE_H

// Filter
float ** read_filter(const char * path, int * W, int * H);
void free_filter(float ** filter, int H);

// Image
unsigned char ** read_image(const char * path, int * width, int * height);
void write_image(const char * path, unsigned char ** pixels,
                 int width, int height);
void free_image(unsigned char ** pixels);

#endif
