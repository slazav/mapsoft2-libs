#ifndef GEO_DATA_H
#define GEO_DATA_H

#include <vector>
#include <string>
#include <cmath>
#include "geom/point.h"
#include "geom/multiline.h"
#include "geom/rect.h"
#include "opt/opt.h"

///\addtogroup libmapsoft
///@{

///\defgroup GeoData Mapsoft geodata classes and functions.
///@{

/********************************************************************/
/// Single waypoint, a child of dPoint. Can have "undefined" altitude,
/// in this case it is not used in transformations (See ConvGeo class).
/// Also has `name` and `comm` text fields, and time `t` (integer, in
/// milliseconds). All additional information lives in `opts` variable.
/// Note: default z in GeoWpt is NaN, and in dPoint is 0;
///       GeoWpt(dPoint p) constructor sets z=NaN  if p.z==0
struct GeoWpt : dPoint {
  std::string name; ///< name
  std::string comm; ///< comment
  Opt opts;         ///< Waypoint options
  time_t t;         ///< unix time (ms)

  /// constructors
  GeoWpt() {z=nan(""); t=0;}
  GeoWpt(const dPoint &p): dPoint(p), t(0){ if (p.z==0) clear_alt();}
  GeoWpt(const double x, const double y, const double z=nan("")):
    dPoint(x,y,z), t(0){}

  /// check if altitude is defined
  bool have_alt() const {return !std::isnan(z);}

  /// set the altitude to undefined state
  void clear_alt() {z=nan("");}
};

/********************************************************************/
/// Waypoint list. An std::vector of GeoWpt with additional `name` and
/// `comm` test fields and options `opts`.
struct GeoWptList : std::vector<GeoWpt>{
  std::string name; ///< name
  std::string comm; ///< comment
  Opt opts; ///< Waypoint list options

  /// constructor
  GeoWptList() {}

  /// get x-y range in lon-lat coords
  dRect bbox() const;

  /// set the altitude of all points to undefined state
  void clear_alt();
};

/********************************************************************/
/// Single trackpoint, a child of dPoint. Can have "undefined" altitude,
/// in this case it is not used in transformations (See ConvGeo class).
/// Also has time `t` (integer, in milliseconds) and `start` flag.
/// Note: default z in GeoWpt is NaN, and in dPoint is 0;
///       GeoWpt(dPoint p) constructor sets z=NaN  if p.z==0
struct GeoTpt : dPoint {
  bool start; ///< start flag
  time_t t;   ///< unix time (ms)

  /// constructor
  GeoTpt(): start(false), t(0) {z=nan("");}
  GeoTpt(const dPoint &p): dPoint(p), start(false), t(0) { if (p.z==0) clear_alt();}
  GeoTpt(const double x, const double y, const double z=nan(""),
         const bool start = false, const time_t t=0):
    dPoint(x,y,z), start(start), t(t) {}

  /// check if altitude is defined
  bool have_alt() const {return !std::isnan(z);}

  /// set the altitude to undefined state
  void clear_alt() {z=nan("");}
};

/********************************************************************/
/// track
struct GeoTrk : std::vector<GeoTpt>{
  std::string name; ///< name
  std::string comm; ///< comment
  Opt opts; ///< Track options

  /// Constructor
  GeoTrk() {}

  /// Get x-y range in lon-lat coords.
  dRect bbox() const;

  /// Get length in m (using Haversine formula).
  double length() const;

  /// convert to dLine
  operator dLine() const;

  /// set the altitude of all points to undefined state
  void clear_alt();
};


/********************************************************************/
/// GeoMap. Shows how to draw some region on a 2D picture.
struct GeoMap{
  std::string name; ///< name
  std::string comm; ///< comment
  std::map<dPoint,dPoint> ref; ///< reference points, mapping from image to geo coordinates
  dMultiLine border;           ///< map border (in image coordinates)
  std::string proj;            ///< map projection (option string for libproj)

  std::string image;           ///< image file for the map (folder for tile maps)
  iPoint image_size;           ///< image dimensions (in image coordinates)
  double image_dpi;            ///< image dpi value (default: 300)

  int tile_size;               ///< image tile dimensions (for tiled maps, default: 256)
  std::string tile_fmt;        ///< image tile format (for tiled maps)
  bool tile_yswap;             ///< are image tiles swapped in y (for tiled maps)

  /// Constructor: create empty map
  GeoMap(): image_dpi(300), tile_size(256), tile_yswap(false) {}

  /******************************************************************/
  // operators +,-,/,*

  /// Add p to image coordinates (shift the map)
  /// TODO: maybe it is better not to shift image_bbox.
  /// now there is no reason to add/subtract points.
  GeoMap & operator+= (const dPoint & p) {
    for (auto i:ref) i.second+=p;
    border+=p;
    return *this;
  }

  /// Multiply image coordinates by k (scale the map)
  GeoMap & operator*= (const double k) {
    for (auto i:ref) i.second*=k;
    border*=k;
    image_size*=k;
    return *this;
  }

  /// Add p to image coordinates (shift the map)
  GeoMap operator+ (const dPoint & p) const { GeoMap ret(*this); return ret+=p; }

