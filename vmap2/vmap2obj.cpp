#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>

#include "vmap2obj.h"
#include "string_pack.h"

using namespace std;

/**********************************************************/
// simple helpers for reading/writing options:

std::string opt_to_str(const std::pair<std::string, std::string> & o){
  if (o.second.size()) return o.first + ": " + o.second;
  else return o.first;
}

std::pair<std::string, std::string> str_to_opt(const std::string & s){
  auto n = s.find(": ");
  if (n!=s.npos)
    return std::make_pair(s.substr(0,n), s.substr(n+2));
  else
    return std::make_pair(s, "");
}

// reading tags from old files ("tags" is a space-separated stirng)
void
VMap2obj::add_tags(const std::string & s){
  std::istringstream ss(s);
  std::string tag;
  while (ss) {
    ss >> tag;
    if (tag!="") opts.emplace(tag, "");
  }
}

/**********************************************************/
// pack object to a string (for DB storage)
string
VMap2obj::pack(const VMap2obj & obj) {
  ostringstream s;

  // type is as a single 32-bit integer:
  s.write((char *)&obj.type, sizeof(uint32_t));

  // optional values
  if (!isnan(obj.angle)) string_pack<float>(s, "angl", obj.angle);
  if (obj.scale != 1.0)  string_pack<float>(s, "scle", obj.scale);
  if (obj.align != VMAP2_ALIGN_SW)
    string_pack<int8_t>(s, "algn", (int8_t)(obj.align));

  // optional text fields (4-byte tag, 4-byte length, data);
  if (obj.name!="") string_pack_str(s, "name", obj.name);
  if (obj.comm!="") string_pack_str(s, "comm", obj.comm);

  // opts
  for (auto const & o: obj.opts)
    string_pack_str(s, "opts", opt_to_str(o));

  // reference type
  if (obj.ref_type!=0xFFFFFFFF)
    string_pack<uint32_t>(s, "reft", obj.ref_type);

  // reference point
  if (obj.ref_pt!=dPoint())
    string_pack_pt(s, "refp", obj.ref_pt);

  // coordinates
  string_pack_crds(s, "crds", obj);

  return s.str();
}

/**********************************************************/
// write object in a text form
void
VMap2obj::write(std::ostream & s, const VMap2obj & obj) {

  // type in the form "line:0x15"
  s << print_type(obj.type) << '\n';

  // optional values
  s.unsetf(ios_base::floatfield);
  if (!isnan(obj.angle)) string_write(s, "angl", obj.angle);
  if (obj.scale != 1.0)  string_write(s, "scle", obj.scale);
  if (obj.align != VMAP2_ALIGN_SW)
     string_write_str(s, "algn", print_align(obj.align));

  // optional text fields (4-byte tag, 4-byte length, data);
  if (obj.name!="") string_write_str(s, "name", obj.name);
  if (obj.comm!="") string_write_str(s, "comm", obj.comm);

  // opts
  for (const auto & o:obj.opts)
    string_write_str(s, "opts", opt_to_str(o));

  // reference type
  if (obj.ref_type!=0xFFFFFFFF)
    string_write_str(s, "reft", print_type(obj.ref_type));

  // reference point
  if (obj.ref_pt!=dPoint())
    string_write_pt(s, "refp", obj.ref_pt);

  // coordinates
  string_write_crds(s, "crds", obj);
  s << "\n";
}


// unpack object from a string (for DB storage)
VMap2obj
VMap2obj::unpack(const std::string & str) {

  VMap2obj ret;

  istringstream s(str);

  // type
  s.read((char*)&ret.type, sizeof(int32_t));

  // other fields
  while (1){
    string tag = string_unpack_tag(s);
    if (tag == "") break;
    else if (tag == "angl") ret.angle = string_unpack<float>(s);
    else if (tag == "scle") ret.scale = string_unpack<float>(s);
    else if (tag == "algn") ret.align = (VMap2objAlign)string_unpack<int8_t>(s);
    else if (tag == "name") ret.name  = string_unpack_str(s);
    else if (tag == "comm") ret.comm  = string_unpack_str(s);
    else if (tag == "tags") ret.opts.emplace(string_unpack_str(s), ""); // old files
    else if (tag == "opts") ret.opts.insert(str_to_opt(string_unpack_str(s)));
    else if (tag == "reft") ret.ref_type = string_unpack<uint32_t>(s);
    else if (tag == "refp") ret.ref_pt   = string_unpack_pt(s);
    else if (tag == "crds") ret.push_back(string_unpack_crds(s));
    else throw Err() << "Unknown tag: " << tag;
  }
  return ret;
}

// Read object from a stream (for reading text files)
// The stream should be set to a correct place (object type string)
VMap2obj
VMap2obj::read(std::istream & s) {

  VMap2obj ret;

  // read type
  std::string str;
  std::getline(s, str, '\n');
  ret.type = make_type(str);
  // other fields
  while (1){
    string tag = string_read_tag(s);
    if (tag == "") break;
    // note: no chld tag!
    else if (tag == "angl") ret.angle = string_read<float>(s);
    else if (tag == "scle") ret.scale = string_read<float>(s);
    else if (tag == "algn") ret.align = parse_align(string_read_str(s));
    else if (tag == "name") ret.name  = string_read_str(s);
    else if (tag == "comm") ret.comm  = string_read_str(s);
    else if (tag == "tags") ret.add_tags(string_read_str(s)); // old files
    else if (tag == "opts") ret.opts.insert(str_to_opt(string_read_str(s)));
    else if (tag == "reft") ret.ref_type = make_type(string_read_str(s));
    else if (tag == "refp") ret.ref_pt   = string_read_pt(s);
    else if (tag == "crds") ret.push_back(string_read_crds(s));
    else throw Err() << "Unknown tag: " << tag;
  }
  s >> std::ws; // reach eof or next object
  return ret;
}


