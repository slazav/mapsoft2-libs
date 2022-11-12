#include <iostream>
#include <stdint.h>
#include <cstring>
#include <string>
#include "err/err.h"
#include "geom/multiline.h"

using namespace std;

void
string_pack_str(ostream & s, const char *tag, const std::string & str){
  if (strlen(tag)!=4) throw Err() << "string_pack_str: 4-byte tag expected";
  s.write(tag, 4);
  uint32_t size = str.size();
  s.write((char *)&size, sizeof(uint32_t));
  s.write(str.data(), size);
  if (s.fail()) throw Err() << "string_pack_str: write error";
}

void
string_write_str(ostream & s, const char *tag, const std::string & str){
  s << tag << ' ';
  for (const auto c: str){
    switch (c){
    case '\\': s << "\\\\";
    case '\n': s << "\\n";
    case '\0': s << "\\0";
    default: s << c;
    }
  }
  s << '\n';
  if (s.fail()) throw Err() << "string_write_str: write error";
}

Point<int32_t>
convert_crd(dPoint p) {
  while (p.x >  180) p.x-=360;
  while (p.x < -180) p.x+=360;
  while (p.y >   90) p.y-=180;
  while (p.y <  -90) p.y+=180;
  return Point<uint32_t>((int32_t)rint(p.x * 1e7), (int32_t)rint(p.y * 1e7));
}

void
string_pack_crds(ostream & s, const char *tag, const dMultiLine & ml){
  if (strlen(tag)!=4) throw Err() << "string_pack_crds: 4-byte tag expected";
  for (auto const &l:ml) {
    s.write(tag, 4);
    uint32_t size = l.size()*2*sizeof(int32_t); // 2 ints per point
      s.write((char *)&size, sizeof(uint32_t));
    for (auto const & p:l) {
      auto ip = convert_crd(p);
      s.write((char *)&ip, 2*sizeof(int32_t));
    }
  }
  if (s.fail()) throw Err() << "string_pack_crds: write error";
}

void
string_write_crds(ostream & s, const char *tag, const dMultiLine & ml){
  for (auto const &l:ml) {
    s << tag;
    for (auto const & p:l) {
      auto ip = convert_crd(p);
      s << ' ' << ip.x << ' ' << ip.y;
    }
    s << '\n';
  }
  if (s.fail()) throw Err() << "string_write_crds: write error";
}

void
string_pack_pt(ostream & s, const char *tag, const dPoint & pt){
  if (strlen(tag)!=4) throw Err() << "string_pack_pt: 4-byte tag expected";
  s.write(tag, 4);
  uint32_t size = 2*sizeof(int32_t); // 2 ints per point
  s.write((char *)&size, sizeof(uint32_t));
  auto ip = convert_crd(pt);
  s.write((char *)&ip, 2*sizeof(int32_t));
  if (s.fail()) throw Err() << "string_pack_pt: write error";
}

void
string_write_pt(ostream & s, const char *tag, const dPoint & pt){
  auto ip = convert_crd(pt);
  s << tag << ' ' << ' ' << ip.x << ' ' << ip.y << '\n';
  if (s.fail()) throw Err() << "string_write_pt: write error";
}

void
string_pack_bbox(ostream & s, const char *tag, const dRect & box) {
  if (strlen(tag)!=4) throw Err() << "string_pack_bbox: 4-byte tag expected";
  s.write(tag, 4);
  uint32_t size = 4*sizeof(int32_t);
  s.write((char*)&size, sizeof(uint32_t));
  auto ip1 = convert_crd(box.tlc());
  auto ip2 = convert_crd(box.brc());
  s.write((char *)&ip1, 2*sizeof(int32_t));
  s.write((char *)&ip2, 2*sizeof(int32_t));
  if (s.fail()) throw Err() << "string_pack_bbox: write error";
}

void
string_write_bbox(ostream & s, const char *tag, const dRect & box) {
  auto ip1 = convert_crd(box.tlc());
  auto ip2 = convert_crd(box.brc());
  s << tag << ' ' << ip1.x << ' ' << ip1.y << ' ' << ip2.x << ' ' << ip2.y << '\n';
  if (s.fail()) throw Err() << "string_write_bbox: write error";
}