  /// Subtract p from image coordinates (shift the map)
  GeoMap & operator-= (const dPoint & p) { *this+=-p; return *this; }

  /// Subtract p from image coordinates (shift the map)
  GeoMap operator- (const dPoint & p) const { GeoMap ret(*this); return ret+=-p; }

  /// Multiply image coordinates by k (scale the map)
  GeoMap operator* (const double k) const { GeoMap ret(*this); return ret*=k; }

  /// Divide image coordinates by k (scale the map)
  GeoMap & operator/= (const double k) { *this*=1.0/k; return *this; }

  /// Divide image coordinates by k (scale the map)
  GeoMap operator/ (const double k) const { GeoMap ret(*this); return ret*=1.0/k; }


  /******************************************************************/
  // operators <=>

  /// Less then operator
  bool operator< (const GeoMap & other) const {
    if (name != other.name) return (name < other.name);
    if (comm != other.comm) return (comm < other.comm);
    if (ref != other.ref) return (ref < other.ref);
    if (border != other.border) return (border < other.border);
    if (proj != other.proj) return (proj < other.proj);
    if (image != other.image) return (image < other.image);
    if (image_size != other.image_size) return (image_size < other.image_size);
    if (image_dpi != other.image_dpi) return (image_dpi < other.image_dpi);
    if (tile_size != other.tile_size) return (tile_size < other.tile_size);
    if (tile_fmt != other.tile_fmt) return (tile_fmt < other.tile_fmt);
    if (tile_yswap != other.tile_yswap) return (tile_yswap < other.tile_yswap);
    return false;
  }

  /// Equality opertator
  bool operator== (const GeoMap & other) const {
    return (name==other.name)&&(comm==other.comm)&&(ref==other.ref)&&
           (border==other.border)&&(proj==other.proj)&&(image==other.image)&&
           (image_size==other.image_size)&&(image_dpi==other.image_dpi)&&
           (tile_size==other.tile_size)&&(tile_fmt==other.tile_fmt)&&
           (tile_yswap==other.tile_yswap);
  }

  // derived operators:
  bool operator!= (const GeoMap & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const GeoMap & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const GeoMap & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const GeoMap & other) const { return !(*this<=other); } ///< operator>

  /******************************************************************/

  void add_ref(const double x1, const double y1, const double x2, const double y2){
    ref.insert(std::make_pair(dPoint(x1,y1), dPoint(x2,y2))); }

  void add_ref(const dPoint & p1, const dPoint & p2){
    ref.insert(std::make_pair(p1,p2)); }

  void add_ref(const dLine & lr, const dLine & lw){
    if (lr.size()!=lw.size())
      throw Err() << "GeoMap::add_ref: wrong number of ref points";
    for (int i = 0; i<lr.size(); i++)
       ref.insert(std::make_pair(lr[i],lw[i]));
  }

  // bbox of reference points in image coordinates
  dRect bbox_ref_img() const{
    dRect r;
    for (auto pp:ref) r.expand(pp.first);
    return r;
  }

  // bbox of reference points in wgs84 latlong
  dRect bbox_ref_wgs() const{
    dRect r;
    for (auto pp:ref) r.expand(pp.second);
    return r;
  }

  /******************************************************************/

//  /// Get x-y range in lon-lat coords (using border, ref, proj).
//  dRect bbox() const;
};

/********************************************************************/
/// map list
struct GeoMapList : public std::vector<GeoMap>{
  std::string name; ///< name
  std::string comm; ///< comment
  Opt opts; ///< Map list options

  /// constructor
  GeoMapList() {}

//  /// get x-y range in lon-lat coords
//  dRect bbox() const;
};

/******************************************************************/
// additional operators for 

/// Multiply coordinates by k (k*line = line*k)
/// \relates GeoMap
GeoMap operator* (const double k, const GeoMap & l);

/// Add p to every point (p+line = line+p)
/// \relates GeoMap
GeoMap operator+ (const dPoint & p, const GeoMap & l);

/********************************************************************/

struct GeoData{
  Opt opts;
  std::list<GeoWptList> wpts;
  std::list<GeoTrk>     trks;
  std::list<GeoMapList> maps;

  /// clear all data
  void clear() { wpts.clear(); trks.clear(); maps.clear();}

  bool empty() const { return wpts.empty() && trks.empty() && maps.empty(); }

  void push_back(const GeoWptList & d) {wpts.push_back(d);}
  void push_back(const GeoTrk     & d) {trks.push_back(d);}
  void push_back(const GeoMapList & d) {maps.push_back(d);}

//  /// get range of all maps in lon-lat coords, fast
//  dRect range_map() const;
//  /// get range of all maps in lon-lat coords using file size
//  dRect range_map_correct() const;
//  /// get range of all tracks and waypoints in lon-lat coords
//  dRect range_geodata() const;
//  /// get range of all data in lon-lat coords
//  dRect range() const;

  // add data from another geo_data object
//  void add(const geo_data & w);

};

//#ifdef SWIG
//%template(vector_GeoTpt) std::vector<GeoTpt>;
//%template(vector_GeoTrk)  std::vector<GeoTrk>;
//#endif  // SWIG

///@}
///@}
#endif

