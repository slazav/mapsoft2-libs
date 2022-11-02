#ifndef VMAP2MEM_H
#define VMAP2MEM_H

#include <string>
#include <map>
#include <set>

#include "vmap2.h"
#include "geohash/storage.h"

/*********************************************************************/
// VMap2mem -- memory implementation for VMap2

class VMap2mem : public VMap2{
private:

  std::map<uint32_t, VMap2obj> objects;
  GeoHashStorage geohash;


public:

  VMap2mem(): i(objects.begin()){}

  /// Add new object to the map, return object ID.
  uint32_t add(const VMap2obj & o) override;

  /// Rewrite existing object (add if not exist)
  void put(const uint32_t id, const VMap2obj & o) override;

  /// Read an object.
  VMap2obj get(const uint32_t id) override;

  /// Delete an object (error if not exist).
  void del(const uint32_t id) override;


  /// Find objects with given type and range
  std::set<uint32_t> find(VMap2objClass cl, uint16_t tnum, const dRect & range) override{
    return geohash.get(range, (cl  << 24) | tnum); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t type, const dRect & range) override{
    return geohash.get(range,type); }

  /// get all object types in the database
  std::set<uint32_t> get_types() override { return geohash.get_types();}

  /// get map bounding box (extracted from geohash data)
  dRect bbox() override { return geohash.bbox();}


  /// Iterating through all objects:
  void iter_start() override {i = objects.begin();}
  std::pair<uint32_t, VMap2obj> iter_get_next() override {return *i;}
  bool iter_end() override {return i == objects.end();}

private:
  std::map<uint32_t, VMap2obj>::iterator i;

};


#endif
