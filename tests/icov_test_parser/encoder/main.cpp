#if _MSC_VER
#define _HAS_STD_BYTE 0
#include <Windows.h>
#endif

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

using namespace std;

int main(int argc, char *argv[])
{

    int block_size = 32;

    int image_width = block_size * 16;
    int image_height = block_size * 8;
    int pixel_depth = 1;

    int block_pixel_count = block_size * block_size;

    int image_pixel_count = image_width * image_height * pixel_depth;
    int block_count = image_pixel_count / block_pixel_count;

    int blocks_per_row = image_width / block_size;
    int blocks_per_col = image_height / block_size;
    int pixels_per_row = image_width * block_size;

    //I - ENCODE AS RAW BITMAP
    unsigned char *s_data = new unsigned char[image_pixel_count];
    unsigned char *dst = (unsigned char *)s_data;

    for (int y = 0; y < image_height; y++)
    {
        unsigned char *ptr = dst;

        int block_y = y / block_size;

        for (int x = 0; x < image_width; x++)
        {
            int block_x = x / block_size;

            ptr[0] = (blocks_per_col - block_y - 1) * (127.0 / blocks_per_col) + block_x * (127.0 / blocks_per_row);

            cout << x << ", " << y << ": " << (int)ptr[0] << endl;

            // To next pixel
            ptr += pixel_depth;
        }
        // To next image row
        dst += image_width * pixel_depth;
    }

    cout << "write raw bitmap file" << endl;

    ofstream fout;
    fout.open("test.data", ios::binary | ios::out);
    fout.write((char *)s_data, image_pixel_count);
    fout.close();

    //II - ENCODE AS RAW BLOCKSMAP

    unsigned char *blocks_data = new unsigned char[image_pixel_count];
    unsigned char *blocks_dst = (unsigned char *)blocks_data;

    for (int k = 0; k < block_count; k++)
    {
        int srcOffset = (k / blocks_per_row) * pixels_per_row + k % blocks_per_row * block_size;
        unsigned char *src = (unsigned char *)s_data + srcOffset;

        //cout << "OFFSET BLOCK " << k << "=" << srcOffset << endl;

        for (int j = 0; j < block_size; j++)
        {
            for (int i = 0; i < block_size; i++)
            {
                blocks_dst[0] = src[i];
                blocks_dst++;
            }
            src += image_width;
        }
    }

    cout << "write raw blocks file" << endl;

    fout.open("test.icov", ios::binary | ios::out);
    fout.write((char *)blocks_data, image_pixel_count);
    fout.close();

    return 0;
}
