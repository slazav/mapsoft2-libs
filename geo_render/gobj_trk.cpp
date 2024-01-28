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
    "Use transparent color (0..1, default - 0.5).");
  opts.add("trk_draw_dots", 1,0,g,
    "Draw dots (for normal drawing mode), default: 1.");
  opts.add("trk_draw_width", 1,0,g, "Track width factor, default: 3px.");
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
  o.put("trk_draw_transp", 0.5);
  o.put("trk_draw_mode", "normal");
  o.put("trk_draw_dots", 1);
  o.put("trk_draw_width", 3.0);
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

double
GObjTrk::calc_vel(const size_t i, const size_t j){
  if (j==0) return NAN;

  auto & p1 = trk[i][j];
  auto & p2 = trk[i][j-1];
  double d = geo_dist_2d(p1,p2)/1000.0; // in m
  double t = (p1.t - p2.t)/1000.0; // in s
  return (t>0 && t<3600*24)? 3600*d/t : NAN;
}

double
GObjTrk::calc_alt(const size_t i, const size_t j){
  if (j==0) return trk[i][j].z;
  auto & p1 = trk[i][j];
  auto & p2 = trk[i][j-1];
  if (p1.have_alt() && p2.have_alt())
    return (p1.z + p2.z)/2.0;
  else
    return p1.z;
}


void
GObjTrk::update_data(){
  // calculate additional data
  points.clear();
  for (size_t i = 0; i<trk.size(); i++){
    points.push_back(std::vector<data_t>());
    for (size_t j = 0; j<trk[i].size(); j++){
      data_t p;
      // velocity and altitude (for the segment, not point!)
      // TODO: use time window
      p.vel = calc_vel(i,j);
      p.alt = calc_alt(i,j);
      points.rbegin()->push_back(p);
    }
  }
  update_crd();
  update_opt();
  redraw_me();
}

void
GObjTrk::update_crd(){
  range = dRect();
  for (size_t i = 0; i<trk.size(); i++){
    for (size_t j = 0; j<trk[i].size(); j++){
      auto & p = points[i][j];
      p.crd = cnv->bck_pts(trk[i][j]);
      range.expand(p.crd);
    }
  }
}

void
GObjTrk::update_opt(){

  linewidth = trk.opts.get<double>("thickness", 1.0)
            * opt.get<double>("trk_draw_width", 3.0);
  int  color  = trk.opts.get<int>("color", 0x0000FF);

  // color from track is always non-transparent.
  // set transparency (0..1)
  double tr = opt.get<double>("trk_draw_transp", 0.5);
  color = color | ((int)rint((1-tr)*255)<<24);

  // track drawing mode (normal, speed, height)
  std::string trk_mode = opt.get<string>("trk_draw_mode", "normal");

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

  // set colors
  for (size_t i = 0; i<points.size(); i++){
    for (size_t j = 0; j<points[i].size(); j++){
      auto & p = points[i][j];
      p.color = 0xFF000000;

      if (trk_mode == "normal"){
        p.color = color;
      }
      else if (trk_mode == "speed" && !isnan(p.vel)){
        p.color = RB.get(p.vel) + 0xFF000000;
      }
      else if (trk_mode == "height" && !isnan(p.alt)){
        p.color = RB.get(p.alt) + 0xFF000000;
      }
      else throw Err() << "GObjTrk: unknown track drawing mode" << trk_mode;
    }
  }
}



/********************************************************************/

