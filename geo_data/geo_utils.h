#ifndef GEO_LON0_H
#define GEO_LON0_H

#include "geom/point.h"
#include "geom/multiline.h"

/********************************************************************/
/// Distance in m between two points (Haversine formula, altitude is ignored).
double geo_dist_2d(const dPoint &p1, const dPoint &p2);

/// Haversine distance to the nearest point of a dLine or dMultiLine.
/// Similar to nearest_dist() from geom/poly_tools.h
double geo_nearest_dist(const dLine & l, const dPoint & pt);
double geo_nearest_dist(const dMultiLine & ml, const dPoint & pt);

/// Nearest point of a dLine or dMultiLine (Haversine distance).
/// Similar to nearest_pt() from geom/poly_tools.h
dPoint geo_nearest_pt(const dLine & l, const dPoint & pt);
dPoint geo_nearest_pt(const dMultiLine & ml, const dPoint & pt);


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

/********************************************************************/

#endif

