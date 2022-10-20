#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "mapdb_obj.h"
#include "string_pack.h"

using namespace std;

/**********************************************************/
// pack object to a string (for DB storage)
string
MapDBObj::pack(const MapDBObj & obj) {
  ostringstream s;

  // type is as a single 32-bit integer:
  s.write((char *)&obj.type, sizeof(uint32_t));

  // optional values
  if (!isnan(obj.angle)) string_pack<float>(s, "angl", obj.angle);
  if (obj.scale != 1.0)  string_pack<float>(s, "scle", obj.scale);
  if (obj.align != MAPDB_ALIGN_SW)
    string_pack<int8_t>(s, "algn", (int8_t)(obj.align));

  // optional text fields (4-byte tag, 4-byte length, data);
  if (obj.name!="") string_pack_str(s, "name", obj.name);
  if (obj.comm!="") string_pack_str(s, "comm", obj.comm);

  // tags
  for (auto const & t: obj.tags)
    string_pack_str(s, "tags", t);

  // children
  for (auto const & c: obj.children)
    string_pack<uint32_t>(s, "chld", c);

  // reference point and type
  if (obj.ref_type!=0xFFFFFFFF) {
    string_pack<uint32_t>(s, "reft", obj.ref_type);
    string_pack_pt(s, "refp", obj.ref_pt);
  }

  // coordinates
  string_pack_crds(s, "crds", obj);

  return s.str();
}

// unpack object from a string (for DB storage)
MapDBObj
MapDBObj::unpack(const std::string & str) {

  MapDBObj ret;

  istringstream s(str);

  // type
  s.read((char*)&ret.type, sizeof(int32_t));

  // other fields
  while (1){
    string tag = string_unpack_tag(s);
    if (tag == "") break;
    else if (tag == "angl") ret.angle = string_unpack<float>(s);
    else if (tag == "scle") ret.scale = string_unpack<float>(s);
    else if (tag == "algn") ret.align = (MapDBObjAlign)string_unpack<int8_t>(s);
    else if (tag == "name") ret.name  = string_unpack_str(s);
    else if (tag == "comm") ret.comm  = string_unpack_str(s);
    else if (tag == "tags") ret.tags.insert(string_unpack_str(s));
    else if (tag == "chld") ret.children.insert(string_unpack<uint32_t>(s));
    else if (tag == "reft") ret.ref_type = string_unpack<uint32_t>(s);
    else if (tag == "refp") ret.ref_pt   = string_unpack_pt(s);
    else if (tag == "crds") ret.push_back(string_unpack_crds(s));
    else throw Err() << "Unknown tag: " << tag;
  }
  return ret;
}

/**********************************************************/

uint32_t
MapDBObj::make_type(const uint16_t cl, const uint16_t tnum){
  switch (cl){
    case MAPDB_POINT:   return  tnum;
    case MAPDB_LINE:    return (1<<24) | tnum;
    case MAPDB_POLYGON: return (2<<24) | tnum;
    case MAPDB_TEXT:    return (3<<24) | tnum;
    default: throw Err() << "unknown object class: " << cl;
  }
}

uint32_t
MapDBObj::make_type(const std::string & s){
  try{
    if (s == "") throw Err() << "empty string";
    size_t n = s.find(':');
    if (n==std::string::npos) throw Err() << "':' separator not found";
    int tnum = str_to_type<int>(s.substr(n+1));
    if (tnum>0xFFFF) throw Err() << "too large number";
    if (s.substr(0,n) == "point") return make_type(MAPDB_POINT,   tnum);
    if (s.substr(0,n) == "line")  return make_type(MAPDB_LINE,    tnum);
    if (s.substr(0,n) == "area")  return make_type(MAPDB_POLYGON, tnum);
    if (s.substr(0,n) == "text")  return make_type(MAPDB_TEXT,    tnum);
    throw Err() << "point, line, area, or text word expected";
  }
  catch (Err & e) {
    throw Err() << "can't parse MapDB object type"
                << (s!=""? string(" \"") + s + "\"": "")
                << ": " << e.str();
  }
}

std::string
MapDBObj::print_type(const uint32_t t){
  std::ostringstream s;
  switch (t>>24){
    case 0: s << "point:"; break;
    case 1: s << "line:";  break;
    case 2: s << "area:";  break;
    case 3: s << "text:";  break;
    default: s << "unknown:";
  }
  s << "0x" << std::hex << (t&0xFFFF);
  return s.str();
}


MapDBObjClass
MapDBObj::get_class() const {
  switch (type>>24){
    case 0: return MAPDB_POINT;
    case 1: return MAPDB_LINE;
    case 2: return MAPDB_POLYGON;
    case 3: return MAPDB_TEXT;
    default: throw Err() << "unknown object class: " << (type>>24);
  }
}

uint16_t
MapDBObj::get_tnum()  const {
  return type & 0xFFFF; }

void
MapDBObj::set_coords(const std::string & s){
  if (get_class() == MAPDB_POINT || get_class() == MAPDB_TEXT){
    dLine l;
    l.push_back(dPoint(s));
    clear();
    push_back(l);
  }
  else {
    dMultiLine::operator=(dMultiLine(s));
  }
}

