#ifndef GEO_LON0_H
#define GEO_LON0_H

#include "geom/point.h"
#include "geom/multiline.h"

/********************************************************************/
/// Distance in m between two points (Haversine formula, altitude is ignored).
double geo_dist_2d(const dPoint &p1, const dPoint &p2);

// Line length in m (Haversine formula, altitude is ignored)
template<typename CT, typename PT>
double
geo_length_2d(const Line<CT,PT> & l) {
  double ret=0;
  for (size_t i=0; i+1<l.size(); i++)
    ret += geo_dist_2d(l[i], l[i+1]);
  return ret;
}

// MultiLine length in m (Haversine formula, altitude is ignored)
template<typename CT, typename PT>
double
geo_length_2d(const MultiLine<CT,PT> & ml) {
  double ret=0;
  for(auto const & l:ml) ret+=geo_length_2d(l);
  return ret;
}

/// Go from point p1 using bearing th (deg) and distance d (m)
dPoint geo_bearing_2d(const dPoint &p1, const double th, const double d);


/// Read a figure from the string and get its bounding box.
/// Same as figure_line from poly_tools.h, but can also read data from a file (tracks and waypoints).
dMultiLine figure_geo_line(const::std::string &str);

/********************************************************************/
// Functions for working with cordinate prefixes and lon0 parameter
// in soviet map coordinates

// Get lon0 (-3,3,9,15...).
// Example: 0.25 -> 3
int lon2lon0(const double lon);

// Get prefix (1..60) for given longitude.
// Example 38.5 -> 7
int lon2pref(const double lon);

// Extract prefix from coordinate and return lon0.
// Example: 7800000 -> 39
int crdx2lon0(const double X);

// Extract non-prefix part.
// Example: 7800000 -> 800000
double crdx2nonpref(const double X);

// To be removed!
std::string GEO_PROJ_SU(double lon);

// Ordnance Survey letter codes. "code" -> x,y (in 100 km units)
// https://en.wikipedia.org/wiki/Ordnance_Survey_National_Grid
extern std::map<std::string, std::pair<int, int> > os_codes;

// Convert OS grid reference to a x,y point
dPoint os_to_pt(const std::string & s);

// Convert extended nomenclature name to WGS rectangle.
dRect nom_to_wgs(const std::string & name);

/********************************************************************/

#endif

