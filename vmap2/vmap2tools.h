#include "vmap2.h"
#include "vmap2types.h"
#include <string>

// Replace labels in mapn with labels from mapo
void do_keep_labels(VMap2 & mapo, VMap2 & mapn);

// Assign a tag to all objects in mapn. Add all objects from mapo without this tag
void do_replace_tag(VMap2 & mapo, VMap2 & mapn, const std::string & tag);

// Replace all objects in mapn with objects from mapo
// except ones with the given type
void do_replace_type(VMap2 & mapo, VMap2 & mapn, const uint32_t type);

// Keep all objects in mapn, transfer objects of other types from mapo
void do_replace_types(VMap2 & mapo, VMap2 & mapn);

// Try to fix rounding errors in mapn using information from mapo.
void do_fix_rounding(VMap2 & mapo, VMap2 & mapn, double D);

// Join line objects. D - max distance between ends, A - max
// join angle in degrees.
void do_join_lines(VMap2 & map, const double D, const double A);

// Reduce number of points in lines and polygons
void do_filter_pts(VMap2 & map, const double D);

// Update labels: find labels for each object, add missing labels,
// remove labels without objects, update label type, name, and reference.
// If label_names = true, get names from labels, not from objects.
// (Useful for updating map from a source (e.g. gpx) where names are missing.)
void do_update_labels(VMap2 & map, const VMap2types & types, const bool label_names=false);

// Crop map with a rectangle
void do_crop_rect(VMap2 & map, const dRect & r, const bool crop_labels);

// Crop to a finnish nomenclature map (e.g "V51")
void do_crop_nom_fi(VMap2 & map, const std::string & name, const bool crop_labels);

/************************************************************/
// Additional functions, used for custom filters

// For each object of <type> move object ends towards nearest point
// or extend/reduce end segments to nearest segment of an object from <types>.
void vmap_move_ends(VMap2 & vmap, const std::string & type0, const std::list<std::string> types, double r);

// remove 0- and 1- point segments and duplicated points for a given type
void vmap_rem_dups(VMap2 & vmap, const std::string & type, double r);

// Join objects with best matching angles (with min_ang limit)
// (should be used after after vmap_move_ends and vmap_rem_dups).
// Differences with do_join_lines():
//  - work with selected type
//  - choose best angle
//  - ignore object names, comments (can be lost!)
// Written for merging mountain ranges
void vmap_join(VMap2 & vmap, const std::string & type, double min_ang);
