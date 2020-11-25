#include <map>
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
    "Track drawing mode (normal, speed, height, default - normal).");
  opts.add("trk_draw_transp", 1,0,g,
    "Use transparent color (0..1, default - 0).");
  opts.add("trk_draw_dots", 1,0,g,
    "Draw dots (for normal drawing mode), default: 1.");
  opts.add("trk_draw_smin",    1,0,g, "Min value for speed mode [km/h].");
  opts.add("trk_draw_smax",    1,0,g, "Max value for speed mode [km/h].");
  opts.add("trk_draw_hmin",    1,0,g, "Min value for height mode [m].");
  opts.add("trk_draw_hmax",    1,0,g, "Max value for height mode [m].");
  opts.add("trk_draw_grad",   1,0,g,
    "Color gradient (for speed or height modes), default: BCGYRM.");
}

Opt
GObjTrk::get_def_opt(){
  Opt o;
  o.put("trk_draw_transp", 0);
  o.put("trk_draw_mode", "normal");
  o.put("trk_draw_dots", 1);
  o.put("trk_draw_grad", "BCGYRM");
  // for speed mode
  o.put("trk_draw_smin", 0);
  o.put("trk_draw_smax", 10);
  // for height mode
  o.put("trk_draw_hmin", 0);
  o.put("trk_draw_hmax", 5000);
  return o;
}

void
GObjTrk::set_opt(const Opt & o){
  opt = o;
  update_opt();
  redraw_me();
}


void
GObjTrk::set_cnv(const std::shared_ptr<ConvBase> c) {
  cnv = c;
  update_crd();
  redraw_me();
}

void
GObjTrk::update_crd(){
  if (trk.size() != segments.size())
    throw Err() << "GObjTrk: segments are not syncronized with track";

  bool closed = trk.opts.get<double>("closed", false);

  for (size_t i=0; i<trk.size(); i++){
    dPoint pt(trk[i]);
    if (cnv) cnv->bck(pt);
    pt.z = 0;
    segments[i].p1 = pt;
    segments[i>0? i-1: trk.size()-1].p2 = pt;
    segments[i].hide = false;
    // if the point has start flag, then previous segment is transparent.
    if (i>0 && trk[i].start) segments[i-1].hide = true;
  }

  // if track is not closed, the last segment should be transparent
  if (!closed) segments[trk.size()-1].hide = true;

  // update range
  range = dRect();
  for (auto const & s:segments) range.expand(s.p1);
}

void
GObjTrk::update_opt(){
  if (trk.size() != segments.size())
    throw Err() << "GObjTrk: segments are not syncronized with track";

  linewidth = trk.opts.get<double>("thickness", 1);
  int  color  = trk.opts.get<int>("color", 0xFFFF000) | (0xFF << 24);

  // color from track is always non-transparent.
  // set transparency (0..1)
  double tr = opt.get<double>("trk_draw_transp", 0);
  color = color | ((int)rint((1-tr)*255)<<24);

  // track drawing mode (normal, speed, height)
  string trk_mode = opt.get<string>("trk_draw_mode", "normal");

  Rainbow RB(0,1);
  if (trk_mode == "normal"){
    draw_dots   = opt.get("trk_draw_dots", 1);
  }
  else if (trk_mode == "speed"){
    draw_dots   = 0;
    RB = Rainbow(opt.get<double>("trk_draw_smin", 0),
                 opt.get<double>("trk_draw_smax", 10),
                 opt.get<string>("trk_draw_grad", "BCGYRM").c_str());
  }
  else if (trk_mode == "height"){
    draw_dots   = 0;
    RB = Rainbow(opt.get<double>("trk_draw_hmin", -200),
                 opt.get<double>("trk_draw_hmax", 8000),
                 opt.get<string>("trk_draw_grad", "BCGYRM").c_str());
  }

  // set segment colors
  for (size_t i=0; i<trk.size(); i++){

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

  }
}

void
GObjTrk::update_data(){
  segments.resize(trk.size());
  update_opt();
  update_crd();
  redraw_me();
}


/********************************************************************/

