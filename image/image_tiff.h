#ifndef IMAGE_TIFF_H
#define IMAGE_TIFF_H

#include <string>
#include "geom/point.h"
#include "image_r.h"

// get file dimensions (from file)
iPoint image_size_tiff(const std::string & file);

// get file dimensions (from std::istream)
iPoint image_size_tiff(std::istream & str);

// load TIFF image (from file)
ImageR image_load_tiff(const std::string & file, const double scale=1);

// load TIFF image (from std::istream)
ImageR image_load_tiff(std::istream & str, const double scale=1);

// save the whole image (to file)
void image_save_tiff(const ImageR & im, const std::string & file,
                     const Opt & opt = Opt());

// save the whole image (to std::ostream)
void image_save_tiff(const ImageR & im, std::ostream & str,
                     const Opt & opt = Opt());

#endif
