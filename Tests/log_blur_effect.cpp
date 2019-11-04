#include <cstdio>
#include <cstdlib>
#include <cstring>
#define cimg_display 0
#include "bilateral_filter_cpu.h"
#include "utils.h"
#include <boost/filesystem.hpp>
#include <ctime>
#include <sys/time.h>

using T = double;

using namespace boost::filesystem;
int main(int argc, char** argv)
{
    if (argc < 5) {
        printf("Usage: ./log_blur_effect <input folder> <output file> <colour standard deviation> <number of points>\n");
        return 1;
    }

    for (directory_entry& entry : directory_iterator(argv[1])) {
        int point_dim = 3;
        int N = atof(argv[5]);
        int sdims[2]{ atof(argv[4]), 1.0 };
        auto flat = new T[N * point_dim];

        load_txt_file_flattened(flat, entry);

        //float invSpatialStdev = 1.0f / atof(argv[3]);
        //float invColorStdev = 1.0f / atof(argv[4]);
        //auto positions = compute_kernel(image, invSpatialStdev, invColorStdev);
        T theta_alpha = 0;
        T theta_beta = atof(argv[3]);

        std::clock_t begin = std::clock();
        bilateral_filter_cpu<T>(flat, point_dim, 0, sdims, N, theta_alpha, theta_beta);
        std::clock_t end = std::clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        printf("Measured from function call: %f seconds\n", elapsed_secs);

        printf("Saving output...\n");
        // save_output<T>(flat, image, argv[2], pixel_depth);

        delete[] flat;
    }

    return 0;
}
