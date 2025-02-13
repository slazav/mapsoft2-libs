#include <vector>
#include <cmath>

#include "geohash/storage.h"
#include "geom/line.h"

#include "gobj_wpts.h"

using namespace std;

void
ms2opt_add_drawwpt(GetOptSet & opts){

  const char *g = "DRAWWPT";
  opts.add("wpt_text_font", 1,0,g,
    "Waypoint font (default: \"serif\").");
  opts.add("wpt_text_size", 1,0,g,
    "Waypoint font size, pixels (default: 10).");
  opts.add("wpt_text_pad", 1,0,g,
    "Waypoint text padding, pixels (default: 2).");
  opts.add("wpt_draw_size", 1,0,g,
    "Waypoint dot radius, pixels (default: 3).");
  opts.add("wpt_line_width", 1,0,g,
    "Waypoint line width, (default: 1).");
  opts.add("wpt_stick_len", 1,0,g,
    "Length of waypoint flag stick (default: 10).");
  opts.add("wpt_color", 1,0,g,
    "Waypoint color (default: 0xFF000000).");
  opts.add("wpt_bgcolor", 1,0,g,
    "Waypoint background color (default: 0xFFFFFFFF).");
  opts.add("wpt_adj", 1,0,g,
    "Adjust waypoint flag positions to prevent mutual collisions (default: 1).");
  opts.add("wpt_adj_brd", 1,0,g,
    "Adjust waypoint flag positions to prevent boundary collisions (default: 0).");
  opts.add("wpt_skip_far", 1,0,g,
    "Skip points if their labels can not be placed close enough (default: 10*wpt_stick_len).");
}

Opt
GObjWpts::get_def_opt() {
  Opt o;
  o.put("wpt_text_font",  "serif");
  o.put("wpt_text_size",  10);
  o.put("wpt_text_pad",   2);
  o.put("wpt_draw_size",  3);
  o.put("wpt_line_width", 1);
  o.put("wpt_stick_len",  10);
  o.put("wpt_color",      0xFF000000);
  o.put("wpt_bgcolor",    0xFFFFFFFF);
  o.put("wpt_adj",        1);
  o.put("wpt_adj_brd",    0);
  o.put("wpt_skip_dist",  10*10);
  return o;
}

void
GObjWpts::set_opt(const Opt & opt){
  text_font = opt.get("wpt_text_font",  "serif");
  text_size = opt.get("wpt_text_size",  10.0);
  text_pad  = opt.get("wpt_text_pad",   2.0);
  size      = opt.get("wpt_draw_size",  3.0);
  linewidth = opt.get("wpt_line_width", 1.0);
  stick_len = opt.get("wpt_stick_len",  10.0);
  color     = opt.get("wpt_color",      0xFF000000);
  bgcolor   = opt.get("wpt_bgcolor",    0xFFFFFFFF);
  do_adj_pos = opt.get("wpt_adj", 1);
  do_adj_brd = opt.get("wpt_adj_brd", 0);
  skip_dist = opt.get("wpt_skip_dist", stick_len*10.0);
  update_data();
  redraw_me();
}


void
GObjWpts::set_cnv(const std::shared_ptr<ConvBase> c) {
  if (!c) throw Err() << "GObjWpts::set_cnv: cnv is NULL";
  cnv = c;
  update_data();
  redraw_me();
}

void
GObjWpts::update_data(){

  tmpls.resize(wpts.size());
  for (size_t i=0; i!=wpts.size(); ++i){
    tmpls[i].src = & wpts[i];
    tmpls[i].name = wpts[i].name;
    tmpls[i].text_box = dRect();
  }

  // Update text label sizes. We need a Cairo::context to
  // measure size of text.
  if (text_size>0){
    CairoWrapper cr;
    cr.set_surface_img(1,1);
    cr->set_fc_font(color, text_font.c_str(), text_size);
    for (auto & wt:tmpls){
      cr->move_to(0,0);
      wt.text_box = cr->get_text_extents(std::string());
      wt.text_box = cr->get_text_extents(wt.name);
      wt.text_box.expand(text_pad);
    }
  }

  // update all other coordinates
  for (auto & wt:tmpls){
    dPoint pt(*wt.src);
    if (cnv) cnv->bck(pt);
    wt.x = pt.x; wt.y = pt.y;
    wt.text_pt = (dPoint)wt;
    if (text_size>0){
      wt.text_pt.y -= text_size + text_pad + stick_len;
    }
    update_pt_bbox(wt);
  }

  if (do_adj_pos) adjust_text_pos();

  // update range
  range = dRect();
  for (auto & wt:tmpls) range.expand(wt.bbox);
  range.to_ceil();

}


