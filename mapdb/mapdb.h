#ifndef MAPDB_H
#define MAPDB_H

#include <string>
#include <list>
#include <map>
#include <set>

#include "mapdb_obj.h"
#include "db_simple.h"
#include "db_geohash.h"

/*********************************************************************/
// MapDB -- a storage for map objects

class MapDB {
private:

  DBSimple   objects; // object data
  GeoHashDB  geohash; // geohashes for spatial indexing

public:

  // Constructor. Open all databases
  MapDB(std::string name, bool create = false);

  // delete map databases
  static void delete_db(std::string name);

  /// Add new object to the map, return object ID.
  uint32_t add(const MapDBObj & o);

  /// Rewrite existing object, update geohashes.
  /// If object does not exist it will be added anyway.
  void put(const uint32_t id, const MapDBObj & o);

  /// Read an object.
  MapDBObj get(const uint32_t id);

  /// Delete an object.
  /// If the object does not exist throw an error.
  void del(const uint32_t id);

  /// Find objects with given type and range
  std::set<uint32_t> find(MapDBObjClass cl, uint16_t tnum, const dRect & range){
    return geohash.get((cl  << 24) | tnum, range); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t type, const dRect & range){
    return geohash.get(type, range); }

  /// get all object types in the database
  std::set<uint32_t> get_types() {
    return geohash.get_types();}

  /// get map bounding box (extracted from geohash data)
  dRect bbox() { return geohash.bbox();}


  ///////////////
  /* Import/export */
  public:

  /// Import objects from MP file.
  void import_mp(
    const std::string & mp_file,
    const Opt & opts);

  /// Export objects to MP file.
  void export_mp(
    const std::string & mp_file,
    const Opt & opts);

  /// Import objects from VMAP file.
  void import_vmap(
    const std::string & vmap_file,
    const Opt & opts);

  /// Export objects to VMAP file.
  void export_vmap(
    const std::string & vmap_file,
    const Opt & opts);

};

// add option groups:
//   MAPDB_MP_IMP, MAPDB_MP_EXP, MAPDB_VMAP_IMP, MAPDB_VMAP_EXP

#include "getopt/getopt.h"

void ms2opt_add_mapdb_mp_imp(GetOptSet & opts);
void ms2opt_add_mapdb_mp_exp(GetOptSet & opts);

void ms2opt_add_mapdb_vmap_imp(GetOptSet & opts);
void ms2opt_add_mapdb_vmap_exp(GetOptSet & opts);

#endif
