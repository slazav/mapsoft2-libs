#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "mapdb.h"
#include "mp/mp.h"
#include "opt/opt.h"
#include "string_pack.h"

using namespace std;


/**********************************************************/
MapDB::MapDB(std::string name, bool create):
    objects(name,    NULL, create, false),
    geohash(name + "_gh", NULL, create){
};

/**********************************************************/
void
MapDB::delete_db(std::string name){
  remove(name.c_str());
  remove((name + "_gh").c_str());
}

/**********************************************************/

uint32_t
MapDB::add(const MapDBObj & o){
  if (o.empty())
    throw Err() << "MapDB::add: empty object";

  // get last id + 1
  uint32_t id;
  objects.get_last(id);
  if (id == 0xFFFFFFFF) id=0;
  else id++;

  if (id == 0xFFFFFFFF)
    throw Err() << "MapDB::add: object ID overfull";

  // write object
  objects.put(id, o.pack());

  // write geohash
  geohash.put(id, o.type, o.bbox());

  return id;
}

void
MapDB::put(const uint32_t id, const MapDBObj & o){

  // get old object
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "MapDB::put: object does not exists: " << id;

  if (o.empty())
    throw Err() << "MapDB::put: empty object";

  MapDBObj o1;
  o1.unpack(str);

  // Delete geohashes
  geohash.del(id, o1.type, o1.bbox());

  // write new object
  objects.put(id, o.pack());

  // write geohash
  geohash.put(id, o.type, o.bbox());

}

MapDBObj
MapDB::get(const uint32_t id){
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "MapDB::get: object does not exists: " << id;
  MapDBObj ret;
  ret.unpack(str);
  return ret;
}


void
MapDB::del(const uint32_t id){

  // get old object
  uint32_t id1 = id;
  std::string str = objects.get(id1);
  if (id1 == 0xFFFFFFFF)
    throw Err() << "MapDB::del: object does not exists: " << id;

  MapDBObj o;
  o.unpack(str);

  // Delete heohashes
  geohash.del(id, o.type, o.bbox());

  // Delete the object
  objects.del(id);
}

