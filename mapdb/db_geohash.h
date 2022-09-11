#ifndef DB_GEOHASH_H
#define DB_GEOHASH_H

#include <memory>
#include <set>
#include <stdint.h>
#include "geom/rect.h"

// Geohash database for spatial indexing.
// One can add "object ID -- object type -- object range"
// combinations and then query for objects of a certain type
// which may appear in a given coordinate range.

// There are two implementations: Memory and BerkleyDB.
// Use empty fname parameter in the constructor to use memory implementation.

// Memory version is almost same as GeoHashStorage class in geohash/
// (TODO: remove one)

class GeoHashDB {
  private:
    class Impl;
    class ImplDB;
    class ImplMem;
    std::shared_ptr<Impl> impl;

  public:

    GeoHashDB(std::string fname = std::string(), const char *dbname = NULL, bool create = true);
    ~GeoHashDB();

    // Get id of objects of certain type which may be found in the range.
    // If bbox is empty return zero set.
    std::set<uint32_t> get(const uint32_t type, const dRect & range);

    // Add an object with id, type and range.
    // If bbox is empty do nothing.
    void put(const uint32_t id, const uint32_t type, const dRect & range);

    // Delete an object with id, type and range.
    // If the record does not exist do nothing.
    void del(const uint32_t id, const uint32_t type, const dRect & range);

    // get all object types in the database
    std::set<uint32_t> get_types();

    // get range of the largest geohash
    dRect bbox();

    // dump database to stdout
    void dump();

};

#endif
