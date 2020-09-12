#ifndef GOBJ_TRK_H
#define GOBJ_TRK_H

#include <vector>

#include "cairo/cairo_wrapper.h"
#include "conv/conv_base.h"
#include "geo_data/geo_data.h"
#include "opt/opt.h"
#include "viewer/gobj.h"

/*

Supported track options:
  thickness, closed, color

*/

/********************************************************************/
#include "getopt/getopt.h"

// add DRAWTRK group of options
void ms2opt_add_drawtrk(GetOptSet & opts);

/********************************************************************/

class GObjTrk : public GObj {
private:

  // Original data. It can be edited through the GObj interface.
  GeoTrk & trk;
  dRect range; // data range

  int linewidth;
  bool draw_dots;
  bool draw_arrows;

  struct segment_t{
    dPoint p1, p2;
    uint32_t color;
    bool hide;
  };

  std::vector<segment_t> segments;

public:
  // constructor
  GObjTrk(GeoTrk & trk);

  // recalculate range (after changing coordinates)
  void update_range();

  /************************************************/

  static Opt get_def_opt();

  void set_opt(const Opt & o) override;

  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

  dRect bbox() const override {return range;}

  /************************************************/

  // Find track points near pt within radius r.
  // Return point numbers, sorted by distance.
  std::vector<size_t> find_points (const dPoint & pt, const double r = 3.0);

  // Find all track points within rectangle r.
  // Return point numbers.
  std::vector<size_t> find_points(const dRect & r);

  // Find segments near pt within radius r.
  // Return numbers of starting points.
  std::vector<size_t> find_segments(const dPoint & pt, const double r = 3.0);

};

#endif