/********************************************************************/

GObj::ret_t
GObjWpts::check(const dRect & draw_range) const {
  for (auto & wt:tmpls)
    if (!intersect(draw_range, wt.bbox).is_zsize()) return FILL_PART;
  return FILL_NONE;
}

GObj::ret_t
GObjWpts::draw(const CairoWrapper & cr, const dRect & draw_range) {

  if (is_stopped()) return GObj::FILL_NONE;
  if (check(draw_range) == FILL_NONE) return FILL_NONE;
  if (do_adj_brd) adjust_text_brd(draw_range);


  // selection
  if (selected){
    for (auto const & wt:tmpls){
      if (is_stopped()) return GObj::FILL_NONE;
      if (intersect(draw_range, wt.bbox).is_zsize()) continue;

      if (text_size > 0) {
        // flag sticks
        cr->move_to(wt);
        cr->line_to(wt.text_pt);
        // dots
        cr->circle(wt, size);
        // flags
        if (wt.style == Skip) continue;
        if (wt.style == Multi)
          cr->rectangle(wt.text_pt+wt.text_box+dPoint(2,2));
        cr->rectangle(wt.text_pt+wt.text_box);
      }
    }
    cr->set_line_width(linewidth + 2*sel_w);
    cr->set_color(sel_col);
    cr->stroke();
  }


  // use single line width for drawing
  cr->set_line_width(linewidth);
  cr->set_color(color);

  // first pass: flag sticks
  if (text_size > 0) {
    for (auto const & wt:tmpls){
      if (is_stopped()) return GObj::FILL_NONE;
      if (intersect(draw_range, wt.bbox).is_zsize()) continue;
      cr->move_to(wt);
      cr->line_to(wt.text_pt);
      cr->stroke();
    }
  }

  // second pass: dots
  for (auto const & wt:tmpls){
    if (is_stopped()) return GObj::FILL_NONE;
    if (intersect(draw_range, wt.bbox).is_zsize()) continue;
    // circle
    cr->circle(wt, size);
    cr->set_color(bgcolor);
    cr->fill_preserve();
    cr->set_color(color);
    cr->stroke();
  }

  // third pass: flags
  if (text_size > 0) {
    for (auto const & wt:tmpls){
      if (is_stopped()) return GObj::FILL_NONE;
      if (intersect(draw_range, wt.bbox).is_zsize()) continue;
      if (wt.style == Skip) continue;

      if (wt.style == Multi){
        cr->rectangle(wt.text_pt+wt.text_box+dPoint(2,2));
        cr->set_color(color);
        cr->stroke();
      }
      cr->rectangle(wt.text_pt+wt.text_box);
      cr->set_color(bgcolor);
      cr->fill_preserve();
      cr->set_color(color);
      cr->stroke();

      // text
      cr->move_to(wt.text_pt);
      cr->set_fc_font(color, text_font.c_str(), text_size);
      cr->show_text(wt.name);

      // debugging: draw waypoint bbox
      //cr->set_color(0xFFFF0000);
      //cr->rectangle(wt.bbox);
      //cr->stroke();
    }
  }

  return GObj::FILL_PART;
}

/**********************************************************/

void
GObjWpts::update_pt_bbox(WptDrawTmpl & wt){
  wt.bbox = dRect(dPoint(wt), dPoint(wt));
  wt.bbox.expand(size + linewidth);
  if (wt.text_box)
    wt.bbox.expand(wt.text_pt + wt.text_box);
  wt.bbox.expand(sel_w);
  wt.bbox.to_ceil();
}

