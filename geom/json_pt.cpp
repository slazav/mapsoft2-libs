#include <string>
#include "point.h"
#include "line.h"
#include "multiline.h"
#include "rect.h"
#include "err/err.h"
#include <jansson.h>
/// functions for reading json lines, multilines
/// 

/**********************************************************/

// Get double value from a JSON array.
// Also support integer values written as a string in decimal or hex form.
// Examples: 0.1, 1, "10", "0xA"
// Array type and size should be checked before!
double
json_get_arr_val(json_t *array, int index){
  json_t *V = json_array_get(array, index);
  if (!V) throw Err() << "can't extract array index " << index;
  if (json_is_number(V)) return json_number_value(V);
  if (json_is_string(V)) return str_to_type<int>(json_string_value(V));
  throw Err() << "can't extract a number from array index " << index;
}

dPoint
json_to_point(json_t *P) {
  if (!json_is_array(P) || (json_array_size(P)!=2 && json_array_size(P)!=3))
    throw Err() << "a two-element JSON array expected";

  dPoint ret;
  ret.x = json_get_arr_val(P, 0);
  ret.y = json_get_arr_val(P, 1);

  if (json_array_size(P)==3)
    ret.z = json_get_arr_val(P, 2);

  return ret;
}

dLine
json_to_line(json_t *J) {
  if (!json_is_array(J))
    throw Err() << "a JSON array expected";
  dLine ret;
  json_t *P;
  size_t index;
  json_array_foreach(J, index, P){
    ret.push_back(json_to_point(P));
  }
  return ret;
}

dMultiLine
json_to_mline(json_t *J) {
  if (!json_is_array(J))
    throw Err() << "a JSON array expected";

  dMultiLine ret;
  json_t *L;
  size_t index;
  try {
    // try single line
    dLine l =json_to_line(J);
    if (l.size()>0) ret.push_back(l);
  }
  catch (Err & e){
    // try multiline
    json_array_foreach(J, index, L){ ret.push_back(json_to_line(L)); }
  }
  return ret;
}

dRect
json_to_rect(json_t *P) {
  if (!json_is_array(P))
    throw Err() << "a JSON array expected";

  if (json_array_size(P)==0) return dRect();

  if (json_array_size(P)!=4)
    throw Err() << "a four-element JSON array expected";

  dRect ret;
  ret.e = false;
  ret.x = json_get_arr_val(P, 0);
  ret.y = json_get_arr_val(P, 1);
  ret.w = json_get_arr_val(P, 2);
  ret.h = json_get_arr_val(P, 3);
  return ret;
}


/**********************************************************/

dPoint str_to_point(const std::string & str){
  dPoint ret;
  json_error_t e;
  json_t *J = json_loadb(str.data(), str.size(), 0, &e);

  try {
    if (!J) throw Err() << e.text;
    ret=json_to_point(J);
  }
  catch (Err & e){
    json_decref(J);
    throw Err() << "can't parse point: \"" << str << "\": " << e.str();
  }
  json_decref(J);
  return ret;
}

dLine str_to_line(const std::string & str){
  dLine ret;
  json_error_t e;
  json_t *J = json_loadb(str.data(), str.size(), 0, &e);

  try {
    if (!J) throw Err() << e.text;
    ret=json_to_line(J);
  }
  catch (Err & e){
    json_decref(J);
    throw Err() << "can't parse line: \"" << str << "\": " << e.str();
  }
  json_decref(J);
  return ret;
}

dMultiLine str_to_mline(const std::string & str){
  dMultiLine ret;
  json_error_t e;
  json_t *J = json_loadb(str.data(), str.size(), 0, &e);

  try {
    if (!J) throw Err() << e.text;
    ret=json_to_mline(J);
  }
  catch (Err & e){
    json_decref(J);
    throw Err() << "can't parse multisegment line: \"" << str << "\": " << e.str();
  }
  json_decref(J);
  return ret;
}

dRect str_to_rect(const std::string & str){
  dRect ret;
  json_error_t e;
  json_t *J = json_loadb(str.data(), str.size(), 0, &e);

  try {
    if (!J) throw Err() << e.text;
    ret=json_to_rect(J);
  }
  catch (Err & e){
    json_decref(J);
    throw Err() << "can't parse rectangle: \"" << str << "\": " << e.str();
  }
  json_decref(J);
  return ret;
}
