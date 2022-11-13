#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "vmap2.h"
#include "string_pack.h"

using namespace std;


/**********************************************************/
VMap2::VMap2(const std::string & name, const bool create): fname(name) {

  bdb = (fname!="");
  if (bdb){
    objects_bdb.reset(new DBSimple(name, NULL, create, false));
    geohash.reset(new GeoHashDB(name + "_gh", NULL, create));
  }
  else {
    geohash.reset(new GeoHashStorage);
  }

};

/**********************************************************/

uint32_t
VMap2::add(const VMap2obj & o){
  if (o.empty())
    throw Err() << "VMap2::add: empty object";

  // get last id
  uint32_t id;
  if (bdb)
    objects_bdb->get_last(id);
  else
    id = objects_mem.size() ? objects_mem.rbegin()->first : 0xFFFFFFFF;

  if (id == 0xFFFFFFFF) id=0;
  else id++;
  if (id == 0xFFFFFFFF)
    throw Err() << "VMap2::add: object ID overfull";

  // write object
  if (bdb)
    objects_bdb->put(id, VMap2obj::pack(o));
  else
    objects_mem.emplace(id, o);

  geohash->put(id, o.bbox(), o.type);

  return id;
}

void
VMap2::put(const uint32_t id, const VMap2obj & o){

  if (o.empty())
    throw Err() << "VMap2::put: empty object";

  if (bdb){
    // get old object
    uint32_t id1 = id;
    std::string str = objects_bdb->get(id1);
    if (id1 == 0xFFFFFFFF)
      throw Err() << "VMap2::put: object does not exists: " << id;

    VMap2obj o1 = VMap2obj::unpack(str);

    // write new object
    objects_bdb->put(id, VMap2obj::pack(o));
    // rewrite geohash if needed
    if (o1.bbox()!=o.bbox() || o1.type!=o.type) {
      geohash->del(id, o1.bbox(), o1.type);
      geohash->put(id, o.bbox(), o.type);
    }

  }
  else {
    // get old object
    auto i = objects_mem.find(id);
    if (i == objects_mem.end())
      throw Err() << "VMap2::put: object does not exists: " << id;

    // rewrite geohash if needed
    if (i->second.bbox()!=o.bbox() || i->second.type!=o.type) {
      geohash->del(id, i->second.bbox(), i->second.type);
      geohash->put(id, o.bbox(), o.type);
    }

    // write new object
    i->second = o;
  }
}

VMap2obj
VMap2::get(const uint32_t id){
  if (bdb) {
    uint32_t id1(id);
    std::string str = objects_bdb->get(id1);
    if (id1 == 0xFFFFFFFF)
      throw Err() << "VMap2::get: object does not exists: " << id;
    return VMap2obj::unpack(str);
  }
  else {
    auto i = objects_mem.find(id);
    if (i == objects_mem.end())
      throw Err() << "VMap2::get: object does not exists: " << id;
    return i->second;
  }
}


void
VMap2::del(const uint32_t id){
  if (bdb) {
    // get old object
    uint32_t id1 = id;
    std::string str = objects_bdb->get(id1);
    if (id1 == 0xFFFFFFFF)
      throw Err() << "VMap2::del: object does not exists: " << id;

    VMap2obj o = VMap2obj::unpack(str);

    // Delete geohashes
    geohash->del(id, o.bbox(), o.type);
    // Delete the object
    objects_bdb->del(id);
  }
  else {
    auto i = objects_mem.find(id);
    if (i == objects_mem.end())
      throw Err() << "VMap2::del: object does not exists: " << id;

    // Delete geohashes
    geohash->del(id, i->second.bbox(), i->second.type);
    // Delete the object
    objects_mem.erase(i);
  }
}

/**********************************************************/

void
VMap2::iter_start() {
  if (bdb)
    it_bdb=objects_bdb->begin();
  else
    it_mem=objects_mem.begin();
}

std::pair<uint32_t, VMap2obj>
VMap2::iter_get_next() {
  if (bdb){
    auto p = *it_bdb;
    ++it_bdb;
    return std::make_pair(p.first, VMap2obj::unpack(p.second));
  }
  else
    return *it_mem++;
}

bool
VMap2::iter_end() {
  if (bdb)
    return it_bdb==objects_bdb->end();
  else
    return it_mem==objects_mem.end();
}