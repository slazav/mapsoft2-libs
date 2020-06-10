#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <string>
#include "opt/opt.h"
#include "geom/point.h"
#include "image.h"

// add IMAGE group of options
#include "getopt/getopt.h"
void ms2opt_add_image(GetOptSet & opts);

// get image size
iPoint image_size(const std::string & file, const Opt & opt = Opt());

// load the whole image at some scale
ImageR image_load(const std::string & file, const double scale=1, const Opt & opt = Opt());

// save the whole image
void image_save(const ImageR & im, const std::string & file, const Opt & opt = Opt());


#endif
