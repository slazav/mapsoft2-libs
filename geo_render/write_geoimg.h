#ifndef WRITE_GEOIMG_H
#define WRITE_GEOIMG_H

#include <string>
#include "opt/opt.h"
#include "geo_data/geo_data.h"
#include "getopt/getopt.h"
#include "viewer/gobj.h"


// add options for saving images and map files (including --map)
// groups: MKREF, IMAGE, IMAGE_CMAP, DRAWTRK, GRAWMAP, DRAWWPT, GEO_O
void ms2opt_add_geoimg(GetOptSet & opts);

// Write geodata to a raster file.
// Throw Err with code=-2 if format is unknown.
// Interface is similat to `write_geo` in modules/geo_data
void write_geoimg(const std::string & fname, GObj & obj, const GeoMap & ref, const Opt & opts);

#endif
