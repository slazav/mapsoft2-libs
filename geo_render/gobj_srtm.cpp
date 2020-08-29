#include "gobj_srtm.h"
#include <sstream>
#include <fstream>

void
ms2opt_add_drawsrtm(GetOptSet & opts){

  ms2opt_add_srtm(opts);

  const char *g = "DRAWSRTM";
  opts.add("srtm_draw_mode", 1,0,g,
    "SRTM drawing mode (none, slopes, heights, shades, default - shades).");
  opts.add("srtm_hmin", 1,0,g,
    "Min height [m] for heights and shades modes (default - 0).");
  opts.add("srtm_hmax", 1,0,g,
    "Max height [m] for heights and shades modes (default - 5000).");
  opts.add("srtm_smin", 1,0,g,
    "Min slope [deg] for slopes mode (default - 35).");
  opts.add("srtm_smax", 1,0,g,
    "Max slope [deg] for slopes mode (default - 50).");
  opts.add("srtm_interp_holes", 1,0,g,
    "Interpolate holes (0|1, default 1).");
  opts.add("srtm_bgcolor", 1,0,g,
    "Color to draw no-data and out-of-scale areas (default 0x60FF0000).");
  opts.add("srtm_maxsc", 1,0,g,
    "Do not draw srtm data out of this scale (default 15).");
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
    " Draw summits (0|1, default - 1).");
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
  o.put("srtm_draw_mode", "shades");
  o.put("srtm_hmin", 0);
  o.put("srtm_hmax", 5000);
  o.put("srtm_smin", 35);
  o.put("srtm_smax", 50);
  o.put("srtm_interp_holes", 1);
  o.put("srtm_bgcolor", 0x60FF0000);
  o.put("srtm_maxsc",   15);
  o.put("srtm_maxscv",  0.5);

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
  o.put("srtm_peaks",      1);
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
  if (srtm) srtm->set_opt(o);

  // srtm drawing mode:
  auto m = o.get("srtm_draw_mode", "shades");
  if      (m == "none")    { draw_mode = SRTM_DRAW_NONE; }
  else if (m == "heights") { draw_mode = SRTM_DRAW_HEIGHTS; }
  else if (m == "shades")  { draw_mode = SRTM_DRAW_SHADES; }
  else if (m == "slopes")  { draw_mode = SRTM_DRAW_SLOPES; }
  else throw Err() << "unknown value of srtm_draw_mode parameter "
    << "(none, heights, or slopes expected): " << m;

  // limits
  hmin = o.get("srtm_hmin", 0.0);
  hmax = o.get("srtm_hmax", 5000.0);
  smin = o.get("srtm_smin", 35.0);
  smax = o.get("srtm_smax", 50.0);

  // set rainbow converters:
  if (draw_mode == SRTM_DRAW_HEIGHTS ||
      draw_mode == SRTM_DRAW_SHADES)
    R = Rainbow(hmin,hmax, RAINBOW_NORMAL);
  else if (draw_mode == SRTM_DRAW_SLOPES)
    R = Rainbow(smin,smax, RAINBOW_BURNING);

  interp_holes = o.get("srtm_interp_holes", 1);
  bgcolor = o.get<int>("srtm_bgcolor", 0x60FF0000);
  maxsc    = o.get<double>("srtm_maxsc",   15);
  maxscv   = o.get<double>("srtm_maxscv",  0.5);

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
  peaks        = o.get<bool>("srtm_peaks",      1);
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

int
GObjSRTM::draw(const CairoWrapper & cr, const dRect & draw_range) {

  if (!srtm) return GObj::FILL_NONE;

  dPoint scp = cnv->scales(draw_range);
  double sc = std::min(scp.x, scp.y) * srtm->get_srtm_width();

  if (draw_mode != SRTM_DRAW_NONE) {
    if (sc < maxsc) {
      ImageR image(draw_range.w, draw_range.h, IMAGE_32ARGB);
      for (int j=0; j<image.height(); j++){
        if (is_stopped()) return GObj::FILL_NONE;
        for (int i=0; i<image.width(); i++){
          dPoint p(i + draw_range.x, j+draw_range.y);
          if (cnv) cnv->frw(p);
          short h = srtm->get_val_int4(p,  interp_holes);
          if (h < SRTM_VAL_MIN){
            image.set32(i,j, bgcolor);
            continue;
          }
          double s = srtm->get_slope_int4(p, interp_holes);
          uint32_t c;

          switch (draw_mode){
            case SRTM_DRAW_NONE: break;
            case SRTM_DRAW_SLOPES:  c = R.get(s); break;
            case SRTM_DRAW_HEIGHTS: c = R.get(h); break;
            case SRTM_DRAW_SHADES:  c = color_shade(R.get(h), 1-s/90.0); break;
          }
          image.set32(i,j, c);
       }
      }
      cr->set_source(image_to_surface(image), draw_range.x, draw_range.y);
      cr->paint();
    }
    else {
      cr->set_color_a(bgcolor);
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
        cr->show_text(type_to_str(d.second));
      }
    }
  }

  if (is_stopped()) return GObj::FILL_NONE;
  return GObj::FILL_ALL;
}



