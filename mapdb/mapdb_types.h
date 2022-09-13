#ifndef MAPDB_TYPES_H
#define MAPDB_TYPES_H

#include <string>
#include <list>
#include <map>
#include "geo_data/geo_data.h"

// Information about object type
class MapBDTypeInfo {
public:
  std::string name;     // type short name
  std::string comm;     // type description
  std::string fig_mask; // mask for FIG format
  int mp_sl, mp_el;     // start/end level for MP format
  int text_type;        // type for related text objects

  MapBDTypeInfo(): mp_sl(0), mp_el(0), text_type(-1) {}
};

// All object types
class MapDBTypeMap : public std::map<int, MapBDTypeInfo> {
public:
  // load from a file
  void load(const std::string & fname);
};

#endif