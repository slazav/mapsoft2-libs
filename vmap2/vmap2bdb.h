#ifndef VMAP2BDB_H
#define VMAP2BDB_H

#include <string>
#include <set>

#include "vmap2.h"
#include "db_simple.h"
#include "db_geohash.h"

/*********************************************************************/
// VMap2bdb -- BerkleyDB storage for map objects

class VMap2bdb : public VMap2{
private:

  DBSimple   objects; // object data
  GeoHashDB  geohash; // geohashes for spatial indexing

public:

  // Constructor. Open all databases
  VMap2bdb(std::string name, const bool create = true);

  // delete map databases
  static void delete_db(std::string name);

  /// Add new object to the map, return object ID.
  uint32_t add(const VMap2obj & o) override;

  /// Rewrite existing object, update geohashes.
  /// If object does not exist it will be added anyway.
  void put(const uint32_t id, const VMap2obj & o) override;

  /// Read an object.
  VMap2obj get(const uint32_t id) override;

  /// Delete an object.
  /// If the object does not exist throw an error.
  void del(const uint32_t id) override;

  /// Find objects with given type and range
  std::set<uint32_t> find(VMap2objClass cl, uint16_t tnum, const dRect & range) override {
    return geohash.get(range, (cl  << 24) | tnum); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t type, const dRect & range) override {
    return geohash.get(range,type); }

  /// get all object types in the database
  std::set<uint32_t> get_types() override  {
    return geohash.get_types();}

  /// get map bounding box (extracted from geohash data)
  dRect bbox() override { return geohash.bbox();}


  /// Iterating through all objects:
  void iter_start() override {i=objects.begin();}
  std::pair<uint32_t, VMap2obj> iter_get_next() override {
    return std::make_pair(i->first, VMap2obj::unpack(i->second));}
  bool iter_end() override {return i!=objects.end();}

  private:
    DBSimple::iterator i;


};

#endif
