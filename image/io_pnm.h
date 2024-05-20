#ifndef IMAGE_PNM_H
#define IMAGE_PNM_H

#include <string>
#include "geom/point.h"
#include "image_r.h"

// get file dimensions (from file)
iPoint image_size_pnm(const std::string & file);

// get file dimensions (from std::istream)
iPoint image_size_pnm(std::istream & str);

// load PBM/PGM/PPM/PNM/PAM image (from file)
ImageR image_load_pnm(const std::string & file, const double scale=1);

// load PBM/PGM/PPM/PNM/PAM image (from std::istream)
ImageR image_load_pnm(std::istream & str, const double scale=1);

// save the whole image (to file)
void image_save_pnm(const ImageR & im, const std::string & file,
                     const Opt & opt = Opt());

// save the whole image (to std::ostream)
void image_save_pnm(const ImageR & im, std::ostream & str,
                     const Opt & opt = Opt());

#endif
