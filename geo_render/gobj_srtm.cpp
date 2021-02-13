#include "gobj_srtm.h"
#include <sstream>
#include <fstream>

void
ms2opt_add_drawsrtm(GetOptSet & opts){

  ms2opt_add_srtm(opts);

  const char *g = "DRAWSRTM";
  opts.add("srtm_surf", 1,0,g,
    "draw SRTM color surface (default 1).");
  opts.add("srtm_maxsc", 1,0,g,
    "Do not draw srtm data out of this scale (default 15).");
  opts.add("srtm_maxscv", 1,0,g,
    "Do not draw srtm contours out of this scale (default 1.0).");
  opts.add("srtm_cnt",1,0,g,
    "Draw contours (0|1, default - 1).");
  opts.add("srtm_cnt_step",1,0,g,
    " Contour step [m], default 50.");
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

  Opt o =SRTM::get_def_opt();

  o.put("srtm_surf",    1);
  o.put("srtm_maxsc",   15.0);
  o.put("srtm_maxscv",  1.0);

  // contours parameters
  o.put("srtm_cnt",       1);
  o.put("srtm_cnt_step",  50);
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

  maxsc    = o.get<double>("srtm_maxsc",   15);
  maxscv   = o.get<double>("srtm_maxscv",  1);

  // contours parameters
  cnt          = o.get<bool>("srtm_cnt",      1);
  cnt_step     = o.get<int>("srtm_cnt_step",  50);
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
  redraw_me();
}

void
GObjSRTM::set_cnv(const std::shared_ptr<ConvBase> c) {
  cnv = c;
  redraw_me();
}

GObj::ret_t
GObjSRTM::draw(const CairoWrapper & cr, const dRect & draw_range) {

  if (!srtm) return GObj::FILL_NONE;

  dPoint scp = cnv->scales(draw_range);
  double sc = std::min(scp.x, scp.y) * srtm->get_srtm_width();

  if (surf) {
    if (sc < maxsc) {
      ImageR image(draw_range.w, draw_range.h, IMAGE_32ARGB);

      auto srtm_lock = srtm->get_lock();
      for (size_t j=0; j<image.height(); j++){
        if (is_stopped()) return GObj::FILL_NONE;
        for (size_t i=0; i<image.width(); i++){
          dPoint p(i + draw_range.x, j+draw_range.y);
          if (cnv) cnv->frw(p);
          image.set32(i,j, srtm->get_color(p));
       }
      }
      cr->set_source(image_to_surface(image), draw_range.x, draw_range.y);
      cr->paint();
    }
    else {
      cr->set_color(srtm->get_bgcolor());
      cr->paint();
    }
  }

  dRect wgs_range;
  if ((cnt || holes || peaks) && sc < maxscv) {
    wgs_range = cnv->frw_acc(draw_range);
    wgs_range.expand(1.0/srtm->get_srtm_width()); // +1 srtm point
    cr->set_line_cap(Cairo::LINE_CAP_ROUND);
    cr->set_line_join(Cairo::LINE_JOIN_ROUND);
  }

  // draw contours
  if (cnt && sc < maxscv) {
    auto srtm_lock = srtm->get_lock();
    auto c_data = srtm->find_contours(wgs_range, cnt_step);
    cr->set_color(cnt_color);
    for(auto const & c:c_data){
      if (is_stopped()) return GObj::FILL_NONE;
      bool isth = c.first%(cnt_step*cnt_smult); // is it a thick contour
      cr->set_line_width(cnt_w*(isth? 1:cnt_wmult));
      dMultiLine l = c.second;
      cnv->bck(l);
      cr->mkpath_smline(l, 0, cnt_crv);
      cr->stroke();
    }
  }

  // draw holes
  if (holes && sc < maxscv) {
    auto srtm_lock = srtm->get_lock();
    auto h_data = srtm->find_holes(wgs_range);
    cr->set_color(holes_color);
    cr->set_line_width(holes_w);
    cnv->bck(h_data);
    for(auto const & l:h_data){
      if (is_stopped()) return GObj::FILL_NONE;
      cr->mkpath_smline(l,0,0);
    }
    cr->stroke();
  }

  // draw peaks
  if (peaks && sc < maxscv) {
    auto srtm_lock = srtm->get_lock();
    auto p_data = srtm->find_peaks(wgs_range, peaks_dh, peaks_ps);
    cr->set_color(peaks_color);
    cr->set_line_width(peaks_w);
    for (auto & d:p_data){
      if (is_stopped()) return GObj::FILL_NONE;
      dPoint p0 = d.first;
      cnv->bck(p0);
      cr->move_to(p0);
      cr->line_to(p0);
    }
    cr->stroke();
    if (peaks_text){
      cr->set_fc_font(peaks_color, peaks_text_font.c_str(), peaks_text_size);
      for(auto & d:p_data){
        dPoint p0 = d.first;
        cnv->bck(p0);
        cr->move_to(p0 + dPoint(2,2));
        cr->text_path(type_to_str(d.second));
      }
      cr->set_line_width(2);
      cr->set_color(0xFFFFFFFF);
      cr->stroke_preserve();
      cr->set_color(peaks_color);
      cr->fill();
    }
  }

  if (is_stopped()) return GObj::FILL_NONE;
  return GObj::FILL_ALL;
}



