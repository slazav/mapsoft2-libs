#ifndef VMAP2OBJ_H
#define VMAP2OBJ_H

#include <string>
#include <set>

#include "geom/multiline.h"

// Class for VMAP2 object.
// All coordinates are lat,lon in WGS84 datum.

/*********************************************************************/
// enums

typedef enum{
  VMAP2_POINT    = 0,
  VMAP2_LINE     = 1,
  VMAP2_POLYGON  = 2,
  VMAP2_TEXT     = 3
} VMap2objClass;

typedef enum{
  VMAP2_ALIGN_SW = 0,
  VMAP2_ALIGN_W  = 1,
  VMAP2_ALIGN_NW = 2,
  VMAP2_ALIGN_N  = 3,
  VMAP2_ALIGN_NE = 4,
  VMAP2_ALIGN_E  = 5,
  VMAP2_ALIGN_SE = 6,
  VMAP2_ALIGN_S  = 7,
  VMAP2_ALIGN_C  = 8,
} VMap2objAlign;

/*********************************************************************/
// VMap2obj -- a single map object

struct VMap2obj: public dMultiLine {

  // object type is assembled from following parts:
  // - first byte: object classification (VMap2objClass)
  // - second byte: reserved;
  // - two last bytes: type number (= MP type)
  uint32_t        type;
  float           angle;   // object angle, deg
  float           scale;   // object scale
  VMap2objAlign   align;   // align
  std::string     name;    // object name (to be printed on map labels)
  std::string     comm;    // object comment
  std::set<std::string> tags;    // object tags

  std::set<uint32_t> children;   // id's of related objects (usually labels)
  dPoint   ref_pt;    // type of parent object (for detached labels)
  uint32_t ref_type;  // coordinates of a parent object point (for detached labels)

  // defaults
  VMap2obj(const uint32_t t = 0):
      type(t), angle(std::nan("")),
      scale(1.0), align(VMAP2_ALIGN_SW), ref_type(0xFFFFFFFF) {}


  // assemble object type:
  static uint32_t make_type(const uint16_t cl, const uint16_t tnum);

  // parse object type from string (point|line|area|text):<number>
  static uint32_t make_type(const std::string & s);

  // convert type to string
  static std::string print_type(const uint32_t t);

  // set object type
  void set_type(const uint16_t cl, const uint16_t tnum){ type = make_type(cl, tnum);}

  // set object type from string
  void set_type(const std::string & s) {type = make_type(s);}

  // get object classification (VMap2objClass)
  VMap2objClass get_class() const;

  // get object type number
  uint16_t get_tnum()  const;

  VMap2obj(const uint16_t cl, const uint16_t tnum): VMap2obj(make_type(cl,tnum)) {}
  VMap2obj(const std::string & s): VMap2obj(make_type(s)) { }

  /***********************************************/

  // Set object coordinates from a string.
  // For point and text objects point expected, for lines and areas - line/multilines.
  void set_coords(const std::string & s);

  /***********************************************/


  // pack object to a string (for DB storage)
  static std::string pack(const VMap2obj & obj);

  // unpack object from a string (for DB storage)
  static VMap2obj unpack(const std::string & s);


  /***********************************************/
  // operators <=>
  /// Less then operator.
  bool operator< (const VMap2obj & o) const {
    if (type!=o.type)   return type<o.type;
    if (std::isnan(angle) && !std::isnan(o.angle)) return true;
    if (std::isnan(o.angle) && !std::isnan(angle)) return false;
    if (angle!=o.angle && !std::isnan(angle)) return angle<o.angle;
    if (scale!=o.scale) return scale<o.scale;
    if (align!=o.align) return align<o.align;
    if (name!=o.name)   return name<o.name;
    if (comm!=o.comm)   return comm<o.comm;
    if (tags!=o.tags)   return tags<o.tags;
    if (children!=o.children)   return children<o.children;
    if (ref_type!=o.ref_type)   return ref_type<o.ref_type;
    if (ref_pt!=o.ref_pt)       return ref_pt<o.ref_pt;
    return dMultiLine::operator<(o);
  }

  /// Equal opertator.
  bool operator== (const VMap2obj & o) const {
    bool ang_eq = (angle==o.angle || (std::isnan(angle) && std::isnan(o.angle)));
    return type==o.type && ang_eq && scale==o.scale && align==o.align &&
        name==o.name && comm==o.comm && tags==o.tags && children==o.children &&
        ref_type==o.ref_type && ref_pt==o.ref_pt &&
        dMultiLine::operator==(o);
  }
  // derived operators:
  bool operator!= (const VMap2obj & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const VMap2obj & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const VMap2obj & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const VMap2obj & other) const { return !(*this<=other); } ///< operator>

};
#endif
