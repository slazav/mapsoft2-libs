#ifndef SRTM_H
#define SRTM_H

#include <set>
#include <map>
#include <string>

#include "rainbow/rainbow.h"
#include "cache/cache.h"
#include "image/image_r.h"
#include "geom/multiline.h"

/*
Read-only access to a SRTM data.

Original data can be downloaded from ftp://e0mss21u.ecs.nasa.gov/srtm/
Fixed data can be downloaded from http://www.viewfinderpanoramas.org/dem3.html

SRTM data is stored in [SN][0-9][0-9][EF][0-1][0-9][0-9].hgt
or .hgt.gz files. Each one contains 1x1 degree area.

Default data directory is DIR=$HOME/.srtm_data

*/

// default size of SRTM cache
#define SRTM_CACHE_SIZE 6

// default data width
#define SRTM_DEF_WIDTH 1201

// max size of a hole for interpolation
#define SRTM_MAX_INT_PTS 100000

//
#define SRTM_VAL_NOFILE -32767 // file not found
#define SRTM_VAL_UNDEF  -32768 // hole in data
#define SRTM_VAL_MIN    -32000 // min of altitude data (for testing)
#define SRTM_INT_ZERO    20000 // zero on interpolated data
#define SRTM_INT_MIN     10000 // min of interpolated data (for testing)

class SRTM {

  /// SRTM data folder.
  std::string srtm_dir;

  /// SRTM data width
  size_t srtm_width;

  /// data cache. key is lon,lat in degrees, images are of IMAGE_16 type
  Cache<iPoint, ImageR> srtm_cache;

  /// size (m) of 1 srtm point lat bow
  double size0;

  /// area (m^2) of 1x1 srtm point on equator
  double area0;

  /// load data into cache
  bool load(const iPoint & key);

  public:

    /// Constructor.
    SRTM(const Opt & o = Opt());

    // get srtm width
    int get_srtm_width() const {return srtm_width;}

    // Options can be used to change data dir
    void set_opt(const Opt & opt);

    // Get options
    Opt get_opt() const;

    // Find set of points with same value (used
    // for hole interpolation in get_val) and its border.
    // `max` is max.set size (default is 0 for no limits).
    void plane_and_border(const iPoint& p,
       std::set<iPoint>& set, std::set<iPoint>& brd, int max=0);


    /// get altitude value at a given point (integer coordinates)
    short get_val(const int x, const int y, const bool interp=false);

    /// get altitude value at a given point (long-lat coordinates)
    short get_val(const dPoint & p, const bool interp=false) {
      return get_val(rint(p.x*(srtm_width-1)), rint(p.y*(srtm_width-1)), interp); }

    /// get value, 4-point linear interpolation, long-lat coordinates
    short get_val_int4(const dPoint & p, const bool interp=false);

    /// get value, 16-point cubic interpolation, long-lat coordinates
    short get_val_int16(const dPoint & p, const bool interp=false);


    /// set new hight in cached data (used for interpolation)
    short set_val(const int x, const int y, const short h);


    /// get slope (in degrees) at a given point (integer coordinates)
    double get_slope(const int x, const int y, const bool interp=false);

    /// get slope (in degrees) at a given point (long-lat coordinates)
    short get_slope(const dPoint & p, const bool interp=false) {
      return get_slope(rint(p.x*(srtm_width-1)), rint(p.y*(srtm_width-1)), interp); }

    /// get slope, 4-point interpolation, long-lat coordinates
    double get_slope_int4(const dPoint & p, const bool interp=false);


    // make vector data: contours
    std::map<short, dMultiLine> find_contours(const dRect & range, int step);

    // make vector data: peaks
    std::map<dPoint, short> find_peaks(const dRect & range, int DH, int PS);

    // make vector data: holes
    dMultiLine find_holes(const dRect & range);



};

#endif
