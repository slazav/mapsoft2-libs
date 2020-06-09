#ifndef IMAGE_TIFF_H
#define IMAGE_TIFF_H

#include <string>
#include "geom/point.h"
#include "image.h"

// getting file dimensions
iPoint image_size_tiff(const std::string & file);

// getting file dimensions from std::istream data
iPoint image_size_tiff(std::istream & str);

// load TIFF image from a file
Image image_load_tiff(const std::string & file, const double scale=1);

// load TIFF image from std::istream data
Image image_load_tiff(std::istream & str, const double scale=1);

//save the whole image
void image_save_tiff(const Image & im, const std::string & file,
                     const Opt & opt = Opt());

#endif
