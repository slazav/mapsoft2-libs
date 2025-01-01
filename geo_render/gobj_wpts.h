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

  // Original data. It may be edited through the GObj interface.
  GeoWptList & wpts;

  // coordinate conversion, viewer->wgs84
  std::shared_ptr<ConvBase> cnv;

  const double sel_w = 1.5; // pixels
  const uint32_t sel_col = 0x80FFFF00;

  double text_size, size, linewidth;
  int color, bgcolor;
  std::string text_font;
  bool selected;

  double skip_dist;      // skip point if its label has to be drawn
                         // too far (distance in points).
  double stick_len;      // default flag stick length
  double text_pad;
  bool do_adj_pos;    // adjust text positions to avoid collisions
  bool do_adj_brd;    // adjust text positions fit into picture

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
  // Templates. Should be syncronized with the data.
  std::vector<WptDrawTmpl> tmpls;

  dRect range; // data range

  /************************************************/
  // These functions modify drawing templates, but
  // do not have any locking. They should be called
  // only from locked functions (on_set_opt, on_set_cnv, on_rescale)


  // Update bbox for a waypoint template (after changing coordinates)
  void update_pt_bbox(WptDrawTmpl & t);

  // Adjust text positions to prevent collisions between points
  void adjust_text_pos();

  // Adjust text positions to fit into rng
  void adjust_text_brd(const dRect & rng);

public:

   // update templates (when data changed)
  void update_data();
  GeoWptList & get_data() {return wpts;} // use lock when modifying.


  // constructor
  GObjWpts(GeoWptList & wpts): wpts(wpts), cnv(new ConvBase), selected(false) {
    set_opt(get_def_opt()); // init all parameters
  }

  /************************************************/

  static Opt get_def_opt();

  void set_opt(const Opt & o) override;

  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

  ret_t check(const dRect &box) const override;

  dRect bbox() const override {return range;}

  /************************************************/

  // Find waypoints (point is in the waypoint circle or waypoint label).
  // Return point numbers.
  std::vector<size_t> find_points (const dPoint & pt);

  // Find all waypoints within rectangle r.
  // Return point numbers.
  std::vector<size_t> find_points(const dRect & r);

  // get viewer coordinates of point with index idx
  dPoint get_point_crd(const size_t idx) const;

  // set viewer coordinates of point with index idx
  void set_point_crd(const size_t idx, const dPoint & pt);

  // delete point with index idx
  void del_point(const size_t idx);

  // select/unselect waypoints
  void select(bool v=true) {selected = v; redraw_me();}

  // Delete all waypoints within rectangle r.
  void del_points(const dRect & r);

  // Add new waypoint
  void add_point(const GeoWpt & p);

};

#endif
