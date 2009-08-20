#include "image.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char * filter_channel(float ** filter,
                               int filter_width, int filter_height,
                               unsigned char * pixels,
                               int image_width, int image_height) {
    int offset_r = filter_height / 2;
    int offset_c = filter_width / 2;
    int size = image_width * image_height;
    unsigned char * filtered = calloc(size, sizeof(unsigned char));
    int h;

    #pragma omp parallel for
    for (h = 0; h < image_height; h++) {
        int w, r, c;
        for (w = 0; w < image_width; w++) {
            float val = 0;
            for (r = 0; r < filter_height; r++) {
                for (c = 0; c < filter_width; c++) {
                    int x = w + c - offset_c;
                    int y = h + r - offset_r;
                    int i = y*image_width + x;
                    if (!(i < 0 || i >= size))
                        val += (pixels[i] * filter[r][c]);
                }
            }
            filtered[h*image_width + w] = (unsigned char)(val + 0.5);
        }
    }

    return filtered;
}


unsigned char ** filter_image(float ** filter,
                              int filter_width, int filter_height,
                              unsigned char ** pixels,
                              int image_width, int image_height) {
    int i;
    unsigned char ** filtered = calloc(3, sizeof(unsigned char *));
    for (i = 0; i < 3; i++)
        filtered[i] = filter_channel(filter, filter_width, filter_height,
                                     pixels[i], image_width, image_height);
    return filtered;
}


int main(int argc, char **argv)
{
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s filter.txt input.pnm [output.pnm]\n", argv[0]);
        return 1;
    }

    char * filter_path = argv[1];
    char * image_path  = argv[2];
    char * output_path = argv[argc == 3 ? 2 : 3];

    int filter_width, filter_height, image_width, image_height;
    float ** filter;
    unsigned char ** image;
    unsigned char ** filtered;

    filter = read_filter(filter_path, &filter_width, &filter_height);
    image  = read_image(image_path,   &image_width,  &image_height);

    filtered = filter_image(filter, filter_width, filter_height,
                            image, image_width, image_height);

    write_image(output_path, filtered, image_width, image_height);

    free_filter(filter, filter_height);
    free_image(image);
    free_image(filtered);

    return 0;
}
