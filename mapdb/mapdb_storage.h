#ifndef MAPDB_STORAGE_H
#define MAPDB_STORAGE_H

#include <set>
#include "mapdb_obj.h"

/*********************************************************************/
// MapDBStorage -- interface class for map object storage

class MapDBStorage {
public:

  /// Add new object to the map, return object ID.
  virtual uint32_t add(const MapDBObj & o) = 0;

  /// Rewrite existing object (add if not exist)
  virtual void put(const uint32_t id, const MapDBObj & o) = 0;

  /// Read an object.
  virtual MapDBObj get(const uint32_t id) = 0;

  /// Delete an object (error if not exist).
  virtual void del(const uint32_t id) = 0;


  /// Find objects with given type and range
  virtual std::set<uint32_t> find(MapDBObjClass cl, uint16_t tnum, const dRect & range) = 0;

  /// Find objects with given type and range
  virtual std::set<uint32_t> find(uint32_t type, const dRect & range) = 0;

  /// get all object types in the database
  virtual std::set<uint32_t> get_types() = 0;

  /// get map bounding box (extracted from geohash data)
  virtual dRect bbox() = 0;

};

#endif
