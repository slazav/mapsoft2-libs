#include <cstring>
#include <set>
#include <map>
#include <string>
#include <map>
#include <db.h>

#include "err/err.h"
#include "geohash/geohash.h"
#include "db_geohash.h"
#include "db_tools.h"

// Max hash length. 12 gives 0.1m accuracy
#define HASHLEN 12


/**********************************************************/
// Implementation class (Base)
class GeoHashDB::Impl {
  public:

    // Get id of objects which may be found in the range
    // (same for all implementations, based on get_hash())
    virtual std::set<uint32_t> get(const uint32_t type, const dRect & range){
      std::set<uint32_t> ret;
      if (range.is_empty()) return ret;
      std::set<std::string> hashes = GEOHASH_encode4(range, HASHLEN);
      std::set<std::string> done;
      for (auto const & h:hashes) {
        for (size_t i=0; i<=h.size(); i++) {
          std::string hh = h.substr(0,i);
          if (done.count(hh)) continue; // do not repeat queries with same hash
          bool exact = i < h.size();  // for full hashes look also for smaller regions.
          done.insert(hh);
          //std::cerr << "GET [" << hh << "] " << exact << "\n";
          std::set<uint32_t> r = get_hash(join_type(type, hh), exact);
          ret.insert(r.begin(), r.end());
        }
      }
      return ret;
    }

    // add an object
    virtual void put(const uint32_t id, const uint32_t type, const dRect & range) = 0;

    // delete an object
    virtual void del(const uint32_t id, const uint32_t type, const dRect & range) = 0;

    // get all types
    virtual std::set<uint32_t> get_types() = 0;

    // get range of the largest geohash
    virtual dRect bbox() = 0;

    // dump database
    virtual void dump() = 0;

    // get objects for geohash
    virtual std::set<uint32_t> get_hash(const std::string & hash0, bool exact) = 0;

    // we use type+geohash key
    std::string join_type(const uint32_t type, const std::string & hash){
      std::ostringstream ss;
      ss.write((char*)&type, sizeof(type));
      ss.write(hash.data(), hash.size());
      return ss.str();
    }
};


/**********************************************************/
// Implementation class (Memory storage)
class GeoHashDB::ImplMem : public GeoHashDB::Impl {
  public:
    std::multimap<std::string, uint32_t> db;

    // add an object
    void put(const uint32_t id, const uint32_t type, const dRect & range) override {
      if (range.is_empty()) return;
      auto hashes = GEOHASH_encode4(range, HASHLEN);
      for (auto const & h:hashes) db.emplace(join_type(type,h), id);
    }

    // delete an object
    void del(const uint32_t id, const uint32_t type, const dRect & range) override {
      if (range.is_empty()) return;
      auto hashes = GEOHASH_encode4(range, HASHLEN);
      for (auto const & h:hashes) {
        auto rng = db.equal_range(join_type(type,h));
        auto i = rng.first;
        while (i!=rng.second){
          if (i->second==id) i = db.erase(i);
          else i++;
        }
      }
    }

    // get all types
    std::set<uint32_t> get_types() override {
      std::set<uint32_t> ret;
      uint32_t type = 0;
      while (1) {
        auto i = db.lower_bound(join_type(type, std::string()));
        if (i==db.end()) break;
        if (i->first.size()<4) throw Err() << "broken database, key size<4";
        type = *(uint32_t*)(i->first.data());
        ret.insert(type);
        type++;
      }
      return ret;
    }

    // get range of the largest geohash
    dRect bbox() override {
      dRect ret;
      auto hash0 = join_type(0, std::string());
      while (1) {
        auto i = db.lower_bound(hash0);
        if (i==db.end()) break;
        if (i->first.size()<4) throw Err() << "broken database, key size<4";
        hash0 = i->first;
        ret.expand(GEOHASH_decode(hash0.substr(4)));
        hash0+=('z'+1); // skip longer hashes
      }
      return ret;
    }

    // dump database
    void dump() override {
      for (const auto & i:db){
        if (i.first.size()<4) throw Err() << "broken database, key size<4";
        auto hash     = i.first.substr(4);
        uint32_t type = *(uint32_t*)i.first.data();
        uint32_t id   = i.second;
        std::cout << id << "\t" << type << "\t" << hash << "\n";
      }
    }

    // get objects for geohash
    std::set<uint32_t> get_hash(const std::string & hash0, bool exact) override{
      std::set<uint32_t> ret;
      for (auto i = db.lower_bound(hash0); i!=db.end(); i++){
        if (i->first.size()<4) throw Err() << "broken database, key size<4";
        if (exact && i->first != hash0) break;
        if (i->first.size() < hash0.size() ||
            i->first.compare(0, hash0.size(), hash0)!=0) break;
        ret.insert(i->second);
      }
      return ret;
    }
};


/**********************************************************/
// Implementation class (DB storage)
class GeoHashDB::ImplDB : public GeoHashDB::Impl {
  public:
    std::shared_ptr<void> db;


    ImplDB(std::string fname, const char *dbname, bool create) {
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
    ~ImplDB() {}


    // add an object
    void put(const uint32_t id, const uint32_t type, const dRect & range) override {
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
    void del(const uint32_t id, const uint32_t type, const dRect & range) override{
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
    std::set<uint32_t> get_types() override {
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
    dRect bbox() override {
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
    void dump() override {
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
    std::set<uint32_t> get_hash(const std::string & hash0, bool exact) override {
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
};


/**********************************************************/
// Main class methods
GeoHashDB::GeoHashDB(std::string fname, const char *dbname, bool create) {
  if (fname != "") impl.reset(new ImplDB(fname, dbname, create));
  else  impl.reset(new ImplMem());
}

std::set<uint32_t>
GeoHashDB::get(const uint32_t type, const dRect & range){
  return impl->get(type, range);}

void
GeoHashDB::put(const uint32_t id, const uint32_t type, const dRect & range){
  impl->put(id, type, range);}

void
GeoHashDB::del(const uint32_t id, const uint32_t type, const dRect & range){
  impl->del(id, type, range);}

std::set<uint32_t>
GeoHashDB::get_types() {return impl->get_types();}

dRect
GeoHashDB::bbox() {return impl->bbox();}

void
GeoHashDB::dump() {return impl->dump();}

GeoHashDB::~GeoHashDB(){}

