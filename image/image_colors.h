#ifndef IMAGE_COLORS_H
#define IMAGE_COLORS_H

#include <stdint.h>
#include <vector>
#include "image_r.h"
#include "geom/line.h"
#include "opt/opt.h"

// add IMAGE group of options
#include "getopt/getopt.h"
void ms2opt_add_image_cmap(GetOptSet & opts);

// Create a colormap.
// Based on pnmcolormap.c from netpbm package.
std::vector<uint32_t> image_colormap(const ImageR & img, const Opt & opt = Opt());

// Reduce number of colors
ImageR image_remap(const ImageR & img, const std::vector<uint32_t> & cmap);

// convert image data to 32-bit colors (for making Cairo patterns etc).
ImageR image_to_argb(const ImageR & img);

// Image tranparency (only for 32bpp images)
// returns 0: fully non-transparent image.
// returns 1: image contains transparent (but not semi-transparent) pixels.
// returns 2: image contains semi-transparent pixels.
int image_classify_alpha(const ImageR & img);

// Image colors (only for 32bpp images)
// returns 0: grayscale
// returns 1: color,     <=clen colors.
// returns 2: color,     >clen colors.
// colors[clen] array is filled with the color palette.
int image_classify_color(const ImageR & img, uint32_t *colors, int clen=256);

#endif
