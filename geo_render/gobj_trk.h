#ifndef GOBJ_TRK_H
#define GOBJ_TRK_H

#include "cairo/cairo_wrapper.h"
#include "conv/conv_base.h"
#include "geo_data/geo_data.h"
#include "opt/opt.h"

/*

Supported track options:
  thickness, closed, color

*/

/********************************************************************/
#include "getopt/getopt.h"

// add MS2OPT_DRAWTRK options
void ms2opt_add_drawtrk(ext_option_list & opts);

/********************************************************************/

void
draw_trk(CairoWrapper & cr, const iPoint & origin,
         ConvBase & cnv, GeoTrk & trk,
         const Opt & opt);

#include "viewer/gobj.h"

/************************************************/
class GObjTrk : public GObj {
private:

  // Original data. It can be edited through the GObj interface.
  GeoTrk & trk;

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
  GObjTrk(ConvBase & cnv, GeoTrk & trk, const Opt & opt);

  // drawing waypoints on the image
  int draw(const CairoWrapper & cr, const iPoint &origin) override;

  // recalculate range (after changing coordinates)
  void update_range();

  // update point coordinates
  void on_set_cnv() override;

  // rescale point coordinates, update range
  void on_rescale(double k) override;

  void on_set_opt(const Opt & opt);

};

#endif