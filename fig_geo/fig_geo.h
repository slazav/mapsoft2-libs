#ifndef FIG_GEO_H
#define FIG_GEO_H

#include <vector>

#include "fig/fig.h"
#include "opt/opt.h"
#include "geo_data/geo_data.h"


#include "getopt/getopt.h"
// add options for converting geodata to fig
// groups: GEOFIG_REF, GEOFIG_DATA
void ms2opt_add_geofig_ref(GetOptSet & opts);
void ms2opt_add_geofig_data(GetOptSet & opts);

// Get geo-reference from fig:
// - Take map name from fig option `name`. Default - empty.
// - Take map projection from fig options `map_proj`, `lon0`.
//   Old-style projection settings are used (not many projections are supported).
//   Default projection: `tmerc`, default lon0 - autodetect for
//    map center calculated using refpoints.
// - Take reference points from "REF <lon> <lat>" objects.
// - Take border from "BRD <name>" objects.
//   Name of the border should match map name.
//   Multi-segment borders can be read from a few objects - this is new to mapsoft2
GeoMap fig_get_ref(const Fig & F);


/// Remove geo reference from Fig:
// - Name, map_proj and lon0 options.
// - All objects with REF ... comment.
// - All objects with BRD <name> comment (and matching name).
void fig_del_ref(Fig & F);


/// Add geo reference to Fig.
void fig_add_ref(Fig & F, const GeoMap & m, const Opt & o = Opt());

/// get waypoints, tracks, map refrences from fig
void fig_get_wpts(const Fig & F, const GeoMap & m, GeoData & d);
void fig_get_trks(const Fig & F, const GeoMap & m, GeoData & d);
void fig_get_maps(const Fig & F, const GeoMap & m, GeoData & d);

/// remove waypoins, tracks, maps, or map borders:
void fig_del_wpts(Fig & F);
void fig_del_trks(Fig & F);
void fig_del_maps(Fig & F);

/// Add waypoints or tracks from to fig
void fig_add_wpts(Fig & F, const GeoMap & m, const GeoData & d, const Opt & o = Opt());
void fig_add_trks(Fig & F, const GeoMap & m, const GeoData & d, const Opt & o = Opt());
void fig_add_maps(Fig & F, const GeoMap & m, const GeoData & d, const Opt & o = Opt());

#endif
