#ifndef VMAP2TYPES_H
#define VMAP2TYPES_H

#include <string>
#include <list>
#include <map>
#include "geo_data/geo_data.h"

// Information about object type.
// Used for converting objects to different vector formats.
class VMap2type {
public:
  std::string name;     // type short name
  std::string comm;     // type description
  std::string fig_mask; // mask for FIG format
  std::string fig_pic;  // picture for FIG format
  int mp_start, mp_end; // start/end level for MP format
  int label_type;       // type for related lables

  VMap2type(): mp_start(0), mp_end(0), label_type(-1) {}
};

// All object types
class VMap2types : public std::map<int, VMap2type> {
public:
  // load from a file
  void load(const std::string & fname);
};

#endif