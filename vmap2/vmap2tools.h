#include "vmap2.h"
#include "vmap2types.h"
#include <string>

// Replace labels in mapn with labels from mapo
void do_keep_labels(VMap2 & mapo, VMap2 & mapn);

// Replace all objects in mapn with objects from mapo
// except ones with the given tag
void do_update_tag(VMap2 & mapo, VMap2 & mapn, const std::string & tag);

// Try to fix rounding errors in mapn using information from mapo.
void do_fix_rounding(VMap2 & mapo, VMap2 & mapn, double D);

// Join line objects. D - max distance between ends, A - max
// join angle in degrees.
void do_join_lines(VMap2 & map, const double D, const double A);

// Update labels
void do_update_labels(VMap2 & map, const VMap2types & types);

// Crop map with a rectangleUpdate labels
void do_crop_rect(VMap2 & map, const dRect & r);
