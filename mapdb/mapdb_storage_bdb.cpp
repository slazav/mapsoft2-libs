#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "mapdb_storage_bdb.h"
#include "string_pack.h"

using namespace std;


/**********************************************************/
MapDBStorageBDB::MapDBStorageBDB(std::string name, bool create):
    objects(name,    NULL, create, false),
    geohash(name + "_gh", NULL, create){
};

/**********************************************************/
void
MapDBStorageBDB::delete_db(std::string name){
  remove(name.c_str());
  remove((name + "_gh").c_str());
}

/**********************************************************/

uint32_t
MapDBStorageBDB::add(const MapDBObj & o){
  if (o.empty())
    throw Err() << "MapDBStorageBDB::add: empty object";

  // get last id + 1
  uint32_t id;
  objects.get_last(id);
  if (id == 0xFFFFFFFF) id=0;
  else id++;

  if (id == 0xFFFFFFFF)
    throw Err() << "MapDBStorageBDB::add: object ID overfull";

  // write object
  objects.put(id, o.pack());

  // write geohash
  geohash.put(id, o.bbox(), o.type);

  return id;
}

void
MapDBStorageBDB::put(const uint32_t id, const MapDBObj & o){

  // get old object
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "MapDBStorageBDB::put: object does not exists: " << id;

  if (o.empty())
    throw Err() << "MapDBStorageBDB::put: empty object";

  MapDBObj o1;
  o1.unpack(str);

  // Delete geohashes
  geohash.del(id, o1.bbox(), o1.type);

  // write new object
  objects.put(id, o.pack());

  // write geohash
  geohash.put(id, o.bbox(), o.type);

}

MapDBObj
MapDBStorageBDB::get(const uint32_t id){
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "MapDBStorageBDB::get: object does not exists: " << id;
  MapDBObj ret;
  ret.unpack(str);
  return ret;
}


void
MapDBStorageBDB::del(const uint32_t id){

  // get old object
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "MapDBStorageBDB::del: object does not exists: " << id;

  MapDBObj o;
  o.unpack(str);

  // Delete geohashes
  geohash.del(id, o.bbox(), o.type);

  // Delete the object
  objects.del(id);
}

