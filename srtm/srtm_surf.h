#ifndef SRTM_SURF_H
#define SRTM_SURF_H

#include "srtm.h"
#include "rainbow/rainbow.h"
#include "opt/opt.h"

/*
A small object for drawing SRTM color surface.
These options are used in GObjSRTM and GobjPano,
comon code is here.
*/

/********************************************************************/
#include "getopt/getopt.h"

// add SRTM group of options
void ms2opt_add_srtm_surf(GetOptSet & opts);

/********************************************************************/


class SRTMSurf : public SRTM{
private:

  /// how to draw data
  enum draw_mode_t {
    SRTM_DRAW_SHADES, // heights shaded with slope value
    SRTM_DRAW_HEIGHTS,
    SRTM_DRAW_SLOPES,
  } draw_mode;

  double hmin,hmax;  // limits for heights and shades modes
  double smin,smax;  // limits for slopes mode
  uint32_t bgcolor;  // how to draw holes
  Rainbow R; // color converter

public:

  SRTMSurf(const Opt & o = Opt()) { set_opt(o); }

  // Get color for given height and slope, according with drawing options
  uint32_t get_color(const double h, const double s);

  /// Get color for a point (lon-lat coords), according with drawing options.
  uint32_t get_color(const dPoint & p);

  uint32_t get_bgcolor() const {return bgcolor;}

  static Opt get_def_opt();

  void set_opt(const Opt & o);
};

#endif
