#include "gobj_srtm.h"
#include "geom/poly_tools.h"
#include <sstream>
#include <fstream>

void
ms2opt_add_drawsrtm(GetOptSet & opts){

  ms2opt_add_srtm_surf(opts);

  const char *g = "DRAWSRTM";
  opts.add("srtm_surf", 1,0,g,
    "draw SRTM color surface (default 1).");
  opts.add("srtm_cnt",1,0,g,
    "Draw contours (0|1, default - 1).");
  opts.add("srtm_cnt_step",1,0,g,
    " Contour step [m], default 50.");
  opts.add("srtm_cnt_vtol",1,0,g,
    " altitude tolerance for smoothing contours [m], default 5");
  opts.add("srtm_cnt_smult",1,0,g,
    " Step multiplier for thick contours, default - 5.");
  opts.add("srtm_cnt_col",1,0,g,
    " Contour color, default - 0xFF000000.");
  opts.add("srtm_cnt_w",1,0,g,
    " Contour line width, default - 0.25.");
  opts.add("srtm_cnt_wmult",1,0,g,
    " Width multiplier for thick contours, default - 2.");
  opts.add("srtm_cnt_crv",1,0,g,
    " Size of round corners on contours, in linewidth units, default - 20.");
  opts.add("srtm_holes",1,0,g,
    " Draw contours around data holes (1|0, default - 0).");
  opts.add("srtm_holes_col",1,0,g,
    " Color of hole contours, default - 0xFF000000.");
  opts.add("srtm_holes_w",1,0,g,
    " Linewidth of hole contours, default - 1.");
  opts.add("srtm_peaks",1,0,g,
    " Draw summits (0|1, default - 0).");
  opts.add("srtm_peaks_col",1,0,g,
    " Summit's color, default - 0xFF000000.");
  opts.add("srtm_peaks_w",1,0,g,
    " Summit point size, default - 3.");
  opts.add("srtm_peaks_dh",1,0,g,
    " DH parameter for peak finder [m], default - 20.");
  opts.add("srtm_peaks_ps",1,0,g,
    " PS parameter fr peak finder [pts], default - 1000.");
  opts.add("srtm_peaks_text",1,0,g,
    " Draw peak text (0|1, default - 1).");
  opts.add("srtm_peaks_text_size",1,0,g,
    " Peak text size, default - 10.");
  opts.add("srtm_peaks_text_font",1,0,g,
    " Peak text font, default - serif.");

}

Opt
GObjSRTM::get_def_opt(){

  Opt o = SRTM::get_def_opt();

  // draw_surface?
  o.put("srtm_surf",    1);

  // contours parameters
  o.put("srtm_cnt",       1);
  o.put("srtm_cnt_step",  50);
  o.put("srtm_cnt_vtol",  5);
  o.put("srtm_cnt_smult", 5);
  o.put("srtm_cnt_col",   0xFF000000);
  o.put("srtm_cnt_w",     0.25);
  o.put("srtm_cnt_wmult", 2);
  o.put("srtm_cnt_crv",   20);

  // holes parameters
  o.put("srtm_holes",     0);
  o.put("srtm_holes_col", 0xFF000000);
  o.put("srtm_holes_w",   0.5);

  // peaks parameters
  o.put("srtm_peaks",      0);
  o.put("srtm_peaks_col",  0xFF000000);
  o.put("srtm_peaks_",     3);
  o.put("srtm_peaks_dh",   20);
  o.put("srtm_peaks_ps",   1000);
  o.put("srtm_peaks_text", 1);
  o.put("srtm_peaks_text_size", 10);
  o.put("srtm_peaks_text_font", "serif");
  return o;
}

void
GObjSRTM::set_opt(const Opt & o){
  if (srtm){
    auto srtm_lock = srtm->get_lock();
    srtm->set_opt(o);
  }

  surf = o.get("srtm_surf", 1);

  // contours parameters
  cnt          = o.get<bool>("srtm_cnt",      1);
  cnt_step     = o.get<int>("srtm_cnt_step",  50);
  cnt_vtol     = o.get<double>("srtm_cnt_vtol",  5);
  cnt_smult    = o.get<int>("srtm_cnt_smult", 5);
  cnt_color    = o.get<int>("srtm_cnt_col",   0xFF000000);
  cnt_w        = o.get<double>("srtm_cnt_w",    0.25);
  cnt_wmult    = o.get<double>("srtm_cnt_wmult",2);
  cnt_crv      = o.get<double>("srtm_cnt_crv",  20.0);

  // holes parameters
  holes        = o.get<bool>("srtm_holes",      0);
  holes_color  = o.get<int>("srtm_holes_col",  0xFF000000);
  holes_w      = o.get<double>("srtm_holes_w",  0.5);

  // peaks parameters
  peaks        = o.get<bool>("srtm_peaks",      0);
  peaks_color  = o.get<int>("srtm_peaks_col",  0xFF000000);
  peaks_w      = o.get<double>("srtm_peaks_w",  3);
  peaks_dh     = o.get<int>("srtm_peaks_dh",   20);
  peaks_ps     = o.get<int>("srtm_peaks_ps",  1000);
  peaks_text   = o.get<bool>("srtm_peaks_text",  1);
  peaks_text_size   = o.get<double>("srtm_peaks_text_size",  10);
  peaks_text_font   = o.get("srtm_peaks_text_font",  "serif");

  tiles.clear();
  redraw_me();
}

