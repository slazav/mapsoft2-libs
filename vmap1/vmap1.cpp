#include <list>
#include <vector>
#include <string>
#include <map>
#include <iostream>

//#include "2d/line_utils.h"
//#include "2d/line_rectcrop.h"
//#include "geo_io/geofig.h"
//#include "geo/geo_convs.h"
//#include "geo/geo_nom.h"
//#include "vmap/zn.h"
#include "vmap1.h"

using namespace std;

/***************************************/

bool
VMap1Lab::operator< (const VMap1Lab & o) const{
  if (pos != o.pos) return (pos < o.pos);
  if (dir != o.dir) return (dir < o.dir);
  if (hor != o.hor) return (hor < o.hor);
  if (ang != o.ang) return (ang < o.ang);
  return false;
}

VMap1Lab::VMap1Lab(){
  dir=0; ang=0.0; fsize=0; hor=true;
}

bool
VMap1Lfull::operator< (const VMap1Lfull & o) const{
  if (text != o.text) return (text < o.text);
  if (ref != o.ref) return (ref < o.ref);
  return VMap1Lab::operator<(o);
}

object_class
VMap1Obj::get_class() const{
  if (type & 0x200000) return POLYGON;
  if (type & 0x100000) return POLYLINE;
  return POI;
}

bool
VMap1Obj::operator< (const VMap1Obj & o) const{
  if (type != o.type) return (type < o.type);
  if (text != o.text) return (text < o.text);
  if (dir  != o.dir)  return (dir < o.dir);
  return dMultiLine::operator<(o);
}

VMap1Obj::VMap1Obj(){
  type=0; dir=0;
}

dRect
VMap1::range() const{
  dRect ret;
  for (auto const & o:*this) ret.expand(o.bbox2d());
  return ret;
}

void
VMap1::add(const VMap1 & W){
  mp_id  = W.mp_id;
  name   = W.name;
  style  = W.style;
  rscale = W.rscale;
  brd    = W.brd;
  insert(end(), W.begin(), W.end());
  lbuf.insert(lbuf.end(), W.lbuf.begin(), W.lbuf.end());
}

VMap1::VMap1(){
  mp_id=0;
  rscale=0;
  style="";
}