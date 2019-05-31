#ifndef DB_GEOHASH_H
#define DB_GEOHASH_H

#include <memory>
#include <set>
#include "geom/rect.h"

// Berkleydb-based geohash database for spatial indexing
class GeoHashDB {
  private:
    class Impl;
    std::unique_ptr<Impl> impl;

  public:

   GeoHashDB(const char *fbase, bool create);
   ~GeoHashDB();

   // add object with id and range.
   void put(const int id, const dRect & range);

   // get id of objects which may be found in the range
   std::set<int> get(const dRect & range);
};


#endif
