from PIL import Image
import argparse
import numpy as np

from bilateral_filter import bilateral_filter


def main(args):
    im = Image.open(args.input)
    pixel_depth = 255.0
    flat = np.array(im).flatten()/pixel_depth
    N = im.height*im.width
    sdims = [im.height, im.width]

    theta_alpha = args.spatial_std
    theta_beta = args.colour_std

    bilateral_filter(flat, 3, 2, sdims, N, theta_alpha, theta_beta)

    result = Image.fromarray(
        (flat.reshape((im.height, im.width, 3)) * 255).astype(np.uint8))
    result.save('out.bmp')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Usage: python test_bilateral_cpu <input file> <output file> <spatial standard deviation> <colour standard deviation>\n')
    parser.add_argument('input', type=str,
                        help='input file to run on')
    parser.add_argument('output', type=str,
                        help='input file to run on')
    parser.add_argument('--spatial_std', type=float, default=8,
                        help='spatial standard deviation')
    parser.add_argument('--colour_std', type=float, default=0.125,
                        help='colour standard deviation')

    args = parser.parse_args()
    main(args)
