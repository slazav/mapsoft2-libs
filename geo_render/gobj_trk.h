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

  // drawing options
  Opt opt;
  // coordinate conversion, viewer->wgs84
  std::shared_ptr<ConvBase> cnv;


  // Drawing parameters.
  const double dot_w = 0.5; // multiple of linewidth
  const double sel_w = 1.5; // pixels
  const uint32_t sel_col = 0x80FFFF00;
  double linewidth;
  bool draw_dots;
  bool selected;

  // Data for drawing track segments.
  // Must be updated when options, data, or cnv are changing.
  struct segment_t{
    dPoint p1, p2;
    bool hide;
    uint32_t color;
  };
  std::vector<segment_t> segments;

public:

  void update_crd(); // update segment coordinates (when cnv changed)
  void update_opt(); // update segment colors (when options changed)
  void update_data(); // update segments (when data changed)

  GeoTrk & get_data() {return trk;} // use lock when modifying.

  // constructor
  GObjTrk(GeoTrk & trk_): trk(trk_), cnv(new ConvBase),
      linewidth(1), draw_dots(true), selected(false) {
    update_data();
  }


  /************************************************/

  static Opt get_def_opt();

  void set_opt(const Opt & o) override;

  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

  // range with linewidths included
  dRect bbox() const override {
    return expand(range, (dot_w+1)*linewidth + sel_w); }

  /************************************************/

  // Find track points near pt within radius r.
  // Return point numbers, sorted by distance.
  std::vector<size_t> find_points (const dPoint & pt);

  // Find all track points within rectangle r.
  // Return point numbers.
  std::vector<size_t> find_points(const dRect & r);

  // Find segments near pt within radius r.
  // Return numbers of starting points.
  std::vector<size_t> find_segments(const dPoint & pt);

  // Get viewer coordinates of point with index idx,
  // coordinates of next and/or previous point if
  // segments are visible.
  std::vector<dPoint> get_point_crd(const size_t idx) const;

  // set viewer coordinates of point with index idx
  void set_point_crd(const size_t idx, const dPoint & pt);

  // add new point with viewer coordinates pt after index idx
  void add_point_crd(const size_t idx, const dPoint & pt);

  // add new segment using viewer coordinates
  void add_segment_crd(const dLine & pts);

  // get index of the nearest segment end
  size_t get_nearest_segment_end(const size_t idx) const;

  // continue segment by adding points at index idx
  void add_points_crd(const size_t idx, const dLine & pts);

  // delete track point with index idx
  void del_point(const size_t idx);

  // split track after point idx
  void split_trk(const size_t idx);

  // delete track segment which contains point with given index
  void del_seg(const size_t idx);

  // Delete all points within rectangle r.
  void del_points(const dRect & r);

  // select/unselect track
  void select(bool v=true) {
    if (selected ==v ) return;
    selected = v; redraw_me();
  }

};

#endif
