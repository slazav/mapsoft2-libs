#ifndef FIG_GEO_H
#define FIG_GEO_H

#include <vector>

#include "fig/fig.h"
#include "opt/opt.h"
#include "geo_data/geo_data.h"

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
void fig_add_ref(Fig & F, const GeoMap & m);

/*
  /// get waypoints, tracks, map refrences from fig
  void get_wpts(const fig_world & w, const g_map & m, geo_data & d);
  void get_trks(const fig_world & w, const g_map & m, geo_data & d);
  void get_maps(const fig_world & w, const g_map & m, geo_data & d);

  /// remove waypoins, tracks, maps, or map borders:
  void rem_wpts(fig_world & w);
  void rem_trks(fig_world & w);
  void rem_maps(fig_world & w);
  void rem_brds(fig_world & w);

  /// Add waypoints or tracks from to fig
  /// if raw = 1, no geofig comments are added
  void put_wpts(fig_world & w, const g_map & m, const geo_data & d, bool raw=true);
  void put_trks(fig_world & w, const g_map & m, const geo_data & d, bool raw=true);
*/

#endif
