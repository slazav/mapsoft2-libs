#include <cstring>
#include <string>
#include <db.h>

#include "err/err.h"
#include "db_simple.h"
#include "db_tools.h"

/**********************************************************/
// pack/unpack uint32_t key

std::string
pack_uint32(const uint32_t v){
  std::string ret(4,0);
  ret[0] = (char)((v>>24)&0xff);
  ret[1] = (char)((v>>16)&0xff);
  ret[2] = (char)((v>>8)&0xff);
  ret[3] = (char)(v&0xff);
  return ret;
}

uint32_t
unpack_uint32(DBT *k){
  if (k->size != sizeof(uint32_t))
    throw Err() << "db_simple: broken database, wrong key size: " << k->size;
  unsigned char *d = (unsigned char *)k->data;
  uint32_t ret = ((uint32_t)d[0]<<24) + ((uint32_t)d[1]<<16)
               + ((uint32_t)d[2]<<8) + (uint32_t)d[3];
  return ret;
}

/**********************************************************/

DBSimple::~DBSimple(){}

DBSimple::DBSimple(std::string fname, const char *dbname, bool create, bool dup){
  // set flags
  int open_flags = create? DB_CREATE|DB_EXCL:0;

  /* Initialize the DB handle */
  DB *dbp;
  int ret = db_create(&dbp, NULL, 0);
  if (ret != 0) throw Err() << "db_simple: " << db_strerror(ret);
  db = std::shared_ptr<void>(dbp, bdb_close);

  // allow duplicates if needed
  if (dup){
    ret = dbp->set_flags(dbp, DB_DUP);
    if (ret != 0) throw Err() << "db_simple: " << db_strerror(ret);
  }

  /* Open the database */
  ret = dbp->open(dbp,    /* Pointer to the database */
                  NULL,          /* Txn pointer */
                  fname.c_str(), /* file */
                  dbname,        /* database (can be NULL) */
                  DB_BTREE,      /* Database type (using btree) */
                  open_flags,    /* Open flags */
                  0644);         /* File mode*/
  if (ret != 0)
    throw Err() << "db_simple: " << fname << ": " << db_strerror(ret);

  /* open cursor */
  DBC *curp=NULL;
  dbp->cursor(dbp, NULL, &curp, 0);
  if (curp==NULL)
    throw Err() << "db_simple: can't get a cursor";
  cur = std::shared_ptr<void>(curp, bdb_cur_close);
}


bool
DBSimple::exists(const uint32_t key){
  DBC *dbc = (DBC*)cur.get();
  std::string key_s = pack_uint32(key);
  DBT k = mk_dbt(key_s);
  DBT v = mk_dbt();
  int ret = dbc->c_get(dbc, &k, &v, DB_SET);

  if (ret == DB_NOTFOUND) return false;
  if (ret != 0) throw Err() << "db_simple: " << db_strerror(ret);
  return true;
}

void
DBSimple::put(const uint32_t key, const std::string & val){
  DB  *dbp = (DB*)db.get();
  std::string key_s = pack_uint32(key);
  DBT k = mk_dbt(key_s);
  DBT v = mk_dbt(val);
  int ret = dbp->put(dbp, NULL, &k, &v, 0);
  if (ret != 0) throw Err() << "db_simple: " << db_strerror(ret);
}

// Main get function. Uses cursor, supports all flags.
// Set key to 0xFFFFFFFF if nothing is found.
std::string
DBSimple::get(uint32_t & key, int flags){
  DBC *dbc = (DBC*)cur.get();
  std::string key_s = pack_uint32(key);
  DBT k = mk_dbt(key_s);
  DBT v = mk_dbt();
  int ret = dbc->c_get(dbc, &k, &v, flags);

  if (ret == DB_NOTFOUND) {key = 0xFFFFFFFF; return std::string();}
  if (ret != 0) throw Err() << "db_simple: " << db_strerror(ret);
  if (k.size != sizeof(uint32_t))
    throw Err() << "db_simple: broken database, wrong key size: " << k.size;
  key = unpack_uint32(&k);
  return std::string((char*)v.data, (char*)v.data+v.size);
}

std::string
DBSimple::get(uint32_t & key){ return get(key, DB_SET);}

std::string
DBSimple::get_range(uint32_t & key){ return get(key, DB_SET_RANGE);}

std::string
DBSimple::get_next(uint32_t & key){ return get(key, DB_NEXT);}

std::string
DBSimple::get_prev(uint32_t & key){ return get(key, DB_PREV);}

std::string
DBSimple::get_last(uint32_t & key){ return get(key, DB_LAST);}

std::string
DBSimple::get_first(uint32_t & key){ return get(key, DB_FIRST);}


// Delete function.
uint32_t
DBSimple::del(const uint32_t key){
  DBC *dbc = (DBC*)cur.get();
  std::string key_s = pack_uint32(key);
  DBT k = mk_dbt(key_s);
  DBT v = mk_dbt();
  uint32_t num = 0;
  int flags = DB_SET_RANGE;
  while (1) {
    int ret = dbc->c_get(dbc, &k, &v, flags);
    if (ret == DB_NOTFOUND) {return num;}
    if (ret != 0) throw Err() << "db_simple: " << db_strerror(ret);
    if (k.size != sizeof(uint32_t))
      throw Err() << "db_simple: broken database, wrong key size: " << k.size;
    if (key != unpack_uint32(&k)) break;
    flags = DB_NEXT;
    dbc->c_del(dbc, 0);
    num++;
  }
  return num;
}

