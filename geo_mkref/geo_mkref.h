#ifndef MK_REF_H
#define MK_REF_H

#include "geo_data/geo_data.h"
#include "opt/opt.h"

/********
Create map reference from options.

In many cases it is needed to draw some geodata on a raster image. For
this one have to specify the image reference (GeoMap object):
datum and projection, position, scale, border etc. This function can
build certain types of references using test options.

Options:

--mkref nom -- make Soviet nomenclature map.
  --name <name> -- map name (mapsoft2 extensions are allowed).
  --dpi <double> -- map resolution in dots per inch (default 300).
  --mag <factor> -- rescale map without changing dpi setting (default 1).
  --margins <pixels>
  --top_margin <pixels>
  --left_margin <pixels>
  --right_margin <pixels>
  --bottom_margin <pixels>
    Set margins between map border and image boundary.

--mkref tms_tile
--mkref google_tile -- create a single tile or rectangular area with multiple tiles

  --zindex <integer>  -- tile zindex

  --tiles <point or rectangle>
      Tile or tile range. If single point is given then non-zero z-coordinate
       overrides zindex option. --tiles "[1,2,3]" is same as --tiles "[1,2]" --zindex 0.

  --coords_wgs <wgs84 point, rectangle, line, or multiline>
      Build a map which covers given figure.

  --border_wgs <wgs84 line or multiline>
      Set map border.

--mkref proj
  --proj <proj string> -- set datum and projection (libproj parameter string), default: "+datum=WGS84 +proj=lonlat"
  --coords <rect, line or multiline in proj coords>
  --coords_wgs <rect, line or multiline in wgs84 coordinates>
       Set map area in projection or wgs84 coordinates
  --border <multiline in proj coords>
  --border_wgs <multiline in wgs84 coords>
       Set map border in projection or wgs84 coordinates
  --dpi
  --scale
      Scale (projection units per map cm), default 1000 for non-degree projections,
      1/100 for degree projections.
  --margins <pixels>
  --top_margin <pixels>
  --left_margin <pixels>
  --right_margin <pixels>
  --bottom_margin <pixels>
      Set map margins.

--image <name> --set image file name in the map

Following map fields are set:
  map.name -- map name
  map.image -- map name (from --image option)
  map.proj -- map projection (string of libproj parameters)
  map.image_dpi -- map resolution, dots per inch
  map.image_size -- map image size (in pixels)
  map.border -- map border (in pixels)
  map.ref -- four reference points
*/

/********************************************************************/
#include "getopt/getopt.h"

// add MS2OPT_GEO_MKREF options
void ms2opt_add_mkref(ext_option_list & opts);

/********************************************************************/

GeoMap geo_mkref(const Opt & o);

// try to get some information from GeoData if there is
// no mkref option
GeoMap geo_mkref(const GeoData & data, const Opt & o);

#endif
