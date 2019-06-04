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

// A single map object

struct VMapObj: public dMultiLine {
  VMapObjClass    cl;      // POINT, LINE, POLYGON
  int             type;    // = MP type
  VMapObjDir      dir;     // direction from mp, arrows from fig
  std::string     name;    // label text
  std::string     comm;    // comments
  std::string     src;     // object source
  VMapObj() {cl=POINT; type=0; dir=FRW;};

  // pack object to a string (for DB storage)
  std::string pack() const;

  // unpack object from a string (for DB storage)
  void unpack(const std::string & s);

  /******************************************************************/
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

// TODO: use DB storage instead of map!

class VMap {
private:
    std::map<int, VMapObj> storage;
    GeoHashStorage         geo_ind;
    dMultiLine  brd; // border (will be kept in the DB)
    dRect  bbox;     // bounding box (will be kept in the DB)

public:
  VMap() {};

  // get/set border
  dMultiLine get_brd() const {return brd;}
  void set_brd(const dMultiLine & b) { brd = b;}

  // get bbox
  dRect get_bbox() const {return bbox;}



  // add object to the map
  void add(const VMapObj & o);

  // import objects from MP file
  void import_mp(
    const std::string & mp_file,
    const std::string & conf_file);

  // export objects to MP file
  void export_mp(
    const std::string & mp_file,
    const std::string & conf_file);

};



#endif
