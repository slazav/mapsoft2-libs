#ifndef VMAP2_H
#define VMAP2_H

#include <set>
#include <memory>
#include "vmap2obj.h"

// BerkleyDB storage
#include "db_simple.h"
#include "db_geohash.h"

// in-memory geohash storage
#include "geohash/storage.h"

/*********************************************************************/
// VMap2 -- interface class for map object storage

class VMap2 {

  // BerkleyDB or in-memory storage
  bool bdb;

  /// data for BerkleyDB storage (empty by default)
  std::shared_ptr<DBSimple>  objects_bdb; // object data
  DBSimple::iterator it_bdb;

  // data for in-memory storage
  std::map<uint32_t, VMap2obj> objects_mem;
  std::map<uint32_t, VMap2obj>::iterator it_mem;

  // geohashes for spatial indexing
  std::shared_ptr<GeoHashStorage> geohash; 

  /// filename (empty for in-memory database)
  std::string fname;

public:

  // Constructor. If fname is empty create VMap2 in-memory storage.
  // It not, open BerkleyDB storage (create if needed).
  VMap2(const std::string & fname = std::string(), const bool create = false);

  ~VMap2() {}

  /// Add new object to the map, return object ID.
  uint32_t add(const VMap2obj & o);

  /// Rewrite existing object (add if not exist)
  void put(const uint32_t id, const VMap2obj & o);

  /// Read an object.
  VMap2obj get(const uint32_t id);

  /// Delete an object (error if not exist).
  void del(const uint32_t id);


  /// Find objects with given type and range
  std::set<uint32_t> find(VMap2objClass cl, uint16_t tnum, const dRect & range) {
    return geohash->get(range, (cl  << 24) | tnum); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t type, const dRect & range) {
    return geohash->get(range,type); }

  /// get all object types in the database
  std::set<uint32_t> get_types() { return geohash->get_types();}

  /// get map bounding box (extracted from geohash data)
  dRect bbox() { return geohash->bbox();}


  /// get filename (empty for in-memory database)
  public: std::string get_fname() const {return fname;}


  // Functions for getting all elements.
  // It's too complicated to write proper iterators for multiple derived classes
  // (note: we should not care what are iterator classes, but it's impossible to
  //  iterators from different classes).
  // But here I need a much simpler thing: iterating through all objects.
  void iter_start(); // reset the iterator
  std::pair<uint32_t, VMap2obj> iter_get_next(); // get next object
  bool iter_end(); // is it end, or we can get one more object?

};

#endif