GObj::ret_t
GObjTrk::draw(const CairoWrapper & cr, const dRect & draw_range){

  if (is_stopped()) return GObj::FILL_NONE;
  if (intersect(draw_range, bbox()).is_zsize()) return GObj::FILL_NONE;

  // draw selection
  if (selected) {

    for (size_t i = 0; i<trk.size(); i++){
      for (size_t j = 0; j<trk[i].size(); j++){
        auto & p1 = points[i][j];
        auto & p2 = points[i][j>0? j-1: points[i].size()-1];

        dRect r(p1.crd,p2.crd);
        r.expand(sel_w + linewidth*(dot_w+1));
        if (intersect(draw_range, r).is_zsize()) continue;

        if (j!=0){
          cr->move_to(p1.crd);
          cr->line_to(p2.crd);
        }
        if (draw_dots) cr->circle(p1.crd, dot_w*linewidth);
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

  for (size_t i = 0; i<trk.size(); i++){
    for (size_t j = 0; j<trk[i].size(); j++){
      auto & p1 = points[i][j];
      auto & p2 = points[i][j>0? j-1: points[i].size()-1];

      dRect r(p1.crd,p2.crd);
      r.expand((1+dot_w)*linewidth);
      if (intersect(draw_range, r).is_zsize()) continue;

      cr->set_color_a(p1.color);

      if (j!=0){
        cr->move_to(p1.crd);
        cr->line_to(p2.crd);
      }

      if (draw_dots) cr->circle(p2.crd, dot_w*linewidth);
      cr->stroke();
    }
  }

  return GObj::FILL_PART;
}


/********************************************************************/

std::vector<GObjTrk::idx_t>
GObjTrk::find_points(const dPoint & pt){
  double R = (dot_w+1)*linewidth;
  std::map<double, idx_t> m;

  for (size_t i = 0; i<points.size(); i++){
    for (size_t j = 0; j<points[i].size(); j++){
      double d = dist2d(points[i][j].crd, pt);
      if (d<R) m.emplace(d, idx_t(i,j));
    }
  }
  std::vector<idx_t> ret;
  for (const auto & x:m) ret.push_back(x.second);
  return ret;
}

std::vector<GObjTrk::idx_t>
GObjTrk::find_points(const dRect & r){
  std::vector<idx_t> ret;
  for (size_t i = 0; i<points.size(); i++){
    for (size_t j = 0; j<points[i].size(); j++){
      if (r.contains(points[i][j].crd))
        ret.push_back(idx_t(i,j));
    }
  }
  return ret;
}

std::vector<GObjTrk::idx_t>
GObjTrk::find_segments(const dPoint & pt){
  double R = linewidth; // search distance
  std::map<double, idx_t> m;
  std::vector<idx_t> ret;

  // todo: support for closed tracks
  for (size_t i = 0; i<points.size(); i++){
    for (size_t j = 1; j<points[i].size(); j++){
      auto & p1 = points[i][j];
      auto & p2 = points[i][j-1];

      double d12 = dist2d(p1.crd, p2.crd);
      if (d12 == 0.0) {
        double d = dist2d(p1.crd, pt);
        if (d<R) ret.push_back(idx_t(i,j));
        continue;
      }

      // for point projection to the segment
      // calculate length of p1->proj(pt)
      double vn = pscal2d(pt-p1.crd, p2.crd-p1.crd)/d12;
      if (vn < -R || vn > d12 + R) continue;

      double d;
      if (vn < 0) d = dist2d(p1.crd, pt);
      else if (vn > d12) d = dist2d(p2.crd, pt);
      else d = dist2d(pt-p1.crd, norm2d(p2.crd-p1.crd) * vn);
      if (d < R) m.emplace(d,idx_t(i,j));
    }
  }
  for (const auto & x:m) ret.push_back(x.second);
  return ret;
}

std::vector<dPoint>
GObjTrk::get_point_crd(const idx_t & idx) const {
  std::vector<dPoint> ret;

  if (idx.sn>=points.size() ||
      idx.pn>=points[idx.sn].size()) return ret;

  size_t i = idx.sn, j = idx.pn;
  ret.push_back(points[i][j].crd);

  // previous/next points
  if (j>0) ret.push_back(points[i][j-1].crd);
  if (j+1 < points[i].size()) ret.push_back(points[i][j+1].crd);
  return ret;
}

void
GObjTrk::set_point_crd(const idx_t & idx, const dPoint & pt) {
  auto lk = get_lock();
  if (idx.sn>=points.size() ||
      idx.pn>=points[idx.sn].size()) return;

  auto & p = points[idx.sn][idx.pn];
  auto & tpt = trk[idx.sn][idx.pn];

  dPoint pc = cnv->frw_pts(pt);

  // keep altitude and time
  tpt.x = pc.x;
  tpt.y = pc.y;

  // update additional data: only coordinates and velocity
  p.crd = pt;
  p.vel = calc_vel(idx.sn, idx.pn);

  // no need to update_data!
  redraw_me();
}

void
GObjTrk::add_point_crd(const idx_t & idx, const dPoint & pt) {
  auto lk = get_lock();

  if (idx.sn>=trk.size()) return;
  GeoTpt pto = cnv->frw_pts(pt);

  auto dest = idx.pn<trk[idx.sn].size() ?
    trk[idx.sn].begin()+idx.pn : trk[idx.sn].end();
  trk[idx.sn].insert(dest, pto);

  update_data();
  redraw_me();
}

void
GObjTrk::add_segment_crd(const dLine & pts) {
  auto lk = get_lock();
  GeoTrkSeg seg;
  for (size_t i=0; i<pts.size(); ++i) {
    GeoTpt tpt(pts[i]);
    cnv->frw(tpt);
    seg.push_back(tpt);
  }
  trk.push_back(seg);
  update_data();
  redraw_me();
}

GObjTrk::idx_t
GObjTrk::get_nearest_segment_end(const idx_t & idx) const {
  auto ret = idx;
  if (idx.sn < trk.size() && trk[idx.sn].size() > 0){
    auto i2 = trk[idx.sn].size()-1;
    ret.pn = idx.pn < i2-idx.pn ? 0:i2;
  }
  return ret;
}

void
GObjTrk::add_points_crd(const idx_t & idx, const dLine & pts){
  auto lk = get_lock();
  if (idx.sn>=trk.size()) return;

  GeoTrkSeg seg;
  for (auto const & p: pts) {
    GeoTpt tpt(p);
    cnv->frw(tpt);
    seg.push_back(tpt);
  }

  if (idx.pn == 0){
    trk[idx.sn].insert(trk[idx.sn].begin(), seg.rbegin(), seg.rend());
  }
  else if (idx.pn == trk[idx.sn].size()-1){
    trk[idx.sn].insert(trk[idx.sn].end(), seg.begin(), seg.end());
  }
  else {
    return;
  }

  update_data();
  redraw_me();
}

void
GObjTrk::del_point(const idx_t & idx){
  auto lk = get_lock();
  if (idx.sn>=trk.size() ||
      idx.pn>=trk[idx.sn].size()) return;
  trk[idx.sn].erase(trk[idx.sn].begin()+idx.pn);
  update_data();
  redraw_me();
}

void
GObjTrk::split_trk(const idx_t & idx){
  auto lk = get_lock();
  if (idx.sn>=trk.size() ||
      idx.pn>=trk[idx.sn].size()) return;
  GeoTrkSeg seg;
  seg.insert(seg.begin(),trk[idx.sn].begin()+idx.pn, trk[idx.sn].end());
  trk[idx.sn].erase(trk[idx.sn].begin()+idx.pn, trk[idx.sn].end());
  trk.insert(trk.begin()+idx.sn+1, seg);
  update_data();
  redraw_me();
}

void
GObjTrk::del_seg(const idx_t & idx){
  auto lk = get_lock();
  if (idx.sn>=trk.size()) return;
  trk.erase(trk.begin()+idx.sn);
  update_data();
  redraw_me();
}

void
GObjTrk::del_points(const dRect & r){
  auto lk = get_lock();
  for (size_t i = 0; i<points.size(); i++){
    for (size_t j = points[i].size(); j>0; j--){
      auto & p = points[i][j-1];
      if (r.contains(p.crd)) trk[i].erase(trk[i].begin()+j-1);
    }
  }
  update_data();
  redraw_me();
}
