//
// Created by Miguel Monteiro on 29/01/2018.
//
#include "../src/PermutohedralLatticeCPU.h"
#include "memory"

//for the case where the image is its own reference image
template <typename T>
static void bilateral_filter_cpu(T* input,
    int n_input_channels,
    int n_sdims,
    int* sdims,
    int num_super_pixels,
    T theta_alpha,
    T theta_beta)
{

    int pd = n_input_channels;
    int vd = n_input_channels + 1;
    int n = num_super_pixels;

    printf("Constructing kernel...\n");
    auto positions = new T[num_super_pixels * pd];
    compute_kernel_cpu<T>(input, positions, n, n_input_channels, n_sdims, sdims, theta_alpha, theta_beta);

    printf("Calling filter...\n");
    auto output = new T[num_super_pixels * n_input_channels];

    auto lattice = PermutohedralLatticeCPU<T>(pd, vd, n);
    lattice.filter(output, input, positions, false);

    //std::swap(input, output);
    std::memcpy(input, output, num_super_pixels * n_input_channels * sizeof(T));
    delete[] output;
    delete[] positions;
}