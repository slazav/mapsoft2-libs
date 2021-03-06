#ifndef IMAGE_GIF_H
#define IMAGE_GIF_H

#include <string>
#include "geom/point.h"
#include "image_r.h"

// getting file dimensions
iPoint image_size_gif(const std::string & file);

// load the whole image
ImageR image_load_gif(const std::string & file, const double scale=1);

// save the whole image
void image_save_gif(const ImageR & im, const std::string & file,
                    const Opt & opt = Opt());

#endif
