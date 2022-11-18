#include <set>
#include <map>
#include <string>

#include "err/err.h"
#include "geohash.h"
#include "storage.h"

// Max hash length. 12 gives 0.1m accuracy
#define HASHLEN 12

// Get id of objects which may be found in the range
std::set<uint32_t>
GeoHashStorage::get(const dRect & range, const uint32_t type) const{
  std::set<uint32_t> ret;
  if (range.is_empty()) return ret;
  std::set<std::string> hashes = GEOHASH_encode4(GEOHASH_encode_box(range, BB), HASHLEN);
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

// Get id of all objects with one type
std::set<uint32_t>
GeoHashStorage::get(const uint32_t type) const{
  return get_hash(join_type(type, ""), false);
}


// add an object
void
GeoHashStorage::put(const uint32_t id, const dRect & range, const uint32_t type) {
  if (range.is_empty()) return;
  auto hashes = GEOHASH_encode4(GEOHASH_encode_box(range, BB), HASHLEN);
  for (auto const & h:hashes) db.emplace(join_type(type,h), id);
}

// delete an object
void
GeoHashStorage::del(const uint32_t id, const dRect & range, const uint32_t type) {
  if (range.is_empty()) return;
  auto hashes = GEOHASH_encode4(GEOHASH_encode_box(range, BB), HASHLEN);
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
std::set<uint32_t>
GeoHashStorage::get_types() const {
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
dRect
GeoHashStorage::bbox() const {
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
  return GEOHASH_decode_box(ret,BB);
}

// dump database
void
GeoHashStorage::dump() const {
  for (const auto & i:db){
    if (i.first.size()<4) throw Err() << "broken database, key size<4";
    auto hash     = i.first.substr(4);
    uint32_t type = *(uint32_t*)i.first.data();
    uint32_t id   = i.second;
    std::cout << id << "\t" << type << "\t"
              << hash << "\t" << GEOHASH_decode(hash) << "\n";
  }
}

// get objects for geohash
std::set<uint32_t>
GeoHashStorage::get_hash(const std::string & hash0, bool exact) const{
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

// we use type+geohash key
std::string
GeoHashStorage::join_type(const uint32_t type, const std::string & hash){
  std::ostringstream ss;
  ss.write((char*)&type, sizeof(type));
  ss.write(hash.data(), hash.size());
  return ss.str();
}