GObj::ret_t
GObjTrk::draw(const CairoWrapper & cr, const dRect & draw_range){

  if (is_stopped()) return GObj::FILL_NONE;
  if (intersect(draw_range, bbox()).is_zsize()) return GObj::FILL_NONE;

  // draw selection
  if (selected) {
    for (size_t i = 0; i<segments.size(); ++i){
      if (is_stopped()) return GObj::FILL_NONE;
      dPoint p1 = segments[i].p1;
      dPoint p2 = segments[i].p2;
      dRect r(p1,p2);
      r.expand(sel_w + linewidth*(dot_w+1));
      if (intersect(draw_range, r).is_zsize()) continue;

      if (!segments[i].hide){
        cr->move_to(p1);
        cr->line_to(p2);
      }
      if (draw_dots ||
          (segments[i].hide && segments[i>0?i-i:trk.size()-1].hide)){
        cr->circle(p1, dot_w*linewidth);
      }
    }
    cr->cap_round();
    cr->set_line_width(linewidth + 2*sel_w);
    cr->set_color_a(sel_col);
    cr->stroke();
  }

  // draw all segments
  cr->cap_round();
  cr->set_line_width(linewidth);
  for (size_t i = 0; i<segments.size(); ++i){

    if (is_stopped()) return GObj::FILL_NONE;

    dPoint p1 = segments[i].p1;
    dPoint p2 = segments[i].p2;
    dRect r(p1,p2);
    r.expand((1+dot_w)*linewidth);
    if (intersect(draw_range, r).is_zsize()) continue;

    cr->set_color_a(segments[i].color);

    if (!segments[i].hide){
      cr->move_to(p1);
      cr->line_to(p2);
    }

    if (draw_dots ||
        (segments[i].hide && segments[i>0?i-i:trk.size()-1].hide)){
      cr->circle(p1, dot_w*linewidth);
    }
    cr->stroke();
  }

  return GObj::FILL_PART;
}


/********************************************************************/

std::vector<size_t>
GObjTrk::find_points(const dPoint & pt){
  double R = (dot_w+1)*linewidth;
  std::map<double, size_t> m;
  for (size_t i = 0; i < segments.size(); ++i){
    double d = dist(segments[i].p1, pt);
    if (d<R) m.emplace(d,i);
  }
  std::vector<size_t> ret;
  for (const auto & x:m) ret.push_back(x.second);
  return ret;
}

std::vector<size_t>
GObjTrk::find_points(const dRect & r){
  std::vector<size_t> ret;
  for (size_t i = 0; i < segments.size(); ++i)
    if (r.contains(segments[i].p1)) ret.push_back(i);
  return ret;
}

std::vector<size_t>
GObjTrk::find_segments(const dPoint & pt){
  double R = linewidth; // search distance
  std::map<double, size_t> m;
  std::vector<size_t> ret;


  if (segments.size() == 0) return ret;

  for (size_t i=0; i<segments.size(); ++i) {
    if (segments[i].hide) continue;
    auto p1 = segments[i].p1;
    auto p2 = segments[i].p2;

    double d12 = dist(p2,p1);
    if (d12 == 0) {
      double d = dist(segments[i].p1,pt);
      if (d<R) ret.push_back(i);
      continue;
    }

    // for point projection to the segment
    // calculate length of p1->proj(pt)
    double vn = pscal(pt-p1, p2-p1)/d12;

    if (vn < -R || vn > d12 + R)
      continue;

    double d;
    if (vn < 0)
      d = dist(segments[i].p1, pt);
    else if (vn > d12)
      d = dist(segments[i].p2, pt);
    else {
      d = dist(pt-p1, norm(p2-p1) * vn);
    }
    if (d < R) m.emplace(d,i);
  }
  for (const auto & x:m) ret.push_back(x.second);
  return ret;
}

std::vector<dPoint>
GObjTrk::get_point_crd(const size_t idx) const {
  std::vector<dPoint> ret;
  if (idx>=segments.size()) return ret;

  ret.push_back(segments[idx].p1);

  // next point
  if (!segments[idx].hide)
    ret.push_back(segments[idx].p2);

  // previous point
  size_t idxp = idx>0 ? idx-1: segments.size()-1;
  if (!segments[idxp].hide)
    ret.push_back(segments[idxp].p1);

  return ret;
}

