#ifndef IMAGE_COLORS_H
#define IMAGE_COLORS_H

#include <stdint.h>
#include <vector>
#include "image_r.h"
#include "geom/line.h"
#include "opt/opt.h"

// add IMAGE_CMAP group of options
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
int image_classify_color(const ImageR & img, uint32_t *colors, size_t clen=256);

// Invert colors (black <=> white), inplace, to change of image type
void image_invert(ImageR & img);

// Adjust color levels (darkest->0, lightest->max, middle->mr,mg,mb)
// brd - border for calculation [px]
// mr,mg,mb - middle levels (fraction of the full color range 0..1)
// t1,t2 - min/max color threshold (fraction of points which will be too dark/too bright, 0..1)
void image_autolevel(ImageR & img, size_t brd,
  double mr, double mg, double mb, double t1, double t2);

// Crop black/white borders.
// brd - max border width
// threshold - crop threshold, how far cropped lines should be
// from the inner border lines (0..1)
dRect image_autocrop(ImageR & img, size_t brd, double threshold=0.5);

// crop image to the rectangle (keep same image type)
ImageR image_crop(ImageR & img, const iRect & r);

#endif
