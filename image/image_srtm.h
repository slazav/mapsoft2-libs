#ifndef IMAGE_SRTM_H
#define IMAGE_SRTM_H

#include <string>
#include <mutex>

#include "rainbow/rainbow.h"
#include "cache/cache.h"
#include "image_r.h"

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

//
#define SRTM_VAL_NOFILE -32767 // file not found
#define SRTM_VAL_UNDEF  -32768 // hole in data
#define SRTM_VAL_MIN    -32000 // min of altitude data (for testing)
#define SRTM_INT_ZERO    20000 // zero on interpolated data
#define SRTM_INT_MIN     10000 // min of interpolated data (for testing)

class ImageSRTM: public Image {

  /// SRTM data folder.
  std::string srtm_dir;

  /// SRTM data width
  size_t srtm_width;

  /// data cache. key is lon,lat in degrees, images are of IMAGE_16 type
  Cache<iPoint, ImageR> srtm_cache;

  /// try to do thread-safe cache
  std::mutex cache_mutex;

  /// size (m) of 1 srtm point lat bow
  double size0;

  /// area (m^2) of 1x1 srtm point on equator
  double area0;

  /// load data into cache
  bool load(const iPoint & key);

  /// how to draw the surface
  enum draw_mode_t {
    SRTM_DRAW_NONE,
    SRTM_DRAW_DEFAULT, // heights shaded with slope value
    SRTM_DRAW_HEIGHTS,
    SRTM_DRAW_SLOPES,
  } draw_mode;

  Rainbow R; // color converter

  public:


    /// Constructor.
    ImageSRTM(const Opt & o = Opt());

    // Options can be used to change data dir and
    // color options.
    void set_opt(const Opt & opt);

    /// get altitude value at a given point (integer coordinates)
    short get_val(const int x, const int y, const bool interp=false);

    /// get slope (in degrees) at a given point (integer coordinates)
    double get_slope(const int x, const int y, const bool interp);

    // get point color
    uint32_t get_color_fast(const int x, const int y) override {
      switch (draw_mode){

        case SRTM_DRAW_NONE: return 0;

        case SRTM_DRAW_SLOPES:
          return R.get(get_slope(x,y, false));

        case SRTM_DRAW_HEIGHTS:
          return R.get(get_val(x,y,false));

        case SRTM_DRAW_DEFAULT: {
          double c = R.get(get_val(x,y,false));
          double s = get_slope(x,y, false);
          return color_shade(c, 1-s/90.0);
        }
      }
      return 0;
    }

    std::ostream & print (std::ostream & s) const override{
      s << "ImageSRTM(" << srtm_dir << ")";
      return s;
    }
};

#endif
