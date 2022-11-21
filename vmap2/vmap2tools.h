#include "vmap2.h"
#include <string>

// Replace labels in mapn with labels from mapo
void do_keep_labels(VMap2 & mapo, VMap2 & mapn);

// Replace all objects in mapn with objects from mapo
// except ones with the given tag
void do_update_tag(VMap2 & mapo, VMap2 & mapn, const std::string & tag);

// Try to fix rounding errors in mapn using information from mapo.
void do_fix_rounding(VMap2 & mapo, VMap2 & mapn, double D);
