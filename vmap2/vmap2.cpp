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

std::multimap<uint32_t, uint32_t>
VMap2::find_refs(const double & dist1, const double & dist2){
  std::multimap<uint32_t, uint32_t> tab;

  // Find all labels (text objects).
  // It's better to use geohash, because there we have
  // objects sorted by type.

  // We work with every type independently:
  for (const auto & t:  geohash->get_types()){
    if (t>>24 != VMAP2_TEXT) continue;

    // all labels
    auto labels = geohash->get(t);

    // Pass 1. For each label try to find object with
    // same name within distance dist1.
    // Pass 2. Repeat same with distance dist2.
    for (auto pass=0; pass<2; pass++){
      auto & dist = pass==0? dist1:dist2;
      auto it = labels.begin();
      while (it != labels.end()){
        auto l = get(*it);

        dRect r(l.ref_pt, l.ref_pt);
        r.expand(dist);
        double md = INFINITY;
        uint32_t mi = 0xFFFFFFFF;
        for (auto const & i:find(l.ref_type, r)){
          auto o1 = get(i);
          if (o1.name != l.name) continue;
          auto d1 = geo_nearest_vertex(o1, l.ref_pt);
          if (d1<md) {md=d1, mi=i;}
        }

        if (md<dist) {
          tab.emplace(mi,*it);
          it = labels.erase(it);
        }
        else ++it;
      }
    }

    // Pass 3. Find object within dist1, without
    // other connected labels OR with a label of same name.
    // Pass 4. Repeat same with dist2.
    for (auto pass=0; pass<2; pass++){
      auto & dist = pass==0? dist1:dist2;
      auto it = labels.begin();
      while (it != labels.end()){
        auto l = get(*it);

        dRect r(l.ref_pt, l.ref_pt);
        r.expand(dist);
        double md = INFINITY;
        uint32_t mi = 0xFFFFFFFF;
        for (auto const & i:find(l.ref_type, r)){
          if (tab.count(i)>0){
            auto l1 = get(tab.find(i)->second);
            if (l1.name != l.name) continue;
          }
          auto o1 = get(i);
          auto d1 = geo_nearest_vertex(o1, l.ref_pt);
          if (d1<md) {md=d1, mi=i;}
        }
        if (md<dist) {
          tab.emplace(mi,*it);
          it = labels.erase(it);
        }
        else ++it;
      }
    }

    // For unconnected labels put 0x0xFFFFFFFF into the tab.
    for (auto & i:labels){
      tab.emplace(0xFFFFFFFF, i);
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
