#include <cstring>
#include <set>
#include <map>
#include <string>
#include <map>
#include <db.h>

#include "err/err.h"
#include "geohash/geohash.h"
#include "geohash/storage.h"
#include "db_geohash.h"
#include "db_tools.h"

// Max hash length. 12 gives 0.1m accuracy
#define HASHLEN 12

/**********************************************************/
GeoHashDB::GeoHashDB(std::string fname, const char *dbname, bool create) {
  // set flags
  int open_flags = create? DB_CREATE:0;

  /* Initialize the DB handle */
  DB *dbp;
  int ret = db_create(&dbp, NULL, 0);
  if (ret != 0) throw Err() << "db_geohash: " << db_strerror(ret);
  db.reset(dbp, bdb_close);

  // allow duplicates
  ret = dbp->set_flags(dbp, DB_DUPSORT);
  if (ret != 0) throw Err() << "db_geohash: " << db_strerror(ret);

  /* Open the database */
  ret = dbp->open(dbp,    /* Pointer to the database */
                  NULL,          /* Txn pointer */
                  fname.c_str(), /* file */
                  dbname,        /* database (can be NULL)*/
                  DB_BTREE,      /* Database type (using btree) */
                  open_flags,    /* Open flags */
                  0644);         /* File mode*/
  if (ret != 0)
    throw Err() << "db_geohash: " << fname << ": " << db_strerror(ret);
}

// add an object
void
GeoHashDB::put(const uint32_t id, const dRect & range, const uint32_t type) {
  if (range.is_empty()) return;
  std::set<std::string> hashes = GEOHASH_encode4(range, HASHLEN);
  for (auto const & h:hashes) {
    std::string s = join_type(type,h);
    DBT k = mk_dbt(s);
    DBT v = mk_dbt(&id);
    // std::cerr << "PUT [" << h << "] " << id << "\n";
    // do nothing if key/value pair already exists
    int ret = ((DB*)db.get())->put((DB*)db.get(), NULL, &k, &v, DB_NODUPDATA);
    if (ret!=0 && ret!=DB_KEYEXIST) throw Err() << "db_geohash::put: " << db_strerror(ret);
  }
}


// delete an object
void
GeoHashDB::del(const uint32_t id, const dRect & range, const uint32_t type) {
  if (range.is_empty()) return;
  std::set<std::string> hashes = GEOHASH_encode4(range, HASHLEN);
  DBC *curs = NULL;
  try {
    // get cursor
    ((DB*)db.get())->cursor((DB*)db.get(), NULL, &curs, 0);
    if (curs==NULL) throw Err() << "db_geohash: can't get a cursor";

    for (auto const & h:hashes) {
      DBT k = mk_dbt(join_type(type,h));
      DBT v = mk_dbt(&id);
      int ret = curs->get(curs, &k, &v, DB_GET_BOTH);
      if (ret == DB_NOTFOUND) continue;
      if (ret != 0)
        throw Err() << "db_geohash::del " << db_strerror(ret);
      ret = curs->del(curs,0);
      if (ret != 0) throw Err() << "db_geohash::del " << db_strerror(ret);
      // std::cerr << "DEL [" << h << "] " << id << "\n";
    }
  }
  catch (Err & e){
    if (curs) curs->close(curs);
    throw;
  }
  if (curs) curs->close(curs);
}


// get all types
std::set<uint32_t>
GeoHashDB::get_types() const {
  DBT k = mk_dbt();
  DBT v = mk_dbt();

  std::set<uint32_t> ret;

  int fl = DB_FIRST; // get first
  DBC *curs = NULL;
  try {
    // get cursor
    ((DB*)db.get())->cursor((DB*)db.get(), NULL, &curs, 0);
    if (curs==NULL) throw Err() << "db_geohash: can't get a cursor";

    while (1){
      // get next record
      int res = curs->get(curs, &k, &v, fl);
      if (res == DB_NOTFOUND) break;
      if (res!=0) throw Err() << "db_geohash: " << db_strerror(res);

      // extract type
      if (k.size < sizeof(uint32_t))
        throw Err() << "db_geohash: bad value";
      uint32_t type = *(uint32_t*)k.data;
      ret.insert(type);

      // set key to type+1 and repeat search
      *(uint32_t*)k.data = type+1;
      fl=DB_NEXT; // switch to DB_NEXT
    }
  }
  catch (Err & e){
    if (curs) curs->close(curs);
    throw;
  }
  if (curs) curs->close(curs);
  return ret;
}


