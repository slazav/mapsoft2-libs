#ifndef VMAP_H
#define VMAP_H

#include <string>
#include <list>
#include <map>

#include "geom/multiline.h"
#include "geohash/storage.h"
//#include "fig/fig.h"

// Class for vector map (without label information!)
// All coordinates are lat,lon in WGS84 datum.

/*********************************************************************/
// enums

typedef enum{
  POINT    = 0,
  LINE     = 1,
  POLYGON  = 2
} VMapObjClass;

typedef enum{
  NO    = 0,
  FRW   = 1,
  BCK   = 2
} VMapObjDir;

/*********************************************************************/
// VMapObj -- a single map object

struct VMapObj: public dMultiLine {
  VMapObjClass    cl;      // object class: POINT, LINE, POLYGON
  int             type;    // = MP type
  VMapObjDir      dir;     // object direction: NO, FRW, BCK
  std::string     name;    // object name (to be printed on map labels)
  std::string     comm;    // object comment
  std::string     src;     // object source

  // defaults
  VMapObj() {cl=POINT; type=0; dir=FRW;};

  // pack object to a string (for DB storage)
  std::string pack() const;

  // unpack object from a string (for DB storage)
  void unpack(const std::string & s);

  /***********************************************/
  // operators <=>
  /// Less then operator.
  bool operator< (const VMapObj & o) const {
    if (cl!=o.cl)     return cl<o.cl;
    if (type!=o.type) return type<o.type;
    if (dir!=o.dir)   return dir<o.dir;
    if (name!=o.name) return name<o.name;
    if (comm!=o.comm) return comm<o.comm;
    if (src!=o.src)   return src<o.src;
    return dMultiLine::operator<(o);
  }

  /// Equal opertator.
  bool operator== (const VMapObj & o) const {
    return cl==o.cl && type==o.type && dir==o.dir &&
        name==o.name && comm==o.comm && src==o.src &&
        dMultiLine::operator==(o);
  }
  // derived operators:
  bool operator!= (const VMapObj & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const VMapObj & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const VMapObj & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const VMapObj & other) const { return !(*this<=other); } ///< operator>

};


/*********************************************************************/
// VMap -- a storage for map objects

// TODO: use DB storage instead of map!
class VMap {
private:
    std::map<int, VMapObj> storage;
    GeoHashStorage         geo_ind;

    dMultiLine  brd; // border (will be kept in the DB)
    dRect  bbox;     // bounding box (will be kept in the DB)

public:
  VMap() {};

  /// Get border.
  dMultiLine get_brd() const {return brd;}

  /// Set border.
  void set_brd(const dMultiLine & b) { brd = b;}

  /// Get bbox.
  dRect get_bbox() const {return bbox;}

  /// Add object to the map.
  void add(const VMapObj & o);

  /// Import objects from MP file.
  void import_mp(
    const std::string & mp_file,
    const std::string & conf_file);

  /// Export objects to MP file.
  void export_mp(
    const std::string & mp_file,
    const std::string & conf_file);

};



#endif