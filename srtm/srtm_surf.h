#ifndef SRTM_SURF_H
#define SRTM_SURF_H

#include "rainbow/rainbow.h"
#include "opt/opt.h"

/*
A small object for drawing SRTM color surface.
It is used at least in GObjSRTM and GobjPano.
*/

/********************************************************************/
#include "getopt/getopt.h"

// add SRTM group of options
void ms2opt_add_srtm_surf(GetOptSet & opts);

/********************************************************************/


class SRTMSurf {
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

  SRTMSurf(const Opt & o) { set_opt(o); }

  // Get color for given height and slope, according with drawing options
  uint32_t get_color(const double h, const double s);

  uint32_t get_bgcolor() const {return bgcolor;}

  static Opt get_def_opt();

  void set_opt(const Opt & o);
};

#endif