void
GObjSRTM::redraw() {
  tiles.clear();
  redraw_me();
}

void
GObjSRTM::set_cnv(const std::shared_ptr<ConvBase> c) {
  if (!c) throw Err() << "GObjSRTM::set_cnv: cnv is NULL";
  cnv = c;
  tiles.clear();
  redraw_me();
}


bool
GObjSRTM::
render_tile(const dRect & draw_range){
  if (!srtm) return false;

  ImageR image = ImageR(draw_range.w, draw_range.h, IMAGE_32ARGB);

  // calculate wgs range
  dRect wgs_range = cnv->frw_acc(draw_range);

  // draw surface
  if (surf) {
    auto srtm_lock = srtm->get_lock();

    // data resolution [deg/px]
    dPoint data_res((double)wgs_range.w/draw_range.w, (double)wgs_range.h/draw_range.h);

    // srtm resolution [deg/px]
    dPoint srtm_res = srtm->get_step(wgs_range.cnt());
    bool raw = data_res.x > srtm_res.x || data_res.y > srtm_res.y;

    for (size_t j=0; j<image.height(); j++){
      if (is_stopped()) return false;
      for (size_t i=0; i<image.width(); i++){
        dPoint p(i + draw_range.x, j+draw_range.y);
        if (cnv) cnv->frw(p);
        image.set32(i,j, srtm->get_color(p, raw));
      }
    }
  }
  else { image.fill32(0); }

  CairoWrapper cr;
  cr.set_surface_img(image);
  //  set some cairo parameters for drawing vector data
  if (cnt || holes) {
    cr->set_line_cap(Cairo::LINE_CAP_ROUND);
    cr->set_line_join(Cairo::LINE_JOIN_ROUND);
    cr->translate(-draw_range.tlc());
  }

  // draw contours
  if (cnt) {
    double cnt_rdp = 0.2; // todo: move to options?
    auto srtm_lock = srtm->get_lock();
    auto c_data = srtm->find_contours(wgs_range, cnt_step, cnt_vtol);
    cr->set_color(cnt_color);
    for(auto const & c:c_data){
      if (is_stopped()) return false;
      bool isth = int(c.first)%(cnt_step*cnt_smult); // is it a thick contour
      cr->set_line_width(cnt_w*(isth? 1:cnt_wmult));
      dMultiLine l = c.second;
      cnv->bck(l);
      line_filter_rdp(l, 0.5); // in pixels
      cr->mkpath_smline(l, 0, cnt_crv);
      cr->stroke();
    }
  }

  // draw holes
  if (holes) {
    auto srtm_lock = srtm->get_lock();
    auto h_data = srtm->find_holes(wgs_range);
    cr->set_color(holes_color);
    cr->set_line_width(holes_w);
    cnv->bck(h_data);
    for(auto const & l:h_data){
      if (is_stopped()) return false;
      cr->mkpath_smline(l,0,0);
    }
    cr->stroke();
  }

  // draw peaks
  if (peaks) {
    auto srtm_lock = srtm->get_lock();
    wgs_range = cnv->frw_acc(expand(draw_range,peaks_text_size*4));
    auto p_data = srtm->find_peaks(wgs_range, peaks_dh, peaks_ps);
    cnv->bck(p_data);
    cr->set_color(peaks_color);
    cr->set_line_width(peaks_w);
    for (const auto & pt:p_data){
      if (is_stopped()) return false;
      cr->move_to(pt);
      cr->line_to(pt);
    }
    cr->stroke();
    if (peaks_text){
      cr->set_fc_font(peaks_color, peaks_text_font.c_str(), peaks_text_size);
      for(const auto & pt:p_data){
        cr->move_to(pt + dPoint(2,2));
        cr->text_path(type_to_str((int)pt.z));
      }
      cr->set_line_width(2);
      cr->set_color(0xFFFFFFFF);
      cr->stroke_preserve();
      cr->set_color(peaks_color);
      cr->fill();
    }
  }

  tiles.add(draw_range, image);
  return true;
}

GObj::ret_t
GObjSRTM::draw(const CairoWrapper & cr, const dRect & draw_range) {

  if (!srtm) return GObj::FILL_NONE;
  if (is_stopped()) return GObj::FILL_NONE;

  if (!tiles.contains(draw_range)){
    if (!render_tile(draw_range)) return GObj::FILL_NONE;
  }
  cr->set_source(image_to_surface(tiles.get(draw_range)),
                 draw_range.x, draw_range.y);
  cr->paint();

  if (is_stopped()) return GObj::FILL_NONE;
  return surf? GObj::FILL_ALL : GObj::FILL_PART;
}



