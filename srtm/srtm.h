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
#include "image_cnt/image_trace.h"

/*
DEM/SRTM data access.

Original SRTM data: ftp://e0mss21u.ecs.nasa.gov/srtm/
Fixed SRTM data: from http://www.viewfinderpanoramas.org/dem3.html
ALOS data: https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm

- Default data directory is DIR=$HOME/.srtm_data, it can be changed
with --srtm_dir option.

- Data is stored in [SN][0-9][0-9][EF][0-1][0-9][0-9].hgt
or .hgt.gz, .tif, or .tiff files. Each file contains 1x1 degree area.
Different resolutions and formats can be mixed in a singe folder.

SRTM data: https://surferhelp.goldensoftware.com/subsys/HGT_NASA_SRTM_Data_File_Description.htm
1x1-degree tiles, 1201x1201 or 3601x3601 pixels. Resolution is 1/1200 or 1/3600 degree.
Center of lower-left pixel is at integer degree coordinate. First and last column/row are identical.

ALOS format: https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/data/aw3d30v4.1_product_e_1.0.pdf
1x1-degree tiles, 3600x3600, 1800x3600, 1200x3600, 600x3600 pixels, TIFF
It's not so clearly written, but I assume that resolution is 1/3600, 1/1800, etc. degree,
center of lower-left point is at integer degree coordinate + 1/2 of the resolution.

*/

// default size of SRTM tile cache
#define SRTM_CACHE_SIZE 32

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

struct SRTMTile: public ImageR {
  SRTMTile(const std::string & dir, const iPoint & key); // load tile

  iPoint key;
  std::map<iPoint, int16_t> overlay; // overlay data
  bool srtm;   // SRTM/ALOS data format
  dPoint step; // x/y steps in degrees
  size_t w, h; // image dimensions

  bool empty;
  bool is_empty() {return empty;} // slightly different from Image::is_empty

  // Convert lonlat coordinate to pixel coordinate
  inline dPoint ll2px(const dPoint & p){
    dPoint px((p.x - key.x)/step.x, (key.y + 1.0 - p.y)/step.y, p.z);
    if (!srtm) px-=dPoint(0.5,0.5);
    return px;
  }

  // Convert pixel coordinate to lonlat
  inline dPoint px2ll(const dPoint & p){
    dPoint ret(p);
    if (!srtm) ret +=dPoint(0.5,0.5);
    return dPoint(key.x + ret.x*step.x, key.y + 1.0 - ret.y*step.y, p.z);
  }

  // be sure that image type is IMAGE_16 and crd is in the image
  inline int16_t get_unsafe(const iPoint & crd, const bool use_overlay) {
    // Use overlay
    if (use_overlay && overlay.count(crd))
      return overlay.find(crd)->second;

    // obtain the point
    return get16(crd.x, crd.y);
  }


};

/********************************************************************/

class SRTM {

  /// SRTM data folder.
  std::string srtm_dir;

  /// data cache. key is lon,lat in degrees, images are of IMAGE_16 type
  Cache<iPoint, SRTMTile> srtm_cache;

  // Locking srtm cache
  std::mutex cache_mutex;

  bool use_overlay;

  // get tile
  inline SRTMTile & get_tile(const iPoint & key) {
    if (!srtm_cache.contains(key))
      srtm_cache.add(key, SRTMTile(srtm_dir, key));
    return srtm_cache.get(key);
  }

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
    } style_t;

    style_t srtm_interp;

    // Distance between points (dx,dy) at a given place.
    // (0,0) if data is missing.
    dPoint get_step(const iPoint& p);

    // Get points for interpolation (0,1,2,4 points) for a given tile.
    void get_interp_pts(const iPoint key, const dPoint & p, std::set<dPoint> & pts);

    // Get altitude at the nearest point
    double get_nearest(const dPoint& p);

    // Get altitude with bilinear interpolation
    double get_interp(const dPoint& p);

    // get with interpolation
    double get_h(const dPoint& p, bool raw=false);

    // get slope
    double get_s(const dPoint& p, bool raw=false);

    // Get raster image with original points (if possible)
    // rng -- lonlat range
    // blc -- return lonlat coordinate of lower-left point
    // step -- return conversion factor points -> degrees
    // Point grid is taken from the tile in the middle of the range.
    // Other tiles will be interpolated to this grid.
    ImageR get_img(const dRect & rng, dPoint & blc, dPoint & step);

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
    std::map<double, dMultiLine> find_contours(const dRect & range, double step, double vtol = 0.0);

    // make vector data: slope contours
    dMultiLine find_slope_contours(const dRect & range, double val, double vtol = 0.0);

    // make vector data: peaks
    dLine find_peaks(const dRect & range, double DH, size_t PS);

    // make vector data: rivers or mountains (parameters are described in image_cnt/image_trace.h)
    dMultiLine trace_map(const dRect & range, const int nmax, const bool down, const double mina,
          const start_detect_t start_detect = TRACE_START_SIDEH2, const double start_par = 10.0,
          const size_t smooth_passes = 2);

    // make vector data: holes
    dMultiLine find_holes(const dRect & range);

};

#endif
