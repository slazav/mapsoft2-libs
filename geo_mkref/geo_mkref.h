#ifndef MK_REF_H
#define MK_REF_H

#include "geo_data/geo_data.h"
#include "opt/opt.h"

/********
Create map reference from options.

Following map fields are set:
  map.name -- map name
  map.proj -- map projection (string of libproj parameters)
  map.image_dpi -- map resolution, dots per inch
  map.image_size -- map image size (in pixels)
  map.border -- map border (in pixels)
  map.ref -- four reference points

One can use following code to create map reference:
  GeoMap ref = geo_mkref_opts(o);  // create map from --mkref options if possible
  if (ref.empty())
    ref = geo_mkref_data(data, o);
  geo_mkref_brd(ref, o); // update border;

*/

/********************************************************************/
#include "getopt/getopt.h"

// add MKREF_OPTS, MKREF_BRD, MKREF_DATA group of options
void ms2opt_add_mkref_opts(GetOptSet & opts);
void ms2opt_add_mkref_brd(GetOptSet & opts);
void ms2opt_add_mkref_data(GetOptSet & opts);

/********************************************************************/

// Make map reference from --mkref options, return empty
// map if --mkref does not exist.
GeoMap geo_mkref_opts(const Opt & o);

// update map border from options
// if reference is empty, use WGS84 coordinates
void geo_mkref_brd(GeoMap & ref, const Opt & o);

// make map reference from data
GeoMap geo_mkref_data(const GeoData & data, const Opt & o);

// create default web-mercator projection (256x256 image)
GeoMap geo_mkref_web();


#endif
