#ifndef IMAGE_CNT_H
#define IMAGE_CNT_H

#include <map>
#include "image/image_r.h"
#include "geom/multiline.h"

// Find contours on the image.
// Parameters:
//   img -- image of any type which supports get_double() (see image/image_r.h)
//   vmin, vmax, step -- contours. Both vmin and vmax could be NaN, step should be positive.
//   closed -- produce closed polygons instead of lines.
std::map<double, dMultiLine> image_cnt(const ImageR & img,
          const double vmin, const double vmax, const double vstep,
          const bool closed);

// Filter result of image_cnt with vertical tolerance vtol
// (move lines within vertical tolerance vtol minimizing their length)
void image_cnt_vtol_filter(const ImageR & img,
  std::map<double, dMultiLine> & lines, const double vtol=0.0);

// Smooth image with a limited vertical change
// - Smoothing is done using Gaussian weight with radius dr.
// - Actial smoothing is obtained from calculated change my
//   multiplying it with some limited function  (options available in the code)
//   h => h*f(h/dh)
//   where f(x)->1 at x<<1, f(x)->1/x or smaller at h/dh>>Inf,

ImageR image_smooth_lim(const ImageR & img, const double dh, const double dr);

// Find peaks on the image.
// Parameters:
//   img -- image of any type which supports get_double() (see image/image_r.h)
//   DH -- min "distance" between peaks (minimum acsent needed to move to the nearest peak)
//   PS -- When calculating a peak do not collect more then PS points
//        (0 means image.width * image.height)
//   minh -- exclude peaks below this value (could be useful to avoid long calculations on flats)
dLine image_peaks(const ImageR & img, double DH, size_t PS=0, double minh = NAN);

#endif