void
GObjTrk::set_point_crd(const size_t idx, const dPoint & pt) {
  auto lk = get_lock();
  if (idx>=trk.size()) return;
  // keep altitude and time
  auto z = trk[idx].z;
  trk[idx].dPoint::operator=(pt);
  cnv->frw(trk[idx]);
  trk[idx].z = z;
  update_crd();
  redraw_me();
}

void
GObjTrk::add_point_crd(const size_t idx, const dPoint & pt) {
  auto lk = get_lock();
  if (idx>=trk.size()) return;
  GeoTpt p(pt);
  cnv->frw(p);
  trk.insert(trk.begin()+idx+1, p);
  update_data();
  redraw_me();
}

void
GObjTrk::add_segment_crd(const dLine & pts) {
  auto lk = get_lock();
  for (size_t i=0; i<pts.size(); ++i) {
    GeoTpt tpt(pts[i]);
    cnv->frw(tpt);
    tpt.start = (i==0);
    trk.insert(trk.end(), tpt);
  }
  update_data();
  redraw_me();
}

size_t
GObjTrk::get_nearest_segment_end(const size_t idx) const {
  if (trk.size()==0) return 0;
  if (idx>=trk.size()) return trk.size()-1;
  // find segment which contains idx (0 <= idx1 <= idx <= idx2 < trk.size())
  size_t idx1 = 0, idx2 = trk.size()-1;
  for (size_t i=0; i<trk.size(); ++i) {
    if (trk[i].start && i<=idx) idx1=i;
    if (trk[i].start && i>idx)  {idx2=i-1; break;}
  }
  // find which end is closer to idx:
  bool start = (idx-idx1 < idx2-idx);
  return start? idx1:idx2;
}

void
GObjTrk::add_points_crd(const size_t idx, const dLine & pts){
  auto lk = get_lock();
  if (idx>=trk.size()) return;
  // idx coresponds to beginning of a segment: add points before idx
  bool start = trk[idx].start;

  if (start) {
    trk[idx].start = false;
    for (auto const & p: pts) {
      GeoTpt tpt(p);
      cnv->frw(tpt);
      trk.insert(trk.begin() + (start?idx:idx+1), tpt);
    }
    trk[idx].start = true;
  }
  else {
    for (auto p = pts.rbegin(); p!= pts.rend(); p++) {
      GeoTpt tpt(*p);
      cnv->frw(tpt);
      trk.insert(trk.begin() + (start?idx:idx+1), tpt);
    }
  }
  update_data();
  redraw_me();
}

void
GObjTrk::del_point(const size_t idx){
  auto lk = get_lock();
  if (idx>=trk.size()) return;

  // move start flag if needed
  if (idx < trk.size()-1 && trk[idx].start)
    trk[idx+1].start = true;

  trk.erase(trk.begin()+idx);
  update_data();
  redraw_me();
}

void
GObjTrk::split_trk(const size_t idx){
  auto lk = get_lock();
  if (idx>=trk.size()-1) return;
  trk[idx+1].start = 1;
  update_data();
  redraw_me();
}

void
GObjTrk::del_seg(const size_t idx){
  auto lk = get_lock();
  if (idx>=trk.size()) return;
  size_t i1=0, i2=trk.size();
  for (size_t i=0; i<trk.size(); ++i) {
    if (i<=idx && trk[i].start) {i1 = i;}
    if (i>idx  && trk[i].start) {i2 = i; break; }
  }
  trk.erase(trk.begin()+i1, trk.begin()+i2);
  update_data();
  redraw_me();
}

void
GObjTrk::del_points(const dRect & r){
  auto lk = get_lock();
  for (ssize_t i = segments.size()-1; i >= 0; --i)
    if (r.contains(segments[i].p1)) trk.erase(trk.begin()+i);
  update_data();
  redraw_me();
}
