import numpy as np

# for the case where the image is its own reference image


def bilateral_filter(inp, n_input_channels, n_sdims,
                     sdims, num_super_pixels, theta_alpha,
                     theta_beta):

    pd = n_input_channels + n_sdims
    vd = n_input_channels + 1
    n = num_super_pixels

    print("Constructing kernel...")
    positions = np.zeros(num_super_pixels * pd)
    compute_kernel_cpu(inp, positions, n, 3,
                       2, sdims, theta_alpha, theta_beta)

    print("Calling filter...")
    out = np.zeros(num_super_pixels * n_input_channels)

    lattice = PermutohedralLatticeCPU(pd, vd, n)
    lattice.filter(out, inp, positions, false)

    inp = out
