#include "image.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

using namespace std;

void filter_channel(float ** filter, int filter_width, int filter_height,
                    unsigned char * pixels, int image_width, int image_height) {
    int offset_r = filter_height / 2;
    int offset_c = filter_width / 2;
    int size = image_width * image_height;
    #pragma omp parallel for
    for (int h = 0; h < image_height; h++) {
        for (int w = 0; w < image_width; w++) {
            float val = 0;
            for (int r = 0; r < filter_height; r++) {
                for (int c = 0; c < filter_width; c++) {
                    int x = w - c + offset_c;
                    int y = h - r + offset_r;
                    int i = y*image_width + x;
                    if (!(i < 0 || i >= size))
                        val += (pixels[i] * filter[r][c]);
                }
            }
            pixels[h*image_width + w] = (unsigned char)(val + 0.5);
        }
    }
}


void filter_image(float ** filter, int filter_width, int filter_height,
                  unsigned char ** pixels, int image_width, int image_height) {
    for (int i = 0; i < 3; i++)
        filter_channel(filter, filter_width, filter_height,
                       pixels[i], image_width, image_height);
}


int main(int argc, char **argv)
{
    if (argc < 3 || argc > 4) {
        cerr << "Usage: " << argv[0] << " filter.txt input.pnm [output.pnm]\n";
        return 1;
    }

    string filter_path = argv[1];
    string image_path  = argv[2];
    string output_path = argv[argc == 3 ? 2 : 3];

    int filter_width, filter_height, image_width, image_height;
    float ** filter;
    unsigned char ** image;

    filter = read_filter(filter_path, filter_width, filter_height);
    image  = read_image(image_path,   image_width,  image_height);

    filter_image(filter, filter_width, filter_height,
                 image, image_width, image_height);

    write_image(output_path, image, image_width, image_height);

    free_filter(filter, filter_height);
    free_image(image);
    
    return 0;
}
