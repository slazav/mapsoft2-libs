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
//   vtol   -- tolerance for filtering.

std::map<double, dMultiLine>
image_cnt(const ImageR & img,
          const double vmin, const double vmax, const double vstep,
          const bool closed, const double vtol=0.0);

#endif