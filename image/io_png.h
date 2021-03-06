#ifndef IMAGE_PNG_H
#define IMAGE_PNG_H

#include <string>
#include "geom/point.h"
#include "opt/opt.h"
#include "image_r.h"


// get file dimensions (from std::istream)
iPoint image_size_png(std::istream & str);

// get file dimensions (from file)
iPoint image_size_png(const std::string & fname);

// load the whole image (from std::istream)
ImageR image_load_png(std::istream & str, const double scale);

// load the whole image (from file)
ImageR image_load_png(const std::string & fname, const double scale=1);

// save the whole image (to std::ostream)
void image_save_png(const ImageR & im, std::ostream & str,
                    const Opt & opt = Opt());

// save the whole image (to file)
void image_save_png(const ImageR & im, const std::string & fname,
                    const Opt & opt = Opt());

#endif
