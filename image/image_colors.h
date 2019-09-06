#ifndef IMAGE_COLORS_H
#define IMAGE_COLORS_H

#include <stdint.h>
#include <vector>
#include "image.h"

// Luminance (from ITU-R BT.601.5)
#define COLOR_LUMINR (0.2989)
#define COLOR_LUMING (0.5866)
#define COLOR_LUMINB (0.1145)

// distance between two colors
double color_dist(const uint32_t c1, const uint32_t c2);

// Assemble 32-bit color from a,r,g,b components.
// Prescaled semi-transparent colors are used
uint32_t color_argb(const uint8_t a, const uint8_t r,
                    const uint8_t g, const uint8_t b);

// Remove transparency (with color scaling).
// if gifmode = true, then keep fully transparent colors.
uint32_t color_rem_transp(const uint32_t c, const bool gifmode);

// Create a colormap.
// Based on pnmcolormap.c from netpbm package.
std::vector<uint32_t> image_colormap(const Image & img);

// Reduce number of colors
Image image_remap(const Image & img, const std::vector<uint32_t> & cmap);

// Image tranparency (only for 32bpp images)
// returns 0: fully non-transparent image.
// returns 1: image contains transparent (but not semi-transparent) pixels.
// returns 2: image contains semi-transparent pixels.
int image_classify_alpha(const Image & img);

// Image colors (only for 32bpp images)
// returns 0: grayscale
// returns 1: color,     <=clen colors.
// returns 2: color,     >clen colors.
// colors[clen] array is filled with the color palette.
int image_classify_color(const Image & img, uint32_t *colors, int clen=256);

#endif
