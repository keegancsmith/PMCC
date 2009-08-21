#include "image.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void image(const char * path, int width, int height) {
    int i, c;
    int size;
    unsigned char ** img;
    unsigned char * colour;

    size = width * height;
    img  = calloc(3, sizeof(unsigned char*));
    for (c = 0; c < 3; c++)
        img[c] = calloc(size, sizeof(unsigned char));

    for (c = 0; c < 3; c++) {
        colour = img[c];
        for (i = 0; i < size; i++)
            colour[i] = rand() % 256;
    }

    write_image(path, img, width, height);

    free_image(img);
}

void filter(const char * path, int width, int height) {
    int r, c;
    float ** filter;
    float * row;
    float total;
    FILE * fout;

    assert(width % 2 == 1 && height % 2 == 1);

    filter = calloc(height, sizeof(float*));
    for (r = 0; r < height; r++)
        filter[r] = calloc(width, sizeof(float));

    total = 0;
    for (r = 0; r < height; r++) {
        row = filter[r];
        for (c = 0; c < width; c++)
            total += (row[c] = rand() % 100000);
    }

    fout = fopen(path, "w");
    fprintf(fout, "%d %d\n", width, height);
    for (r = 0; r < height; r++) {
        row = filter[r];
        for (c = 0; c < width; c++)
            fprintf(fout, "%f ", row[c] / total);
        fprintf(fout, "\n");
    }
    fclose(fout);

    free_filter(filter, height);
}

int usage(const char * prog) {
    fprintf(stderr, "USAGE: %s image output.pnm width height\n", prog);
    fprintf(stderr, "USAGE: %s filter output.filter width height\n", prog);
    return 1;
}

int main(int argc, char ** argv) {
    if (argc != 5)
        return usage(argv[0]);

    srand(time(0));

    const char * command = argv[1];
    const char * output  = argv[2];
    int          width   = atoi(argv[3]);
    int          height  = atoi(argv[4]);

    assert(width > 0 && height > 0);

    if (strcmp(command, "image") == 0)
        image(output, width, height);
    else if (strcmp(command, "filter") == 0)
        filter(output, width, height);
    else
        return usage(argv[0]);

    return 0;
}
