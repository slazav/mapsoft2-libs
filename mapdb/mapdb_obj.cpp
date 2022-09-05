#include <fstream>
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
MapDBObj::pack() const {
  ostringstream s;

  // two integer numbers: flags, type are packed in a single 32-bit integer:
  s.write((char *)&type, sizeof(uint32_t));

  // optional values
  if (!isnan(angle)) string_pack<float>(s, "angl", angle);
  if (scale != 1.0)  string_pack<float>(s, "scle", scale);
  if (align != MAPDB_ALIGN_SW)
    string_pack<int8_t>(s, "algn", (int8_t)(align));

  // optional text fields (4-byte tag, 4-byte length, data);
  if (name!="") string_pack_str(s, "name", name);
  if (comm!="") string_pack_str(s, "comm", comm);

  // tags
  for (auto const & t: tags)
    string_pack_str(s, "tags", t);

  // children
  for (auto const & c: children)
    string_pack<uint32_t>(s, "chld", c);

  // coordinates
  string_pack_crds(s, "crds", *this);

  return s.str();
}

// unpack object from a string (for DB storage)
void
MapDBObj::unpack(const std::string & str) {

  // re-initialize
  *this = MapDBObj();

  istringstream s(str);

  // type
  s.read((char*)&type, sizeof(int32_t));

  // other fields
  while (1){
    string tag = string_unpack_tag(s);
    if (tag == "") break;
    else if (tag == "angl") angle = string_unpack<float>(s);
    else if (tag == "scle") scale = string_unpack<float>(s);
    else if (tag == "algn") align = (MapDBObjAlign)string_unpack<int8_t>(s);
    else if (tag == "name") name  = string_unpack_str(s);
    else if (tag == "comm") comm  = string_unpack_str(s);
    else if (tag == "tags") tags.insert(string_unpack_str(s));
    else if (tag == "chld") children.insert(string_unpack<uint32_t>(s));
    else if (tag == "crds") push_back(string_unpack_crds(s));
    else throw Err() << "Unknown tag: " << tag;
  }
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

