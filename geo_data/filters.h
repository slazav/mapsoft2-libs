#ifndef GEO_FILTERS_H
#define GEO_FILTERS_H

#include <string>
#include "getopt/getopt.h"
#include "opt/opt.h"
#include "geo_data.h"


/********************************************************************/
#include "getopt/getopt.h"

// add GEOFLT group of options
void ms2opt_add_geoflt(GetOptSet & opts);

/********************************************************************/

void geo_filters(GeoData & data, const Opt & opt);

// filter_nom_brd -- set border for soviet nomenclature maps.
// Extended map names are allowed.
void filter_nom_brd(GeoData & data, const Opt & opt);

// skip some types of data
void filter_skip(GeoData & data, const Opt & opt);

// join all tracks, all waypoint lists, all map lists
void filter_join(GeoData & data, const Opt & opt);

// set name/comments
void filter_trk(GeoData & data, const Opt & opt);
void filter_wpt(GeoData & data, const Opt & opt);
void filter_map(GeoData & data, const Opt & opt);

// select_time

#endif
