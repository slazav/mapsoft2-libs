#ifndef VMAP2OBJ_H
#define VMAP2OBJ_H

#include <string>
#include <set>

#include "geom/multiline.h"
#include "opt/opt.h"

// Class for VMAP2 object.
// All coordinates are lat,lon in WGS84 datum.

/*********************************************************************/
// enums

typedef enum{
  VMAP2_POINT    = 0,
  VMAP2_LINE     = 1,
  VMAP2_POLYGON  = 2,
  VMAP2_TEXT     = 3,
  VMAP2_NONE     = 0xFF
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

/*
Vmap2 format contains a number of independent objects of different type:
points, lines, polygons, and text.

Lebels are text objects and they do not have to be connected to other
objects. But in some cases we need this connection: When creating/aditing
an object we may want to create/update labels; when exporting vmap2 to
other format we may want to attach labels to objects.

We use soft connection, similar to mapsoft1: label contains object type and
object nearest point in `ref_type` and `ref_pt` fields.

Soft connection can be kept in formats which do not have object id's
(fig, mp, etc.). Also soft connection is useful when one
wants to update objects and keep old labels (this happens when somebody
sends me corrections in mp format).

*/


struct VMap2obj: public dMultiLine {

  // object type is assembled from following parts:
  // - first byte: object classification (VMap2objClass)
  // - second byte: reserved;
  // - three last bytes: type number
  //   note that type in MP format has only two bytes
  uint32_t        type;
  float           angle;   // object angle, CW, deg, (default is NaN)
  float           scale;   // object scale (default is 1.0)
  VMap2objAlign   align;   // align
  std::string     name;    // object name (to be printed on map labels)
  std::string     comm;    // object comment
  Opt             opts;    // object options

  dPoint   ref_pt;    // type of parent object (for detached labels)
  uint32_t ref_type;  // coordinates of a parent object point (for detached labels)

  // defaults
  VMap2obj(const uint32_t t = 0):
      type(t), angle(std::nan("")),
      scale(1.0), align(VMAP2_ALIGN_SW), ref_type(0xFFFFFFFF) {}

  /***********************************************/
  // type and ref_type operations:

  // assemble object type:
  static uint32_t make_type(const uint16_t cl, const uint32_t tnum);

  // parse object type from string (point|line|area|text):<number>
  static uint32_t make_type(const std::string & s);

  // set object type
  void set_type(const uint16_t cl, const uint32_t tnum){ type = make_type(cl, tnum);}

  // set object type from string
  void set_type(const std::string & s) {type = make_type(s);}

  // set object ref_type
  void set_ref_type(const uint16_t cl, const uint32_t tnum){ ref_type = make_type(cl, tnum);}

  // set object ref_type from string
  void set_ref_type(const std::string & s) {ref_type = make_type(s);}

  // check if type matches string
  bool is_type(const std::string & s) const {return make_type(s) == type;}

  // check if ref_type matches string
  bool is_ref_type(const std::string & s) const {return make_type(s) == ref_type;}

  // convert type to string with hex number
  static std::string print_type(const uint32_t t);

  // print type as string
  std::string print_type() const {return print_type(type);}

  // print ref_type as string
  std::string print_ref_type() const {return print_type(ref_type);}

  // convert type to string with decimal number
  static std::string print_type_dec(const uint32_t t);

  // print type as string
  std::string print_type_dec() const {return print_type_dec(type);}

  // print ref_type as string
  std::string print_ref_type_dec() const {return print_type_dec(ref_type);}


  // get type number for type t
  static uint32_t get_tnum(const uint32_t t) {return t & 0xFFFFFF;}

  // get object type number
  uint32_t get_tnum() const {return get_tnum(type);}

  // get ref_type type number
  uint32_t get_ref_tnum() const {return get_tnum(ref_type);}


  // get classification for a type t
  static VMap2objClass get_class(const uint32_t t);

  // get object classification (VMap2objClass)
  VMap2objClass get_class() const {return get_class(type);}

  // get ref_type classification (VMap2objClass)
  VMap2objClass get_ref_class() const {return get_class(ref_type);}

  // check if class matches
  bool is_class(const uint16_t cl) const {return get_class() == cl;}

  // check if ref_class matches
  bool is_ref_class(const uint16_t cl) const {return get_ref_class() == cl;}


  /***********************************************/

  VMap2obj(const uint16_t cl, const uint32_t tnum): VMap2obj(make_type(cl,tnum)) {}
  VMap2obj(const std::string & s): VMap2obj(make_type(s)) { }

  /***********************************************/

  static std::string print_align(const VMap2objAlign align);
  static VMap2objAlign parse_align(const std::string & str);

  // Add tags from a string with space-separated words.
  // This is needed for reading old vmap2 and mp files with Tags field.
  void add_tags(const std::string & s);

  /***********************************************/

  // Set object coordinates from a string.
  // For point and text objects point expected, for lines and areas - line/multilines.
  void set_coords(const std::string & s);

  void set_coords(const dMultiLine & ml){
    dMultiLine::operator=(ml);}

  void set_coords(const dLine & l){
    clear(); push_back(l);}

  void set_coords(const dPoint & pt){
    clear(); dLine l; l.push_back(pt); push_back(l);}


  /***********************************************/


  // pack object to a string (for DB storage)
  static std::string pack(const VMap2obj & obj);

  // unpack object from a string (for DB storage)
  static VMap2obj unpack(const std::string & s);

  // write object (to text file)
  static void write(std::ostream & s, const VMap2obj & obj);

  // read object (from a text file)
  static VMap2obj read(std::istream & s);



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
    if (opts!=o.opts)   return opts<o.opts;
    if (ref_type!=o.ref_type)   return ref_type<o.ref_type;
    if (ref_pt!=o.ref_pt)       return ref_pt<o.ref_pt;
    return dMultiLine::operator<(o);
  }

  /// compare header (but not coordinates) with another object
  bool is_same_head(const VMap2obj & o) const {
    bool ang_eq = (angle==o.angle || (std::isnan(angle) && std::isnan(o.angle)));
    return type==o.type && ang_eq &&
        scale==o.scale && align==o.align &&
        name==o.name && comm==o.comm && opts==o.opts &&
        ref_type==o.ref_type && ref_pt==o.ref_pt;
  }

  /// Equal opertator.
  bool operator== (const VMap2obj & o) const {
    return is_same_head(o) &&
        dMultiLine::operator==(o);
  }
  // derived operators:
  bool operator!= (const VMap2obj & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const VMap2obj & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const VMap2obj & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const VMap2obj & other) const { return !(*this<=other); } ///< operator>

};

/// \relates VMap2obj
/// \brief Output operator: print VMap2obj
std::ostream & operator<< (std::ostream & s, const VMap2obj & o);

/// \relates VMap2obj
/// \brief Input operator: read VMap2obj
std::istream & operator>> (std::istream & s, VMap2obj & o);

#endif