void
GObjWpts::adjust_text_pos() {
  if (text_size <= 0) return;

  // create geohash storage
  GeoHashStorage db;
  db.set_db_range(range);

  // for each box
  for (int i=0; i<(int)tmpls.size(); ++i){
    dRect bi = tmpls[i].text_box + tmpls[i].text_pt;
    tmpls[i].style = Normal; // all points are Normal in the beginning
    db.put(i, bi);
    auto v = db.get(bi);
    // find box with smaller number which may touch this one
    int i0 = -1;
    for (auto j:v){
      if (j>=i) continue;
      dRect bj = tmpls[j].text_box + tmpls[j].text_pt;
      if (!intersect(bi,bj)) continue;
      i0=j;
      break;
    }
    if (i0==-1) continue;

    // delete old position from the db
    db.del(i, bi);

    // New text position: move current to the side of the previous one
    // It's important that point i in the new position does not
    // touch i0, otherwise infinite loop appear.
    dPoint new_pt = tmpls[i0].text_pt
       + dPoint(5, tmpls[i].text_box.h + linewidth + 2);

    // distances between point and text
    double dist0 = dist2d(tmpls[i0], tmpls[i0].text_pt);
    double dist1 = dist2d(tmpls[i], new_pt);
    double dist01 = dist2d(tmpls[i0], new_pt);
    double dist10 = dist2d(tmpls[i], tmpls[i0].text_pt);

    // should we swap flags?
    if (std::max(dist0,dist1) > std::max(dist01,dist10) &&
        dist01 <= skip_dist){

      dRect bi0 = tmpls[i0].text_box + tmpls[i0].text_pt;
      // swap positions
      tmpls[i].text_pt = tmpls[i0].text_pt;
      tmpls[i0].text_pt = new_pt;

      // update bboxes
      update_pt_bbox(tmpls[i0]);
      update_pt_bbox(tmpls[i]);
      // delete old i and i0 position from the db
      db.del(i, bi);
      db.del(i0, bi0);
      // put new i0 position to the database
      db.put(i0, tmpls[i0].text_box + tmpls[i0].text_pt);

      i = i0-1; // return back to i0
      continue;
    }

    // if it is too far
    if (dist1 > skip_dist){
      tmpls[i0].style = Multi;
      tmpls[i].style  = Skip;
      tmpls[i].text_pt = tmpls[i0].text_pt;
      continue;
    }

    // adjust box position
    tmpls[i].text_pt = new_pt;
    // update bbox
    update_pt_bbox(tmpls[i]);

    // Check that new position of the point i does not
    // touch i0. Avoid infinite loop.
    if ( intersect(tmpls[i].text_box + tmpls[i].text_pt,
                   tmpls[i0].text_box + tmpls[i0].text_pt)){
      std::cerr << "Error in GObjWpts::adjust_text_pos: can't move point properly\n";
      continue;
    }
    // put new position in the database
    db.put(i, tmpls[i].text_box + tmpls[i].text_pt);
    i--;
  }
}

void
GObjWpts::adjust_text_brd(const dRect & rng){
  if (text_size <= 0) return;
  for (auto & wt:tmpls){

    if (!rng.contains(wt)) continue;

    dPoint sh(0,0);
    dRect b = wt.text_box + wt.text_pt;
    if (b.x < rng.x) sh.x = rng.x - b.x;
    if (b.y < rng.y) sh.y = rng.y - b.y;

    if (b.x + b.w > rng.x + rng.w)
      sh.x = rng.x + rng.w - b.x - b.w;
    if (b.y + b.h > rng.y + rng.h)
      sh.y = rng.y + rng.h - b.y - b.h;
    wt.text_pt += sh;
    update_pt_bbox(wt);
  }
}

/**********************************************************/

std::vector<size_t>
GObjWpts::find_points(const dPoint & pt){
  std::vector<size_t> ret;
  for (size_t i = 0; i < tmpls.size(); ++i){
    double d = dist(tmpls[i], pt);
    if ((tmpls[i].text_box && contains(tmpls[i].text_box + tmpls[i].text_pt, pt)) ||
        d<size) ret.push_back(i);
  }
  return ret;
}

std::vector<size_t>
GObjWpts::find_points(const dRect & r){
  std::vector<size_t> ret;
  for (size_t i = 0; i < tmpls.size(); ++i)
    if (r.contains(tmpls[i])) ret.push_back(i);
  return ret;
}

dPoint
GObjWpts::get_point_crd(const size_t idx) const {
  if (idx>=tmpls.size()) return dPoint();
  return tmpls[idx];
}

void
GObjWpts::set_point_crd(const size_t idx, const dPoint & pt) {
  auto lk = get_lock();
  if (idx>=tmpls.size()) return;
  // keep altitude, time and other parameters
  auto z = wpts[idx].z;
  tmpls[idx].x = wpts[idx].x = pt.x;
  tmpls[idx].y = wpts[idx].y = pt.y;
  cnv->frw(wpts[idx]);
  wpts[idx].z = z;
  update_data();
  redraw_me();
}

void
GObjWpts::del_point(const size_t idx){
  auto lk = get_lock();
  if (idx>=wpts.size()) return;
  wpts.erase(wpts.begin()+idx);
  update_data();
  redraw_me();
}

void
GObjWpts::del_points(const dRect & r){
  auto lk = get_lock();
  for (ssize_t i = tmpls.size()-1; i >= 0; --i)
    if (r.contains(tmpls[i])) wpts.erase(wpts.begin()+i);
  update_data();
  redraw_me();
}

void
GObjWpts::add_point(const GeoWpt & p){
  auto lk = get_lock();
  wpts.push_back(p);
  update_data();
  redraw_me();
}

