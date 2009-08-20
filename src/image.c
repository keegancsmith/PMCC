#include "image.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// Filter Stuff
float ** read_filter(const char * path, int * W, int * H) {
    int h, w;
    float ** filter;
    FILE * fin = fopen(path, "r");

    fscanf(fin, "%d %d", W, H);
    assert(*W % 2 == 1 && *H % 2 == 1);

    filter = calloc(*H, sizeof(float *));

    for (h = 0; h < *H; h++) {
        filter[h] = calloc(*W, sizeof(float));
        for (w = 0; w < *W; w++)
            fscanf(fin, "%f", filter[h] + w);
    }

    fclose(fin);

    return filter;
}

void free_filter(float ** filter, int H) {
    int h;
    for (h = 0; h < H; h++)
        free(filter[h]);
    free(filter);
}


// Image stuff
void skip_comment(FILE * fin) {
    int c;

    // Skip whitespace;
    do {
        c = fgetc(fin);
    } while (isspace(c));
    ungetc(c, fin);

    // Check for another comment
    if (c != '#')
        return;

    // Read until a newline
    do {
        c = fgetc(fin);
    } while (c != '\n');

    skip_comment(fin);
}

unsigned char ** read_image(const char * path, int * width, int * height) {
    FILE * fin = fopen(path, "r");
    char magic[2];
    int maxval, size, i, c;
    unsigned char ** pixels;

    assert(fread(magic, sizeof(char), 2, fin));
    assert(magic[0] == 'P' && magic[1] == '6');
    skip_comment(fin);

    fscanf(fin, "%d", width);  skip_comment(fin);
    fscanf(fin, "%d", height); skip_comment(fin);
    fscanf(fin, "%d", &maxval);
    fgetc(fin); // Read in single whitespace

    assert(maxval == 255);

    // Want to store pixels per channel for better cache consistency when
    // filtering
    size = *width * *height;

    pixels = calloc(3, sizeof(unsigned char*));
    for (i = 0; i < 3; i++)
        pixels[i] = calloc(size, sizeof(unsigned char));

    for (i = 0; i < size; i++) {
        char buf[3];
        assert(fread(buf, sizeof(char), 3, fin));
        for (c = 0; c < 3; c++)
            pixels[c][i] = buf[c];
    }

    fclose(fin);

    return pixels;
}

void write_image(const char * path, unsigned char ** pixels,
                 int width, int height) {
    int size, i, c;
    FILE * fout = fopen(path, "wb");
    fprintf(fout, "P6 %d %d 255\n", width, height);

    size = width * height;
    for (i = 0; i < size; i++) {
        char buf[3];
        for (c = 0; c < 3; c++)
            buf[c]= pixels[c][i];
        fwrite(buf, sizeof(char), 3, fout);
    }
    fclose(fout);
}

void free_image(unsigned char ** pixels) {
    int i;
    for (i = 0; i < 3; i++)
        free(pixels[i]);
    free(pixels);
}
