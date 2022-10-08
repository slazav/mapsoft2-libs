#ifndef MAPDB_OBJ_H
#define MAPDB_OBJ_H

#include <string>
#include <set>

#include "geom/multiline.h"

// Class for vector map object.
// All coordinates are lat,lon in WGS84 datum.

/*********************************************************************/
// enums

typedef enum{
  MAPDB_POINT    = 0,
  MAPDB_LINE     = 1,
  MAPDB_POLYGON  = 2,
  MAPDB_TEXT     = 3
} MapDBObjClass;

typedef enum{
  MAPDB_ALIGN_SW = 0,
  MAPDB_ALIGN_W  = 1,
  MAPDB_ALIGN_NW = 2,
  MAPDB_ALIGN_N  = 3,
  MAPDB_ALIGN_NE = 4,
  MAPDB_ALIGN_E  = 5,
  MAPDB_ALIGN_SE = 6,
  MAPDB_ALIGN_S  = 7,
  MAPDB_ALIGN_C  = 8,
} MapDBObjAlign;

/*********************************************************************/
// MapDBObj -- a single map object

struct MapDBObj: public dMultiLine {

  // object type is assembled from following parts:
  // - first byte: object classification (MapDBObjClass)
  // - second byte: reserved;
  // - two last bytes: type number (= MP type)
  uint32_t        type;
  float           angle;   // object angle, deg
  float           scale;   // object scale
  MapDBObjAlign   align;   // align
  std::string     name;    // object name (to be printed on map labels)
  std::string     comm;    // object comment
  std::set<std::string> tags;    // object tags

  std::set<uint32_t> children;   // id's of related objects (usually labels)
  dPoint   ref_pt;    // type of parent object (for detached labels)
  uint32_t ref_type;  // coordinates of a parent object point (for detached labels)

  // defaults
  MapDBObj(const uint32_t t = 0):
      type(t), angle(std::nan("")),
      scale(1.0), align(MAPDB_ALIGN_SW), ref_type(0xFFFFFFFF) {}


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

  // get object classification (MapDBObjClass)
  MapDBObjClass get_class() const;

  // get object type number
  uint16_t get_tnum()  const;

  MapDBObj(const uint16_t cl, const uint16_t tnum): MapDBObj(make_type(cl,tnum)) {}
  MapDBObj(const std::string & s): MapDBObj(make_type(s)) { }

  /***********************************************/

  // Set object coordinates from a string.
  // For point and text objects point expected, for lines and areas - line/multilines.
  void set_coords(const std::string & s);

  /***********************************************/

  // pack object to a string (for DB storage)
  static std::string pack(const MapDBObj & obj);

  // unpack object from a string (for DB storage)
  static MapDBObj unpack(const std::string & s);

  /***********************************************/
  // operators <=>
  /// Less then operator.
  bool operator< (const MapDBObj & o) const {
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
  bool operator== (const MapDBObj & o) const {
    bool ang_eq = (angle==o.angle || (std::isnan(angle) && std::isnan(o.angle)));
    return type==o.type && ang_eq && scale==o.scale && align==o.align &&
        name==o.name && comm==o.comm && tags==o.tags && children==o.children &&
        ref_type==o.ref_type && ref_pt==o.ref_pt &&
        dMultiLine::operator==(o);
  }
  // derived operators:
  bool operator!= (const MapDBObj & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const MapDBObj & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const MapDBObj & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const MapDBObj & other) const { return !(*this<=other); } ///< operator>

};
#endif
