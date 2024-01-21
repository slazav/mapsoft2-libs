#ifndef GEO_DATA_H
#define GEO_DATA_H

#include <vector>
#include <string>
#include <cmath>
#include "geom/point.h"
#include "geom/multiline.h"
#include "geom/rect.h"
#include "opt/opt.h"
#include "image/io.h"

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
  int64_t t;        ///< unix time (ms)

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
/// Single trackpoint, a child of dPoint with additional t field -
/// (integer timestamp, in milliseconds).
/// Can have "undefined" altitude,
/// in this case it is not used in transformations (See ConvGeo class).
/// Note: default z in GeoWpt is NaN, and in dPoint is 0;
///       GeoWpt(dPoint p) constructor sets z=NaN  if p.z==0
struct GeoTpt : dPoint {
  int64_t t;   ///< unix time (ms)

  /// constructor
  GeoTpt(): t(0) {z = nan("");}
  GeoTpt(const dPoint &p): dPoint(p), t(0) { if (p.z==0) clear_alt();}
  GeoTpt(const double x, const double y, const double z=nan(""),
         const int64_t t=0):  dPoint(x,y,z), t(t) {}

  /// check if altitude is defined
  bool have_alt() const {return !std::isnan(z);}

  /// set the altitude to undefined state
  void clear_alt() {z=nan("");}
};

/// Track segment
typedef Line<double,GeoTpt> GeoTrkSeg;

/********************************************************************/
/// track
struct GeoTrk : MultiLine<double,GeoTpt>{
  std::string name; ///< name
  std::string comm; ///< comment
  Opt opts; ///< Track options

  /// Constructor
  GeoTrk() {}

  /// Constructor: make track from dMultiLine
  GeoTrk(const dMultiLine & l);

  /// Constructor: make track from dLine
  GeoTrk(const dLine & l);

  /// Get x-y range in lon-lat coords.
  dRect bbox() const;

  /// convert to dLine (join all segments)
  operator dLine() const;

  /// convert to dMultiLine (keep segments)
  operator dMultiLine() const;

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

  std::string image;           ///< image file for the map (mask for tile maps)
  iPoint image_size;           ///< image dimensions (in image coordinates)
  double image_dpi;            ///< image dpi value (default: 300)

  // for tiled maps reference should be given for z=0
  int tile_size;               ///< image tile dimensions (for tiled maps, default: 256)
  bool tile_swapy;             ///< are image tiles swapped in y (for tiled maps)
  bool is_tiled;               ///< is it a tiled map?
  int  tile_minz, tile_maxz;   ///<

  /// Constructor: create empty map, set defaults
  GeoMap(): image_dpi(300), tile_size(256), tile_swapy(false), is_tiled(false),
            tile_minz(0), tile_maxz(18) {}

  /******************************************************************/
  // operators +,-,/,*

  /// Add p to image coordinates (shift the map)
  /// TODO: maybe it is better not to shift image_bbox.
  /// now there is no reason to add/subtract points.
  GeoMap & operator+= (const dPoint & p) {
    std::map<dPoint,dPoint> ref1;
    for (auto i:ref) ref1.emplace(i.first+p, i.second);
    ref.swap(ref1);
    border+=p;
    return *this;
  }

  /// Multiply image coordinates by k (scale the map)
  GeoMap & operator*= (const double k) {
    std::map<dPoint,dPoint> ref1;
    for (auto i:ref) ref1.emplace(i.first*k, i.second);
    ref.swap(ref1);
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
    if (is_tiled != other.is_tiled) return (is_tiled < other.is_tiled);
    if (tile_swapy != other.tile_swapy) return (tile_swapy < other.tile_swapy);
    return false;
  }

  /// Equality opertator
  bool operator== (const GeoMap & other) const {
    return (name==other.name)&&(comm==other.comm)&&(ref==other.ref)&&
           (border==other.border)&&(proj==other.proj)&&(image==other.image)&&
           (image_size==other.image_size)&&(image_dpi==other.image_dpi)&&
           (tile_size==other.tile_size)&&(is_tiled==other.is_tiled)&&
           (tile_swapy==other.tile_swapy);
  }

  // derived operators:
  bool operator!= (const GeoMap & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const GeoMap & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const GeoMap & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const GeoMap & other) const { return !(*this<=other); } ///< operator>

  /******************************************************************/

  bool empty() const {return ref.size()==0;}

  void add_ref(const double x1, const double y1, const double x2, const double y2){
    ref.insert(std::make_pair(dPoint(x1,y1), dPoint(x2,y2))); }

  void add_ref(const dPoint & p1, const dPoint & p2){
    ref.insert(std::make_pair(p1,p2)); }

  void add_ref(const dLine & lr, const dLine & lw){
    if (lr.size()!=lw.size())
      throw Err() << "GeoMap::add_ref: wrong number of ref points";
    for (size_t i = 0; i<lr.size(); i++)
       ref.insert(std::make_pair(lr[i],lw[i]));
  }

  // bbox of reference points in image coordinates
  dRect bbox_ref_img() const{
    dRect r;
    for (auto const & pp:ref) r.expand(pp.first);
    return r;
  }

  // bbox of reference points in wgs84 latlong
  dRect bbox_ref_wgs() const{
    dRect r;
    for (auto const & pp:ref) r.expand(pp.second);
    return r;
  }

  /******************************************************************/

  /// bbox of the map (image, border, refpoints).
  /// Could be wrong if actual size of image differs from image_size.
  dRect bbox() const {
    // if image_size is non-zero then return it
    if (len2d(image_size)>0) return dRect(dPoint(), image_size);

    // if not - use ref and border extents:
    dRect r;
    for (auto const & pp:ref) r.expand(pp.first);
    for (auto const & l:border)
      for (auto const & p:l) r.expand(p);
    return r;
  }

  /******************************************************************/
  // update image_size (and check that file is readable)
  void update_size() {
    if (is_tiled) return;
    image_size = ::image_size(image);
  }

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

  dRect bbox_wpts() const;
  dRect bbox_trks() const;

  // add data from another geo_data object
//  void add(const geo_data & w);

};

///@}
///@}
#endif

