#include <vector>
#include <cmath>

#include "geom/line.h"
#include "rainbow/rainbow.h"
#include "geo_data/geo_utils.h"

#include "gobj_trk.h"

using namespace std;

void
ms2opt_add_drawtrk(GetOptSet & opts){
  const char *g = "DRAWTRK";
  opts.add("trk_draw_mode", 1,0,g,
    "Track drawing mode (normal, speed, height).");
  opts.add("trk_draw_th", 1,0,g,
    "Line thickness, overrides track setting.");
  opts.add("trk_draw_color", 1,0,g,
    "Color (for normal drawing mode), overrides track color.");
  opts.add("trk_draw_dots", 1,0,g,
    "Draw dots (for normal drawing mode), default: 1.");
  opts.add("trk_draw_arrows", 1,0,g,
    "Draw arrows (for normal drawing mode), default: 0.");
  opts.add("trk_draw_min",    1,0,g,
    "Min value (km/h for speed mode, m for height mode).");
  opts.add("trk_draw_max",    1,0,g,
    "Max value (km/h for speed mode, m for height mode).");
  opts.add("trk_draw_grad",   1,0,g,
    "Color gradient (for speed or height modes), default: BCGYRM.");
}

/********************************************************************/

GObjTrk::GObjTrk(GeoTrk & trk_): trk(trk_){
  segments.resize(trk.size());
}

int
GObjTrk::draw(const CairoWrapper & cr, const dRect & draw_range){

  if (is_stopped()) return GObj::FILL_NONE;
  if (intersect(draw_range, range).is_zsize()) return GObj::FILL_NONE;

  int arr_w = linewidth * 2.0;
  int dot_w = linewidth * 0.5;
  int arr_dist = linewidth * 10; // minimal segment with arrow

  // draw all segments
  cr->cap_round();
  cr->set_line_width(linewidth);
  for (int i = 0; i<segments.size(); ++i){

    if (is_stopped()) return GObj::FILL_NONE;

    dPoint p1 = segments[i].p1;
    dPoint p2 = segments[i].p2;
    dRect r(p1,p2);
    r.expand(arr_w + dot_w + linewidth);
    if (intersect(draw_range, r).is_zsize()) continue;

    cr->set_color_a(segments[i].color);

    if (!segments[i].hide){
      cr->move_to(p1);
      cr->line_to(p2);
    }

    if (draw_dots ||
        (segments[i].hide && segments[i>0?i-i:trk.size()-1].hide)){
      cr->circle(p1, dot_w);
    }

    if (!segments[i].hide && draw_arrows && (len2d(p1-p2) > arr_dist)){
      dPoint p0=(p1+p2)/2;
      dPoint dp = norm2d(p1-p2) * arr_w;
      dPoint p3 = p0 + dp + dPoint(dp.y, -dp.x) * 0.5;
      dPoint p4 = p0 + dp - dPoint(dp.y, -dp.x) * 0.5;
      cr->move_to(p0);
      cr->line_to(p3);
      cr->line_to(p4);
      cr->line_to(p0);
    }
    cr->stroke();
  }

  return GObj::FILL_PART;
}

void
GObjTrk::update_range(){
  range = dRect();
  for (auto const & s:segments)
    range.expand(s.p1);
  range.expand(linewidth);
}

/***************/

void
GObjTrk::set_opt(const Opt & opt){
  linewidth = trk.opts.get<double>("thickness", 1);
  linewidth = opt.get<double>("trk_draw_th", linewidth);

  bool closed = trk.opts.get<double>("closed", false);

  // Track color. Take value from the track, override by
  // value from trk_draw_color option if it exists.
  // Color from track is always non-transparent, but
  // option can set a transparent color.
  int  color  = trk.opts.get<int>("color", 0xFFFF000);
  color |= 0xFF000000;
  color       = opt.get("trk_draw_color", color);

  // track drawing mode (normal, speed, height)
  string trk_mode = opt.get<string>("trk_draw_mode", "normal");

  Rainbow RB(0,1);
  if (trk_mode == "normal"){
    draw_dots   = opt.get("trk_draw_dots", 1);
    draw_arrows = opt.get("trk_draw_arrows", 1);
  }
  else if (trk_mode == "speed"){
    draw_dots   = opt.get("trk_draw_dots", 0);
    draw_arrows = opt.get("trk_draw_arrows", 0);
    RB = Rainbow(opt.get<double>("trk_draw_min", 0),
                 opt.get<double>("trk_draw_max", 10),
                 opt.get<string>("trk_draw_grad", "BCGYRM").c_str());
  }
  else if (trk_mode == "height"){
    draw_dots   = opt.get("trk_draw_dots", 0);
    draw_arrows = opt.get("trk_draw_arrows", 0);
    RB = Rainbow(opt.get<double>("trk_draw_min", -200),
                 opt.get<double>("trk_draw_max", 8000),
                 opt.get<string>("trk_draw_grad", "BCGYRM").c_str());
  }


  // set segment colors
  for (int i=0; i<trk.size(); i++){

    segments[i].color = 0xFF000000;

    if (trk_mode == "normal"){
      segments[i].color = color;
    }
    else if (trk_mode == "speed"){
      if (i<trk.size()-1){
        double d = geo_dist_2d(trk[i], trk[i+1])/1000.0; // in m
        double t = (trk[i+1].t - trk[i].t)/1000.0; // in s
        if (t>0 && t<3600*24)
          segments[i].color = RB.get(d/t * 3600) + 0xFF000000;
      }
    }
    else if (trk_mode == "height"){
      if (i<trk.size()-1 && trk[i].have_alt() && trk[i+1].have_alt()){
        double z = (trk[i].z + trk[i+1].z)/2.0;
        segments[i].color = RB.get(z) + 0xFF000000;
      }
    }
    else throw Err() << "GObjTrk: unknown track drawing mode" << trk_mode;

    segments[i].hide = false;
    // if the point has start flag, then previous segment is
    // transparent.
    if (i>0 && trk[i].start) segments[i-1].hide = true;
  }

  // if track is not closed, the last segment should be transparent
  if (!closed) segments[trk.size()-1].hide = true;
}


void
GObjTrk::set_cnv(const std::shared_ptr<ConvBase> cnv) {
  if (trk.size() != segments.size())
    throw Err() << "GObjTrk: segments are not syncronized with track";

  for (int i=0; i<trk.size(); i++){
    dPoint pt(trk[i]);
    if (cnv) cnv->bck(pt);
    segments[i].p1 = pt;
    segments[i>0? i-1: trk.size()-1].p2 = pt;
  }
  update_range();
}

