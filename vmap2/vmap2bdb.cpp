#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "vmap2bdb.h"
#include "string_pack.h"

using namespace std;


/**********************************************************/
VMap2bdb::VMap2bdb(std::string name, bool create):
    VMap2(name),
    objects(name,    NULL, create, false),
    geohash(name + "_gh", NULL, create),
    i(objects.begin()){
};

/**********************************************************/
void
VMap2bdb::delete_db(std::string name){
  remove(name.c_str());
  remove((name + "_gh").c_str());
}

/**********************************************************/

uint32_t
VMap2bdb::add(const VMap2obj & o){
  if (o.empty())
    throw Err() << "VMap2bdb::add: empty object";

  // get last id + 1
  uint32_t id;
  objects.get_last(id);
  if (id == 0xFFFFFFFF) id=0;
  else id++;

  if (id == 0xFFFFFFFF)
    throw Err() << "VMap2bdb::add: object ID overfull";

  // write object
  objects.put(id, VMap2obj::pack(o));

  // write geohash
  geohash.put(id, o.bbox(), o.type);

  return id;
}

void
VMap2bdb::put(const uint32_t id, const VMap2obj & o){

  // get old object
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "VMap2bdb::put: object does not exists: " << id;

  if (o.empty())
    throw Err() << "VMap2bdb::put: empty object";

  VMap2obj o1 = VMap2obj::unpack(str);

  // Delete geohashes
  geohash.del(id, o1.bbox(), o1.type);

  // write new object
  objects.put(id, VMap2obj::pack(o));

  // write geohash
  geohash.put(id, o.bbox(), o.type);

}

VMap2obj
VMap2bdb::get(const uint32_t id){
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "VMap2bdb::get: object does not exists: " << id;
  return VMap2obj::unpack(str);
}


void
VMap2bdb::del(const uint32_t id){

  // get old object
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "VMap2bdb::del: object does not exists: " << id;

  VMap2obj o = VMap2obj::unpack(str);

  // Delete geohashes
  geohash.del(id, o.bbox(), o.type);

  // Delete the object
  objects.del(id);
}