// get range of the largest geohash
dRect
GeoHashDB::bbox() const {
  DBT k = mk_dbt();
  DBT v = mk_dbt();

  dRect ret;

  int fl = DB_FIRST; // get first
  DBC *curs = NULL;
  try {
    // get cursor
    ((DB*)db.get())->cursor((DB*)db.get(), NULL, &curs, 0);
    if (curs==NULL) throw Err() << "db_geohash: can't get a cursor";

    while (1){
      // get next record
      int res = curs->get(curs, &k, &v, fl);
      if (res == DB_NOTFOUND) break;
      if (res!=0) throw Err() << "db_geohash: " << db_strerror(res);

      // extract type
      if (k.size < sizeof(uint32_t))
        throw Err() << "db_geohash: bad value";
      std::string hash((char*)k.data+sizeof(uint32_t), k.size-sizeof(uint32_t));
      ret.expand(GEOHASH_decode(hash));
      auto type = *(uint32_t*)k.data;
      k = mk_dbt(join_type(type, hash+(char)('z'+1)) );
      fl=DB_SET_RANGE; // switch to DB_NEXT
    }
  }
  catch (Err & e){
    if (curs) curs->close(curs);
    throw;
  }
  if (curs) curs->close(curs);
  return ret;
}


// dump database
void
GeoHashDB::dump() const {
  DBT k = mk_dbt();
  DBT v = mk_dbt();

  dRect ret;

  int fl = DB_FIRST; // get first
  DBC *curs = NULL;
  try {
    // get cursor
    ((DB*)db.get())->cursor((DB*)db.get(), NULL, &curs, 0);
    if (curs==NULL) throw Err() << "db_geohash: can't get a cursor";

    while (1){
      // get next record
      int res = curs->get(curs, &k, &v, fl);
      if (res == DB_NOTFOUND) break;
      if (res!=0) throw Err() << "db_geohash: " << db_strerror(res);

      // extract type
      if (k.size < sizeof(uint32_t))
        throw Err() << "db_geohash: bad value";

      std::string hash((char*)k.data+sizeof(uint32_t), k.size-sizeof(uint32_t));
      auto  type = *(uint32_t*)k.data;
      auto  id   = *(uint32_t*)v.data;
      std::cout << id << "\t" << type << "\t" << hash << "\n";

      fl=DB_NEXT; // switch to DB_NEXT
    }
  }
  catch (Err & e){
  if (curs) curs->close(curs);
    throw;
  }
  if (curs) curs->close(curs);
}

// get objects for geohash
std::set<uint32_t>
GeoHashDB::get_hash(const std::string & hash0, bool exact) const {
  DBT k = mk_dbt(hash0);
  DBT v = mk_dbt();

  std::set<uint32_t> ret;

  int fl = DB_SET_RANGE; // first get t >= t1
  DBC *curs = NULL;
  try {
    // get cursor
    ((DB*)db.get())->cursor((DB*)db.get(), NULL, &curs, 0);
    if (curs==NULL) throw Err() << "db_geohash: can't get a cursor";

    while (1){
      // get new record (hash>=hash0)
      int res = curs->get(curs, &k, &v, fl);
      if (res == DB_NOTFOUND) break;
      if (res!=0) throw Err() << "db_geohash: " << db_strerror(res);

      // check what we found
      if (v.size != sizeof(uint32_t) || k.size < sizeof(uint32_t))
        throw Err() << "db_geohash: bad database record";
      std::string hash((char*)k.data, (char*)k.data+k.size);
      if (exact && hash!=hash0) break;

      if (hash.size()>=hash0.size() &&
          hash.compare(0,hash0.size(), hash0)==0){
        ret.insert(*(uint32_t*)v.data);
        fl=DB_NEXT; // switch to DB_NEXT and repeat
        continue;
      }
      break;
    }
  }
  catch (Err & e){
    if (curs) curs->close(curs);
    throw;
  }
  if (curs) curs->close(curs);
  return ret;
}
