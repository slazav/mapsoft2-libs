#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "mapdb_storage_mem.h"

using namespace std;


uint32_t
MapDBStorageMem::add(const MapDBObj & o){
  if (o.empty()) throw Err() << "MapDBStorageMem::add: empty object";

  // get last id + 1
  uint32_t id = 0;
  if (objects.size()) id = objects.rbegin()->first + 1;

  if (id == 0xFFFFFFFF)
    throw Err() << "MapDBStorageMem::add: object ID overfull";

  // write object
  objects.emplace(id, o);

  // write geohash
  geohash.put(id, o.bbox(), o.type);

  return id;
}

void
MapDBStorageMem::put(const uint32_t id, const MapDBObj & o){

  // get old object
  auto i = objects.find(id);
  if (i == objects.end())
    throw Err() << "MapDBStorageMem::put: object does not exists: " << id;

  if (i->second.empty())
    throw Err() << "MapDBStorageMem::put: empty object";

  // Delete geohashes
  geohash.del(id, i->second.bbox(), i->second.type);

  // write new object
  objects.emplace(id, o);

  // write geohash
  geohash.put(id, o.bbox(), o.type);
}

MapDBObj
MapDBStorageMem::get(const uint32_t id){
  auto i = objects.find(id);
  if (i == objects.end())
    throw Err() << "MapDBStorageMem::put: object does not exists: " << id;
  return i->second;
}


void
MapDBStorageMem::del(const uint32_t id){
  auto i = objects.find(id);
  if (i == objects.end())
    throw Err() << "MapDBStorageMem::put: object does not exists: " << id;

  // Delete geohashes
  geohash.del(id, i->second.bbox(), i->second.type);

  // Delete the object
  objects.erase(i);
}

