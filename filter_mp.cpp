#include <Magick++.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

using namespace std;
using namespace Magick;


// Filter Stuff
float ** read_filter(string path, int & W, int & H) {
    ifstream fin(path.c_str());

    fin >> W >> H;
    float ** filter = new float*[H];

    assert(W % 2 == 1 && H % 2 == 1);

    for (int h = 0; h < H; h++) {
        filter[h] = new float[W];
        for (int w = 0; w < W; w++)
            fin >> filter[h][w];
    }

    return filter;
}

void free_filter(float ** filter, int H) {
    for (int h = 0; h < H; h++)
        delete[] filter[h];
    delete[] filter;
}


// Image stuff
unsigned char ** read_image(string path, int & W, int & H) {
    Image image(path);
    image.type(TrueColorType);

    W = image.size().width();
    H = image.size().height();

    Pixels view(image);
    PixelPacket * pixels = view.get(0, 0, W, H);

    // Want to store pixels per channel for better cache consistency when
    // filtering
    int size = W * H;
    unsigned char ** raw_pixels = new unsigned char* [3];
    for (int i = 0; i < 3; i++)
        raw_pixels[i] = new unsigned char[size];

    unsigned char * r = raw_pixels[0];
    unsigned char * g = raw_pixels[1];
    unsigned char * b = raw_pixels[2];
    for (int i = 0; i < size; i++) {
        *(r++) = pixels->red;
        *(g++) = pixels->green;
        *(b++) = pixels->blue;
        pixels++;
    }

    return raw_pixels;
}

void write_image(string path, unsigned char ** raw_pixels, int W, int H) {
    Image image(Geometry(W,H), "white");
    image.type(TrueColorType);
    image.modifyImage();

    Pixels view(image);
    PixelPacket *pixels = view.get(0, 0, W, H);

    unsigned char * r = raw_pixels[0];
    unsigned char * g = raw_pixels[1];
    unsigned char * b = raw_pixels[2];
    int size = W * H;
    for (int i = 0; i < size; i++) {
        pixels->red   = *(r++);
        pixels->green = *(g++);
        pixels->blue  = *(b++);
        pixels++;
    }

    view.sync();
    image.syncPixels();
    image.write(path);
}

void free_image(unsigned char ** pixels) {
    for (int i = 0; i < 3; i++)
        delete [] pixels[i];
}


// Filtering image
void filter_channel(float ** filter, int filter_width, int filter_height,
                    unsigned char * pixels, int image_width, int image_height) {
    int offset_r = filter_height / 2;
    int offset_c = filter_width / 2;
    int size = image_width * image_height;
    for (int h = 0; h < image_height; h++) {
        for (int w = 0; w < image_width; w++) {
            float val = 0;
            for (int r = 0; r < filter_height; r++) {
                for (int c = 0; c < filter_width; c++) {
                    int i = (h - r + offset_r)*image_width + (w - c + offset_c)*image_height;
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
    assert(argc == 4);

    string filter_path = argv[1];
    string image_path  = argv[2];
    string output_path = argv[3];

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
