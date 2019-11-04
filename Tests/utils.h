//
// Created by Miguel Monteiro on 25/01/2018.
//

#define cimg_display 0
#include "CImg.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
T* get_flat_float_from_image(cimg_library::CImg<unsigned char> image, T pixel_depth = 255.0)
{

    //dim0 = y dim1=x
    auto flat = new T[image.width() * image.height() * 3];
    int idx{ 0 };
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            for (int channel = 0; channel < 3; ++channel) {
                flat[idx] = image(x, y, 0, channel) / pixel_depth;
                idx++;
            }
        }
    }
    return flat;
}

template <typename T>
void save_output(T* out, cimg_library::CImg<unsigned char> image, char* filename, T pixel_depth = 255.0)
{
    int idx{ 0 };
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            for (int channel = 0; channel < 3; ++channel) {
                int value{ int(out[idx] * pixel_depth) };
                if (value > pixel_depth)
                    value = (int)pixel_depth;
                if (value < 0)
                    value = 0;
                image(x, y, channel) = value;
                idx++;
            }
        }
    }
    image.save(filename);
}

template <typename T>
void load_txt_file_flattened(T* out, directory_entry& entry)
{
    std::ifstream in(entry.path().c_str());
    int idx{ 0 };

    while (std::getline(in, str)) {
        std::string::iterator str_iter = str.begin();
        while (str_iter <= str.end()) {
            next_space = std::find(str_iter, str.end(), ' ');
            out[idx] = std::stod(new string(str_iter, next_space));
            str_iter = next_space + 1;
            idx++;
        }
    }
}