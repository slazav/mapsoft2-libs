#ifndef STRING_PACK_H
#define STRING_PACK_H

#include <iostream>
#include <string>
#include <cstring>
#include <stdint.h>
#include "geom/multiline.h"


/**********************************************************/
// 1. To keep objects in a database we pack everything in std::strings.
// Usually it is something silmilar to RIFF format:
// 4-byte tag, 4-byte size, data.
//
// 2. To save objects to a text file we use a human-readable
// notation, "<tag> <data>" in each line. Symbols \n \\ \0 in
// text fields are quoted
//

// pack any type
template <typename T>
void string_pack(std::ostream & s, const char *tag, const T & v){
  if (strlen(tag)!=4) throw Err() << "string_pack_str: 4-byte tag expected";
  s.write(tag, 4);
  uint32_t size = sizeof(T);
  s.write((char *)&size, sizeof(uint32_t));
  s.write((char *)&v, size);
  if (s.fail()) throw Err() << "string_pack_str: write error";
}

// Same but for writing a text file
template <typename T>
void string_write(std::ostream & s, const char *tag, const T & v){
  s << tag << ' ' << v << '\n';
  if (s.fail()) throw Err() << "string_write: write error";
}

// Pack a string and write to the stream. Tag should contain 4 characters.
void string_pack_str(std::ostream & s, const char *tag, const std::string & str);

// Same but for writing a text file
// Quote \\, \n, \t, \0 symbols
void string_write_str(std::ostream & s, const char *tag, const std::string & str);

// Pack a multiline with LonLat coordinates (with multiple records, one tag per segment).
// Double values are multiplied by 1e7 and rounded to nearest integer values.
// Tag should contain 4 characters.
void string_pack_crds(std::ostream & s, const char *tag, const dMultiLine & ml);

// Same but for writing a text file
void string_write_crds(std::ostream & s, const char *tag, const dMultiLine & ml);

// Pack a point with LonLat coordinates.
// Double values are multiplied by 1e7 and rounded to nearest integer values.
// Tag should contain 4 characters.
void string_pack_pt(std::ostream & s, const char *tag, const dPoint & pt);

// Same but for writing a text file
void string_write_pt(std::ostream & s, const char *tag, const dPoint & pt);

// Pack bbox with LonLat coordinates.
// Double values are multiplied by 1e7 and rounded to nearest integer values.
// Tag should contain 4 characters.
void string_pack_bbox(std::ostream & s, const char *tag, const dRect & box);

// Same but for writing a text file
void string_write_bbox(std::ostream & s, const char *tag, const dRect & box);

/**********************************************************/

// read 4-byte tag, return empty string at the end of file,
// throw error if there is not enough data for the tag.
std::string string_unpack_tag(std::istream & s);

// Same but for reading a text file
std::string string_read_tag(std::istream & s);

// unpack any type (tag is already read)
template <typename T>
T string_unpack(std::istream & s){
  uint32_t size;
  s.read((char*)&size, sizeof(uint32_t));
  if (size != sizeof(T)) throw Err() << "string_unpack: bad data size";
  T ret;
  s.read((char*)&ret, size);
  if (s.fail()) throw Err() << "string_unpack_str: read error";
  return ret;
}

// Same for reading a text file
template <typename T>
T string_read(std::istream & s){
  T i;
  s >> i;
  if (s.get()!='\n') throw Err() << "string_read: eol expected";
  if (s.fail()) throw Err() << "string_read: read error or unexpected eof";
  return i;
}

// unpack string (tag is already read)
std::string string_unpack_str(std::istream & s);

// Same for reading a text file
std::string string_read_str(std::istream & s);

// unpack coordinate line (tag is already read)
dLine string_unpack_crds(std::istream & s);

// Same for reading a text file
dLine string_read_crds(std::istream & s);

// unpack point (tag is already read)
dPoint string_unpack_pt(std::istream & s);

// Same for reading a text file
dPoint string_read_pt(std::istream & s);

// unpack bbox (tag is already read)
dRect string_unpack_bbox(std::istream & s);

// Same for reading a text file
dRect string_read_bbox(std::istream & s);

#endif