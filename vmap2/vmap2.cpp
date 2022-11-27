#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <string>
#include <cstdio>
#include <algorithm>
#include <unistd.h>

#include "vmap2.h"
#include "string_pack.h"
#include "filename/filename.h"

using namespace std;


/**********************************************************/
VMap2::VMap2(const std::string & name, const bool create) {

  bdb = (name!="");
  if (bdb){
    dbname = file_ext_repl(name, VMAP2DB_EXT);
    ghname = file_ext_repl(dbname, VMAP2GH_EXT);
    bool nogh = file_exists(dbname) && !file_exists(ghname);
    objects_bdb.reset(new DBSimple(dbname, NULL, create, false));
    if (nogh)
      geohash_rebuild();
    else
      geohash.reset(new GeoHashDB(ghname, NULL, create));
  }
  else {
    geohash.reset(new GeoHashStorage);
  }

};

void
VMap2::remove_db(const std::string & name){
  auto dbname = file_ext_repl(name,   VMAP2DB_EXT);
  auto ghname = file_ext_repl(dbname, VMAP2GH_EXT);
  if (file_exists(dbname)) ::unlink(dbname.c_str());
  if (file_exists(ghname)) ::unlink(ghname.c_str());
}

void
VMap2::geohash_rebuild(){
  if (bdb){
    if (file_exists(ghname)) ::unlink(ghname.c_str());
    geohash.reset(new GeoHashDB(ghname, NULL, true));
    for (const auto & p:*objects_bdb){
      auto o = VMap2obj::unpack(p.second);
      geohash->put(p.first, o.bbox(), o.type);
    }
  }
  else {
    geohash.reset(new GeoHashStorage);
    for (const auto & p:objects_mem){
      geohash->put(p.first, p.second.bbox(), p.second.type);
    }
  }
}

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

size_t
VMap2::size() const{
  if (bdb) return objects_bdb->size();
  else     return objects_mem.size();
}

/**********************************************************/

void
VMap2::iter_start(){
  if (bdb)
    it_bdb=objects_bdb->begin();
  else
    it_mem=objects_mem.begin();
}

std::pair<uint32_t, VMap2obj>
VMap2::iter_get_next(){
  if (bdb){
    auto p = *it_bdb;
    ++it_bdb;
    return std::make_pair(p.first, VMap2obj::unpack(p.second));
  }
  else
    return *it_mem++;
}

bool
VMap2::iter_end(){
  if (bdb)
    return it_bdb==objects_bdb->end();
  else
    return it_mem==objects_mem.end();
}

/**********************************************************/

void
VMap2::read(std::istream & s){
  s >> std::ws; // reach eof or next object
  while (!s.eof()) add(VMap2obj::read(s));
}

void
VMap2::read(const std::string & file){
  std::ifstream s(file);
  if (!s) throw Err() << "can't open file: " << file;
  read(s);
}

void
VMap2::write(std::ostream & s){
  // copy objects to vector
  std::vector<VMap2obj> vec;
  iter_start();
  while (!iter_end())
    vec.push_back(iter_get_next().second);

  std::sort(vec.begin(), vec.end());

  for (auto const & o:vec) s << o;
}

void
VMap2::write(const std::string & file){
  std::ofstream s(file);
  if (!s) throw Err() << "can't open file: " << file;
  VMap2::write(s);
}

/**********************************************************/

#include "geo_data/geo_utils.h"
uint32_t
VMap2::find_ref(const dPoint & pt, const uint32_t type, const double & dist1, const double & dist2){
  // First pass, try to find exact coordinate
  // with same or different name.
  dRect r(pt, pt);
  double m = INFINITY;
  uint32_t mi;
  for (auto const & i:find(type, r)){
    auto o1 = get(i);
    auto d1 = geo_nearest_dist(o1, pt);
    if (d1<m) {m=d1, mi=i;}
  }
  // Note that here we can have objects which are really far away.
  // We need to find minimum and check that distance is less then dist1
  if (m<dist1) return mi;

  // Pass 2 with bigger distance. It's slower, but here we have only
  // objects which were moved. Should we check that name is same?
  r.expand(dist2);
  m = INFINITY;
  for (auto const & i:find(type, r)){
    auto o1 = get(i);
    auto d1 = geo_nearest_dist(o1, pt);
    if (d1<m) {m=d1, mi=i;}
  }
  if (m<dist2) return mi;
  return 0xFFFFFFFF;
}

std::multimap<uint32_t, uint32_t>
VMap2::find_refs(const double & dist1, const double & dist2){
  std::multimap<uint32_t, uint32_t> tab;

  // Iterate through all text objects.
  // It's better to use geohash, because there we have
  // objects sorted by type.
  for (auto const t: geohash->get_types()){
    if (t>>24 != VMAP2_TEXT) continue;
    for (auto const i: geohash->get(t)){
      auto l = get(i);
      auto j = find_ref(l.ref_pt, l.ref_type, dist1, dist2);
      tab.emplace(j,i);
    }
  }
  return tab;
}

std::set<uint32_t>
VMap2::find_class(const VMap2objClass cl){
  std::set<uint32_t> ret;
  for (auto const t: geohash->get_types()){
    if (t>>24 != cl) continue;
    auto p = geohash->get(t);
    ret.insert(p.begin(), p.end());
  }
  return ret;
}
