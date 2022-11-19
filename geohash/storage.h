#ifndef GEOHASH_STORAGE_H
#define GEOHASH_STORAGE_H

#include <memory>
#include <map>
#include <set>
#include "geom/rect.h"

/**********************************************************/
// In-memory (std::map-based) geohash database for spatial indexing

class GeoHashStorage {
    std::multimap<std::string, uint32_t> db;
    dRect BB; // coordinate range; empty for default [-180..180, -90..90]

  public:

    // Get id of objects which may be found in the range
    virtual std::set<uint32_t> get(const dRect & range, const uint32_t type=0) const;

    // Get id of all objects with one type
    virtual std::set<uint32_t> get(const uint32_t type) const;

    // add an object
    virtual void put(const uint32_t id, const dRect & range, const uint32_t type=0);

    // delete an object
    virtual void del(const uint32_t id, const dRect & range, const uint32_t type=0);

    // get all types
    virtual std::set<uint32_t> get_types() const;

    // get range of the largest geohash
    virtual dRect bbox() const;

    // dump database
    virtual void dump() const;

    // get objects for geohash
    virtual std::set<uint32_t> get_hash(const std::string & hash0, bool exact) const;

    // we use type+geohash key; type should be written in big endian order
    static std::string join_type(const uint32_t type, const std::string & hash);
    static uint32_t extract_type(char *data);

    // set bbox for coordinate transformation
    virtual void set_db_range(const dRect & range_){ BB = range_; }
};


#endif