std::string
string_unpack_tag(istream & s){
  std::string tag(4,'\0');
  s.read((char*)tag.data(), 4);
  if (s.gcount()==0) return string();
  if (s.fail()) throw Err() << "string_unpack_tag: read error";
  return tag;
}

std::string
string_read_tag(istream & s){
  std::string tag;
  s >> tag;
  if (s.get()=='\n') s.unget(); // skip space
  return tag;
}

std::string
string_unpack_str(istream & s){
  uint32_t size;
  s.read((char*)&size, sizeof(uint32_t));
  std::string str(size, '\0');
  s.read((char*)str.data(), size);
  if (s.fail()) throw Err() << "string_unpack_str: read error";
  return str;
}

std::string
string_read_str(istream & s){
  std::string ret;
  while (1){
    char c;
    switch (c = s.get()){
      case -1:
      case '\n': return ret;
      case '\\':
        switch (c = s.get()){
          case 'n': ret.push_back('\n'); break;
          case '0': ret.push_back('\0'); break;
          default:  ret.push_back(c); break;
        }
        break;
      default:
        ret.push_back(c);
    }
  }
  return ret;
}


dLine
string_unpack_crds(istream & s){
  uint32_t size;
  s.read((char*)&size, sizeof(uint32_t));
  dLine ret;
  for (size_t i=0; i<size/2/sizeof(int32_t); i++) {
    int32_t crd[2];
    s.read((char*)crd, 2*sizeof(int32_t));
    dPoint p(crd[0]/1e7, crd[1]/1e7);
    ret.push_back(p);
  }
  if (s.fail()) throw Err() << "string_unpack_crds: read error";
  return ret;
}

// read two ints, convert to dPoint
dPoint
read_pt(istream & s){
  int x,y;
  s >> x >> y;
  return dPoint(x/1e7, y/1e7);
}

dLine
string_read_crds(istream & s){
  dLine ret;
  while (1) {
    if (s.get()=='\n') return ret;
    else s.unget();
    dPoint p = read_pt(s);
    if (s.fail()) throw Err() << "string_read_crds: read error or unexpected eof";
    ret.push_back(p);
  }
  return ret;
}

dPoint
string_unpack_pt(istream & s){
  uint32_t size;
  s.read((char*)&size, sizeof(uint32_t));
  if (size!=2*sizeof(int32_t))
    throw Err() << "string_unpack_pt: wrong point size: " << size;
  int32_t crd[2];
  s.read((char*)crd, 2*sizeof(int32_t));
  if (s.fail()) throw Err() << "string_unpack_pt: read error";
  return dPoint(crd[0]/1e7, crd[1]/1e7);
}

dPoint
string_read_pt(istream & s){
  dPoint p = read_pt(s);
  if (s.get()!='\n') throw Err() << "string_read_pt: eol expected";
  if (s.fail()) throw Err() << "string_read_pt: read error or unexpected eof";
  return p;
}

dRect
string_unpack_bbox(istream & s) {
  uint32_t size, size0=4*sizeof(int32_t);
  s.read((char*)&size, sizeof(uint32_t));
  if (size!=size0)
    throw Err() << "string_unpack_bbox: wrong data size: " << size;
  int32_t crd[4];
  s.read((char*)&crd, size0);
  if (s.fail()) throw Err() << "string_unpack_bbox: read error";
  return dRect(dPoint(crd[0]/1e7, crd[1]/1e7), dPoint(crd[2]/1e7, crd[3]/1e7));
}

dRect
string_read_bbox(istream & s) {
  dPoint p1 = read_pt(s);
  dPoint p2 = read_pt(s);
  if (s.get()!='\n') throw Err() << "string_read_bbox: eol expected";
  if (s.fail()) throw Err() << "string_read_bbox: read error or unexpected eof";
  return dRect(p1,p2);
}

