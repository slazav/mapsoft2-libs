#ifndef IMAGE_PNG_H
#define IMAGE_PNG_H

#include <string>
#include "geom/point.h"
#include "opt/opt.h"
#include "image.h"


// getting file dimensions
iPoint image_size_png(std::istream & str);
iPoint image_size_png(const std::string & fname);

// load the whole image
Image image_load_png(std::istream & str, const double scale);
Image image_load_png(const std::string & fname, const double scale=1);

// save the whole image
void image_save_png(const Image & im, std::ostream & str,
                    const Opt & opt = Opt());
void image_save_png(const Image & im, const std::string & fname,
                    const Opt & opt = Opt());

#endif
