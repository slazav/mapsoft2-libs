#ifndef MAPDB_STORAGE_MEM_H
#define MAPDB_STORAGE_MEM_H

#include <string>
#include <map>
#include <set>

#include "mapdb_obj.h"
#include "mapdb_storage.h"
#include "geohash/storage.h"

/*********************************************************************/
// MapDBStorageMem -- memory implementation for MapDBStorage

class MapDBStorageMem : public MapDBStorage{
private:

  std::map<uint32_t, MapDBObj> objects;
  GeoHashStorage geohash;

public:

  /// Add new object to the map, return object ID.
  uint32_t add(const MapDBObj & o) override;

  /// Rewrite existing object (add if not exist)
  void put(const uint32_t id, const MapDBObj & o) override;

  /// Read an object.
  MapDBObj get(const uint32_t id) override;

  /// Delete an object (error if not exist).
  void del(const uint32_t id) override;


  /// Find objects with given type and range
  std::set<uint32_t> find(MapDBObjClass cl, uint16_t tnum, const dRect & range) override{
    return geohash.get(range, (cl  << 24) | tnum); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t type, const dRect & range) override{
    return geohash.get(range,type); }

  /// get all object types in the database
  std::set<uint32_t> get_types() override { return geohash.get_types();}

  /// get map bounding box (extracted from geohash data)
  dRect bbox() override { return geohash.bbox();}

};


#endif
