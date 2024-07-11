#ifndef IMAGE_CNT_H
#define IMAGE_CNT_H

#include <map>
#include "image/image_r.h"
#include "geom/multiline.h"

// Find contours on the image
std::map<double, dMultiLine>
image_cnt(const ImageR & img,
          const double vmin, const double vmax, const double vstep,
          const bool closed, const double vtol=0.0);

#endif