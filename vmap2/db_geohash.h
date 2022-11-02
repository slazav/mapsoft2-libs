#ifndef DB_GEOHASH_H
#define DB_GEOHASH_H

#include <memory>
#include <set>
#include <stdint.h>
#include "geom/rect.h"
#include "geohash/storage.h"

// BerkleyDB analog of GeoHashStorage from geohash/.
// One can add "object ID -- object type -- object range"
// combinations and then query for objects of a certain type
// which may appear in a given coordinate range.

/**********************************************************/
class GeoHashDB : public GeoHashStorage {
    std::shared_ptr<void> db;

  public:
    GeoHashDB(std::string fname, const char *dbname, bool create);

    // add an object
    void put(const uint32_t id, const dRect & range, const uint32_t type=0) override;

    // delete an object
    void del(const uint32_t id, const dRect & range, const uint32_t type=0) override;

    // get all types
    std::set<uint32_t> get_types() const override;

    // get range of the largest geohash
    dRect bbox() const override;

    // dump database
    void dump() const override;

    // get objects for geohash
    std::set<uint32_t> get_hash(const std::string & hash0, bool exact) const override;
};

#endif
