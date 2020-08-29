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

// SRTM data interface

/********************************************************************/
#include "getopt/getopt.h"

// add SRTM group of options
void ms2opt_add_srtm(GetOptSet & opts);

/********************************************************************/


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

  // color parameters:

  /// how to draw data
  enum draw_mode_t {
    SRTM_DRAW_SHADES, // heights shaded with slope value
    SRTM_DRAW_HEIGHTS,
    SRTM_DRAW_SLOPES,
  } draw_mode;

  double hmin,hmax;  // limits for heights and shades modes
  double smin,smax;  // limits for slopes mode

  bool interp_holes; // interpolate holes in data
  uint32_t bgcolor;  // how to draw holes

  Rainbow R; // color converter


  /// load data into cache
  bool load(const iPoint & key);

  public:

    /// Constructor.
    SRTM(const Opt & o = Opt());

    // get srtm width
    int get_srtm_width() const {return srtm_width;}

    // Options can be used to change data dir
    void set_opt(const Opt & opt);

    // Get default options.
    static Opt get_def_opt();

    // Find set of points with same value (used
    // for hole interpolation in get_val) and its border.
    // `max` is max.set size (default is 0 for no limits).
    void plane_and_border(const iPoint& p,
       std::set<iPoint>& set, std::set<iPoint>& brd, int max=0);


    /// Get altitude value at a given point (integer coordinates).
    /// Hole interpolation can be switched with `interp` parameter.
    short get_val(const int x, const int y, const bool interp=false);

    /// Get value, 4-point linear interpolation, long-lat coordinates.
    /// Hole interpolation is done according with srtm_interp_holes option.
    short get_val_int4(const dPoint & p);

    /// Get value, 16-point cubic interpolation, long-lat coordinates.
    /// Hole interpolation is done according with srtm_interp_holes option.
    short get_val_int16(const dPoint & p);

    /// set new hight in cached data (used for interpolation)
    short set_val(const int x, const int y, const short h);

    /// Get slope (in degrees) at a given point (integer coordinates).
    /// Hole interpolation can be switched with `interp` parameter.
    double get_slope(const int x, const int y, const bool interp=false);

    /// Get slope, 4-point interpolation, long-lat coordinates.
    /// Hole interpolation is done according with srtm_interp_holes option.
    double get_slope_int4(const dPoint & p);


    // Get color for given height and slope, according with drawing options
    uint32_t get_color(const double h, const double s);

    /// Get color for a point (lon-lat coords), according with drawing options.
    uint32_t get_color(const dPoint & p) {
      return get_color(get_val_int4(p), get_slope_int4(p));}


    // make vector data: contours
    std::map<short, dMultiLine> find_contours(const dRect & range, int step);

    // make vector data: peaks
    std::map<dPoint, short> find_peaks(const dRect & range, int DH, int PS);

    // make vector data: holes
    dMultiLine find_holes(const dRect & range);



};

#endif
