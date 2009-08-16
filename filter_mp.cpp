#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <climits>

using namespace std;


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
void skip_comment(istream & in) {
    in >> ws;
    if (in.peek() == '#') {
        in.ignore(INT_MAX, '\n');
        skip_comment(in);
    }
}

unsigned char ** read_image(string path, int & width, int & height) {
    ifstream fin(path.c_str(), ios::binary);

    char magic[2];
    fin.read(magic, 2);
    assert(magic[0] == 'P' && magic[1] == '6');
    skip_comment(fin);

    int maxval;
    fin >> width;  skip_comment(fin);
    fin >> height; skip_comment(fin);
    fin >> maxval;
    fin.read(magic, 1); // Read in single whitespace

    assert(maxval == 255);

    // Want to store pixels per channel for better cache consistency when
    // filtering
    int size = width * height;
    unsigned char ** pixels = new unsigned char* [3];
    for (int i = 0; i < 3; i++)
        pixels[i] = new unsigned char[size];

    for (int i = 0; i < size; i++) {
        char buf[3];
        fin.read(buf, 3);
        for (int c = 0; c < 3; c++)
            pixels[c][i] = buf[c];
    }

    fin.close();

    return pixels;
}

void write_image(string path, unsigned char ** pixels,
                 int width, int height) {
    ofstream fout(path.c_str(), ios::binary);
    fout << "P6 " << width << ' ' << height << ' ' << 255 << '\n';

    int size = width * height;
    for (int i = 0; i < size; i++) {
        char buf[3];
        for (int c = 0; c < 3; c++)
            buf[c]= pixels[c][i];
        fout.write((const char*)buf, 3);
    }
    fout.close();
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
