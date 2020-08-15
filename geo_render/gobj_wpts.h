#ifndef GOBJ_WPTS_H
#define GOBJ_WPTS_H

#include "cairo/cairo_wrapper.h"
#include "conv/conv_base.h"
#include "geo_data/geo_data.h"
#include "opt/opt.h"
#include "viewer/gobj.h"

/********************************************************************/
#include "getopt/getopt.h"

// add DRAWWPT group of options
void ms2opt_add_drawwpt(GetOptSet & opts);

/********************************************************************/

class GObjWpts : public GObj {
private:

  double text_size, size, linewidth;
  int color, bgcolor;
  std::string text_font;

  enum DrawStyleType {Normal, Multi, Skip};

  // Waypoint template for fast drawing: coorinates are converted,
  // text box is placed correctly, all parameters are set...
  struct WptDrawTmpl : dPoint{
    dPoint text_pt;
    dRect  text_box; // relative to text_pt!
    dRect  bbox;
    std::string name;
    GeoWpt * src;
    DrawStyleType style;
    WptDrawTmpl(): src(NULL), style(Normal) {};
  };

  int skip_dist;      // skip point if its label has to be drawn
                      // too far (distance in points).
  int stick_len;      // default flag stick length
  int text_pad;
  bool do_adj_pos;    // adjust text positions to avoid collisions
  bool do_adj_brd;    // adjust text positions fit into picture

  // Original data. It may be edited through the GObj interface.
  GeoWptList & wpts;

  // Templates. Should be syncronized with the data.
  std::vector<WptDrawTmpl> tmpls;

public:
  // constructor
  GObjWpts(GeoWptList & wpts);

  /************************************************/
  // These functions modify drawing templates, but
  // do not have any locking. They should be called
  // only from locked functions (on_set_opt, on_set_cnv, on_rescale)

  // Update template coordinates for a waypoint template (including bbox!).
  void update_pt_crd(WptDrawTmpl & t, const std::shared_ptr<ConvBase> cnv);

  // Update bbox for a waypoint template (after changing coordinates)
  void update_pt_bbox(WptDrawTmpl & t);

  // Update name and flag dimensions for a waypoint template.
  void update_pt_name(const CairoWrapper & cr, WptDrawTmpl & t);

  // Update range
  // Should be done after update_pt_crd() and update_pt_name()
  // and before adjust_text_pos() or any drawing.
  void update_range();

  // Adjust text positions to prevent collisions between points
  void adjust_text_pos();

  // Adjust text positions to fit into rng
  void adjust_text_brd(const dRect & rng);

  /************************************************/

  int draw(const CairoWrapper & cr, const dRect & draw_range) override;

  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  void set_opt(const Opt & o) override;

};

#endif
