#ifndef MK_REF_H
#define MK_REF_H

#include "geo_data.h"
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

/*
Example of using all three functions (from ms2render.cpp):

  GeoMap ref = geo_mkref_opts(O);  // create ref from options if possible
  if (ref.empty())
    ref = geo_mkref_data(data, O); // if not, create from geo_data
  if (ref.empty())
    ref=ref_v;                     // if not, use some external default
  geo_mkref_brd(ref, O);           // update border from options

*/


// Make map reference from --mkref options, return empty
// map if --mkref does not exist.
GeoMap geo_mkref_opts(const Opt & o);

// update map border from options
// if reference is empty, use WGS84 coordinates
void geo_mkref_brd(GeoMap & ref, const Opt & o);

// make map reference from data
GeoMap geo_mkref_data(const GeoData & data, const Opt & o);

/********************************************************************/
// Functions for individual reference types

// Soviet standard map (e.g n37-001).
//   name -- map name
//   dpi, mag -- map dpi and additional magnification
//   north -- north/grid irientation
//   mt, ml, mr, mb -- top, left, right, bottom margins [pixels]
GeoMap geo_mkref_nom(const std::string & name,
  double dpi=300.0, double mag=1.0, bool north = false,
  int mt=0, int ml=0, int mr=0, int mb=0);

// Finnish standard map (e.g V51)
//   name -- map name
//   dpi, mag -- map dpi and additional magnification
//   mt, ml, mr, mb -- top, left, right, bottom margins [pixels]
GeoMap geo_mkref_nom_fi(const std::string & name,
  double dpi=300.0, double mag=1.0, bool north = false,
  int mt=0, int ml=0, int mr=0, int mb=0);

// Rectangular range of Google/TMS tiles
GeoMap geo_mkref_tiles(const iRect & tile_range, int z, bool G,
   const dMultiLine & border, double mag=1.0);

// create default web-mercator projection (256x256 image)
GeoMap geo_mkref_web();


#endif
