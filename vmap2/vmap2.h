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

#define VMAP2DB_EXT  ".vmap2db"
#define VMAP2GH_EXT  ".vmap2gh"

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

  /// filenames (empty for in-memory database)
  std::string dbname;
  std::string ghname;

public:

  // Constructor. If <name> is empty create VMap2 in-memory storage.
  // It not, open BerkleyDB storage (create if needed).
  // <name> is database name with or without .vmap2db extension.
  VMap2(const std::string & name = std::string(), const bool create = false);

  ~VMap2() {}

  // remove database files
  static void remove_db(const std::string & dbname);

  /// Rebuild geohash database
  void geohash_rebuild();

  /// Dump geohash database
  void geohash_dump() {geohash->dump();}

  /// Add new object to the map, return object ID.
  uint32_t add(const VMap2obj & o);

  // Check if line l is a hole inside some object of type t.
  // If yes, insert it into the object. Return true if line was inserted.
  // Function is_hole from geom module is used for tests.
  // This function is needed for importing formats which do not support
  // multi-segment objects (fig, gpx).
  bool try_add_hole(const uint32_t t, const dLine & l);

  // Collect all holes (objects located within the first loop of obj,
  // having same type as obj and no name and comment). Holes are deleted
  // from the map and transferred into obj.
  // Function is_hole from geom module is used for tests.
  // This function is needed for importing formats which do not support
  // multi-segment objects (fig, gpx).
  void try_colect_holes(VMap2obj & obj);

  /// Rewrite existing object (add if not exist)
  void put(const uint32_t id, const VMap2obj & o);

  /// Read an object.
  VMap2obj get(const uint32_t id);

  /// Delete an object (error if not exist).
  void del(const uint32_t id);


  /// Number of objects
  size_t size() const;

  /// Find objects with given type and range
  std::set<uint32_t> find(VMap2objClass cl, uint16_t tnum, const dRect & range) {
    return geohash->get(range, (cl  << 24) | tnum); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t type, const dRect & range) {
    return geohash->get(range,type); }

  /// Find objects with given type (string representation) and range
  std::set<uint32_t> find(const std::string & type, const dRect & range) {
    return geohash->get(range, VMap2obj::make_type(type)); }

  /// Find objects with given type
  std::set<uint32_t> find(uint32_t type) {
    return geohash->get(type); }

  /// Find objects with given type (string representation)
  std::set<uint32_t> find(const std::string & type) {
    return geohash->get(VMap2obj::make_type(type)); }


  /// get all object types in the database
  std::set<uint32_t> get_types() { return geohash->get_types();}

  /// get map bounding box (extracted from geohash data)
  dRect bbox() { return geohash->bbox();}

  /// get filename (empty for in-memory database)
  public: std::string get_dbname() const {return dbname;}
  public: std::string get_ghname() const {return ghname;}


  // Functions for getting all elements.
  // It's too complicated to write proper iterators for multiple derived classes
  // (note: we should not care what are iterator classes, but it's impossible to
  //  iterators from different classes).
  // But here I need a much simpler thing: iterating through all objects.
  void iter_start(); // reset the iterator
  std::pair<uint32_t, VMap2obj> iter_get_next(); // get next object
  bool iter_end(); // is it end, or we can get one more object?

  /// Read/write text file into VMap2. Keep existing objects.
  void read(std::istream & s);
  void read(const std::string & file);

  /// Write VMap2 to text file. Drop ids, sort objects.
  void write(std::ostream & s);
  void write(const std::string & file);

  // Build a table "object ID -> label IDs".
  // For unconnected labels object ID will be 0xFFFFFFFF.
  std::multimap<uint32_t, uint32_t> find_refs(const double & dist1, const double & dist2);

  // find all objects with a given class
  std::set<uint32_t> find_class(const VMap2objClass cl);

};


#endif
