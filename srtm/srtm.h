#ifndef SRTM_H
#define SRTM_H

#include <set>
#include <map>
#include <string>
#include <mutex>

#include "cache/cache.h"
#include "image/image_r.h"
#include "geom/multiline.h"
#include "rainbow/rainbow.h"

/*
Read-only access to a SRTM data.

Original data can be downloaded from ftp://e0mss21u.ecs.nasa.gov/srtm/
Fixed data can be downloaded from http://www.viewfinderpanoramas.org/dem3.html

SRTM data is stored in [SN][0-9][0-9][EF][0-1][0-9][0-9].hgt
or .hgt.gz, or .tif files. Each one contains 1x1 degree area.

Default data directory is DIR=$HOME/.srtm_data

*/

// default size of SRTM cache
#define SRTM_CACHE_SIZE 32

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
void ms2opt_add_srtm_surf(GetOptSet & opts);

/********************************************************************/

class SRTM {

  /// SRTM data folder.
  std::string srtm_dir;

  /// data cache. key is lon,lat in degrees, images are of IMAGE_16 type
  Cache<iPoint, ImageR> srtm_cache;

  /// overlay
  std::map<iPoint, std::map<iPoint, int16_t> > overlay;

  // Locking srtm cache
  std::mutex cache_mutex;

  bool interp_holes; // interpolate holes in data

  /// load data into cache
  bool load(const iPoint & key);

  public:

    /// Constructor.
    SRTM(const Opt & o = Opt());

    // Options can be used to change data dir
    void set_opt(const Opt & opt);

    // Get default options.
    static Opt get_def_opt();

    // Get lock. SRTM class goes not lock anything itself,
    // User must lock set_opt, get_val, ... methods if they are
    // used from different threads.
    std::unique_lock<std::mutex> get_lock() {
      return std::unique_lock<std::mutex>(cache_mutex);}

    /******************************/
    // new interface

    typedef enum {
      SRTM_NEAREST=0,
      SRTM_LINEAR=1,
      SRTM_CUBIC=2,
    } style_t;

    style_t srtm_interp;

    // Distance between points (dx,dy) at a given place.
    // (0,0) if data is missing.
    dPoint get_step(const iPoint& p);

    // Low-level get function: rounding coordinate to the nearest point
    int16_t get_raw(const dPoint& p);

    // get with interpolation
    double get_h(const dPoint& p, bool raw=false);

    // get slope
    double get_s(const dPoint& p, bool raw=false);

    /******************************/
    // overlay things

    // cut hole in the data
    void overlay_cut(const dLine & l);

    void overlay_clear(const dLine & l);

    /******************************/
    // color surface interface

    enum draw_mode_t {
      SRTM_DRAW_SHADES, // heights shaded with slope value
      SRTM_DRAW_HEIGHTS,
      SRTM_DRAW_SLOPES,
    } draw_mode;

    double hmin,hmax;  // limits for heights and shades modes
    double smin,smax;  // limits for slopes mode
    uint32_t bgcolor;  // how to draw holes
    Rainbow R; // color converter

    // Get color for given height and slope, according with drawing options
    uint32_t get_color(const double h, const double s);

    /// Get color for a point (lon-lat coords), according with drawing options.
    uint32_t get_color(const dPoint & p, bool raw=false);

    uint32_t get_bgcolor() const {return bgcolor;}

    /******************************/

    // make vector data: contours
    // use kx parameter to use only every kx-th horizontal point.
    // if kx == 0 then use some latitude-dependent default.
    std::map<short, dMultiLine> find_contours(const dRect & range, int step, int kx=0, double smooth = 0.0);

    // make vector data: slope contours
    dMultiLine find_slope_contours(const dRect & range, double val, int kx=0, double smooth = 0.0);

    // make vector data: peaks
    std::map<dPoint, short> find_peaks(const dRect & range, int DH, size_t PS);

    // make vector data: holes
    dMultiLine find_holes(const dRect & range);


};

#endif
