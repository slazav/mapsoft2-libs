#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "vmap2mem.h"

using namespace std;


uint32_t
VMap2mem::add(const VMap2obj & o){
  if (o.empty()) throw Err() << "VMap2mem::add: empty object";

  // get last id + 1
  uint32_t id = 0;
  if (objects.size()) id = objects.rbegin()->first + 1;

  if (id == 0xFFFFFFFF)
    throw Err() << "VMap2mem::add: object ID overfull";

  // write object
  objects.emplace(id, o);

  // write geohash
  geohash.put(id, o.bbox(), o.type);

  return id;
}

void
VMap2mem::put(const uint32_t id, const VMap2obj & o){

  // get old object
  auto i = objects.find(id);
  if (i == objects.end())
    throw Err() << "VMap2mem::put: object does not exists: " << id;

  if (i->second.empty())
    throw Err() << "VMap2mem::put: empty object";

  // Delete geohashes
  geohash.del(id, i->second.bbox(), i->second.type);

  // write new object
  objects.emplace(id, o);

  // write geohash
  geohash.put(id, o.bbox(), o.type);
}

VMap2obj
VMap2mem::get(const uint32_t id){
  auto i = objects.find(id);
  if (i == objects.end())
    throw Err() << "VMap2mem::put: object does not exists: " << id;
  return i->second;
}


void
VMap2mem::del(const uint32_t id){
  auto i = objects.find(id);
  if (i == objects.end())
    throw Err() << "VMap2mem::put: object does not exists: " << id;

  // Delete geohashes
  geohash.del(id, i->second.bbox(), i->second.type);

  // Delete the object
  objects.erase(i);
}