/**********************************************************/

uint32_t
VMap2obj::make_type(const uint16_t cl, const uint32_t tnum){
  switch (cl){
    case VMAP2_POINT:   return  tnum;
    case VMAP2_LINE:    return (1<<24) | (tnum & 0xFFFFFF);
    case VMAP2_POLYGON: return (2<<24) | (tnum & 0xFFFFFF);
    case VMAP2_TEXT:    return (3<<24) | (tnum & 0xFFFFFF);
    case VMAP2_NONE:    return 0xFFFFFFFF;
    default: throw Err() << "unknown object class: " << cl;
  }
}

uint32_t
VMap2obj::make_type(const std::string & s){
  try{
    if (s == "") throw Err() << "empty string";
    if (s == "none") return 0xFFFFFFFF;
    size_t n = s.find(':');
    if (n==std::string::npos) throw Err() << "':' separator not found";
    int tnum = str_to_type<int>(s.substr(n+1));
    if (tnum>0xFFFFFF) throw Err() << "too large number";
    if (s.substr(0,n) == "point") return make_type(VMAP2_POINT,   tnum);
    if (s.substr(0,n) == "line")  return make_type(VMAP2_LINE,    tnum);
    if (s.substr(0,n) == "area")  return make_type(VMAP2_POLYGON, tnum);
    if (s.substr(0,n) == "text")  return make_type(VMAP2_TEXT,    tnum);
    throw Err() << "point, line, area, or text word expected";
  }
  catch (Err & e) {
    throw Err() << "can't parse VMAP2 object type"
                << (s!=""? string(" \"") + s + "\"": "")
                << ": " << e.str();
  }
}

std::string
VMap2obj::print_type(const uint32_t t){
  std::ostringstream s;
  switch (t>>24){
    case 0: s << "point:"; break;
    case 1: s << "line:";  break;
    case 2: s << "area:";  break;
    case 3: s << "text:";  break;
    case 0xFF: return "none";
    default: s << "unknown:";
  }
  s << "0x" << std::hex << (t&0xFFFFFF);
  return s.str();
}


VMap2objClass
VMap2obj::get_class(const uint32_t type) {
  switch (type>>24){
    case 0: return VMAP2_POINT;
    case 1: return VMAP2_LINE;
    case 2: return VMAP2_POLYGON;
    case 3: return VMAP2_TEXT;
    case 0xFF: return VMAP2_NONE;
    default: throw Err() << "unknown object class: " << (type>>24);
  }
}

/**********************************************************/

static const std::map<VMap2objAlign, std::string> align_tab_i2s = {
  {VMAP2_ALIGN_SW,"SW"},
  {VMAP2_ALIGN_W,  "W"},
  {VMAP2_ALIGN_NW,"NW"},
  {VMAP2_ALIGN_N,  "N"},
  {VMAP2_ALIGN_NE,"NE"},
  {VMAP2_ALIGN_E,  "E"},
  {VMAP2_ALIGN_SE,"SE"},
  {VMAP2_ALIGN_S,  "S"},
  {VMAP2_ALIGN_C,  "C"}
};

std::string
VMap2obj::print_align(const VMap2objAlign align){
  if (align_tab_i2s.count(align)<1) throw Err() << "unknown object align: " << align;
  return align_tab_i2s.find(align)->second;
}

static const std::map<std::string,VMap2objAlign> align_tab_s2i = {
  {"SW",VMAP2_ALIGN_SW},
  { "W",VMAP2_ALIGN_W},
  {"NW",VMAP2_ALIGN_NW},
  { "N",VMAP2_ALIGN_N},
  {"NE",VMAP2_ALIGN_NE},
  { "E",VMAP2_ALIGN_E},
  {"SE",VMAP2_ALIGN_SE},
  { "S",VMAP2_ALIGN_S},
  { "C",VMAP2_ALIGN_C}
};

VMap2objAlign
VMap2obj::parse_align(const std::string & str){
  if (align_tab_s2i.count(str)<1) throw Err() << "unknown object align: " << str;
  return align_tab_s2i.find(str)->second;
}



/**********************************************************/


void
VMap2obj::set_coords(const std::string & s){
  if (get_class() == VMAP2_POINT || get_class() == VMAP2_TEXT){
    dLine l;
    l.push_back(dPoint(s));
    clear();
    push_back(l);
  }
  else {
    dMultiLine::operator=(dMultiLine(s));
  }
}

/**********************************************************/

std::ostream &
operator<< (std::ostream & s, const VMap2obj & o){
  VMap2obj::write(s,o); return s; }

std::istream &
operator>> (std::istream & s, VMap2obj & o){
  o = VMap2obj::read(s); return s; }
