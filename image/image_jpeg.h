#ifndef IMAGE_JPEG_H
#define IMAGE_JPEG_H

#include <string>
#include <iostream>
#include "geom/point.h"
#include "image.h"

// get file dimensions (from file)
iPoint image_size_jpeg(const std::string & file);

// get jpeg dimensions (from std::istream)
iPoint image_size_jpeg(std::istream & str);

// load the whole image (from file)
Image image_load_jpeg(const std::string & file, const double scale=1);

// load the whole image (from std::istream)
Image image_load_jpeg(std::istream & str, const double scale=1);

// save the whole image (to file)
void image_save_jpeg(const Image & im, const std::string & file,
                     const Opt & opt = Opt());

// save the whole image (to std::ostream)
void image_save_jpeg(const Image & im, std::ostream & str,
                     const Opt & opt = Opt());

#endif
