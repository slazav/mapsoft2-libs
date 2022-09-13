#ifndef MAPDB_STORAGE_BDB_H
#define MAPDB_STORAGE_BDB_H

#include <string>
#include <set>

#include "mapdb_obj.h"
#include "mapdb_storage.h"
#include "db_simple.h"
#include "db_geohash.h"

/*********************************************************************/
// MapDBStorageBDB -- BerkleyDB storage for map objects

class MapDBStorageBDB : public MapDBStorage{
private:

  DBSimple   objects; // object data
  GeoHashDB  geohash; // geohashes for spatial indexing

public:

  // Constructor. Open all databases
  MapDBStorageBDB(std::string name, const bool create = true);

  // delete map databases
  static void delete_db(std::string name);

  /// Add new object to the map, return object ID.
  uint32_t add(const MapDBObj & o) override;

  /// Rewrite existing object, update geohashes.
  /// If object does not exist it will be added anyway.
  void put(const uint32_t id, const MapDBObj & o) override;

  /// Read an object.
  MapDBObj get(const uint32_t id) override;

  /// Delete an object.
  /// If the object does not exist throw an error.
  void del(const uint32_t id) override;

  /// Find objects with given type and range
  std::set<uint32_t> find(MapDBObjClass cl, uint16_t tnum, const dRect & range) override {
    return geohash.get(range, (cl  << 24) | tnum); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t type, const dRect & range) override {
    return geohash.get(range,type); }

  /// get all object types in the database
  std::set<uint32_t> get_types() override  {
    return geohash.get_types();}

  /// get map bounding box (extracted from geohash data)
  dRect bbox() override { return geohash.bbox();}

};

#endif
