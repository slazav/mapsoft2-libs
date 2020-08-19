#include "gobj_srtm.h"
#include <sstream>
#include <fstream>

void
GObjSRTM::set_opt(const Opt & o){
  SRTM::set_opt(o);

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
  cnt_step1    = o.get<int>("srtm_cnt_step",  10);
  cnt_step2    = o.get<int>("srtm_cnt_step2", cnt_step1*5);
  cnt_color    = o.get<int>("srtm_cnt_col",   0xFF000000);
  cnt_th       = o.get<double>("srtm_cnt_th",   1);
  cnt_th2      = o.get<double>("srtm_cnt_th2",  cnt_th*2);
  cnt_crv      = o.get<double>("srtm_cnt_crv",  20.0);

  // holes parameters
  holes        = o.get<bool>("srtm_holes",      1);
  holes_color  = o.get<int>("srtm_holes_col",  0xFFFF0000);
  holes_th     = o.get<double>("srtm_holes_th",  1);

  // peaks parameters
  peaks        = o.get<bool>("srtm_peaks",      1);
  peaks_color  = o.get<int>("srtm_peaks_col",  0xFFFFFF00);
  peaks_th     = o.get<double>("srtm_peaks_th",  3);
  peaks_dh     = o.get<int>("srtm_peaks_dh",   20);
  peaks_ps     = o.get<int>("srtm_peaks_ps",  1000);
  peaks_text   = o.get<bool>("srtm_peaks_text",  1);
  peaks_text_size   = o.get<double>("srtm_peaks_text_size",  10);
  peaks_text_font   = o.get("srtm_peaks_text_font",  "serif");
}

Opt
GObjSRTM::get_opt() const{
  Opt o = SRTM::get_opt();

  // srtm drawing mode:
  switch (draw_mode) {
    case SRTM_DRAW_NONE:    o.put("srtm_draw_mode", "none");    break;
    case SRTM_DRAW_HEIGHTS: o.put("srtm_draw_mode", "heights"); break;
    case SRTM_DRAW_SHADES:  o.put("srtm_draw_mode", "shades");  break;
    case SRTM_DRAW_SLOPES:  o.put("srtm_draw_mode", "slopes");  break;
  }

  // limits
  o.put("srtm_hmin", hmin),
  o.put("srtm_hmax", hmax),
  o.put("srtm_smin", smin),
  o.put("srtm_smax", smax),

  o.put("srtm_interp_holes", interp_holes);
  o.put("srtm_bgcolor", bgcolor);
  o.put("srtm_maxsc",   maxsc);
  o.put("srtm_maxscv",  maxscv);

  // contours parameters
  o.put("srtm_cnt",       cnt);
  o.put("srtm_cnt_step",  cnt_step1);
  o.put("srtm_cnt_step2", cnt_step2);
  o.put("srtm_cnt_col",   cnt_color);
  o.put("srtm_cnt_th",    cnt_th);
  o.put("srtm_cnt_th2",   cnt_th2);
  o.put("srtm_cnt_crv",   cnt_crv);

  // holes parameters
  o.put("srtm_holes",     holes);
  o.put("srtm_holes_col", holes_color);
  o.put("srtm_holes_th",  holes_th);

  // peaks parameters
  o.put("srtm_peaks",      peaks);
  o.put("srtm_peaks_col",  peaks_color);
  o.put("srtm_peaks_th",   peaks_th);
  o.put("srtm_peaks_dh",   peaks_dh);
  o.put("srtm_peaks_ps",   peaks_ps);
  o.put("srtm_peaks_text", peaks_text);
  o.put("srtm_peaks_text_size", peaks_text_size);
  o.put("srtm_peaks_text_font", peaks_text_font);

  return o;
}


int
GObjSRTM::draw(const CairoWrapper & cr, const dRect & draw_range) {

  dPoint scp = cnv->scales(draw_range);
  double sc = std::min(scp.x, scp.y) * get_srtm_width();

  if (draw_mode != SRTM_DRAW_NONE) {
    if (sc < maxsc) {
      ImageR image(draw_range.w, draw_range.h, IMAGE_32ARGB);
      for (int j=0; j<image.height(); j++){
        if (is_stopped()) return GObj::FILL_NONE;
        for (int i=0; i<image.width(); i++){
          dPoint p(i + draw_range.x, j+draw_range.y);
          if (cnv) cnv->frw(p);
          short h = get_val_int4(p,  interp_holes);
          if (h < SRTM_VAL_MIN){
            image.set32(i,j, bgcolor);
            continue;
          }
          double s = get_slope_int4(p, interp_holes);
          uint32_t c;

          switch (draw_mode){
            case SRTM_DRAW_NONE: break;
            case SRTM_DRAW_SLOPES:  c = R.get(s); break;
            case SRTM_DRAW_HEIGHTS: c = R.get(h); break;
            case SRTM_DRAW_SHADES:  c = color_shade(R.get(h), 1-s/90.0); break;
          }
          image.set32(i,j, color_shade(c, 1-s/90.0));
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
    wgs_range.expand(1.0/get_srtm_width()); // +1 srtm point
    cr->set_line_cap(Cairo::LINE_CAP_ROUND);
    cr->set_line_join(Cairo::LINE_JOIN_ROUND);
  }

  // draw contours
  if (cnt && sc < maxscv) {
    auto c_data = find_contours(wgs_range, cnt_step1);
    cr->set_color(cnt_color);
    for(auto const & c:c_data){
      if (is_stopped()) return GObj::FILL_NONE;
      cr->set_line_width( c.first%cnt_step2? cnt_th:cnt_th2 );
      dMultiLine l = c.second;
      cnv->bck(l);
      cr->mkpath_smline(l, 0, cnt_crv);
      cr->stroke();
    }
  }

  // draw holes
  if (holes && sc < maxscv) {
    auto h_data = find_holes(wgs_range);
    cr->set_color(holes_color);
    cr->set_line_width(holes_th);
    cnv->bck(h_data);
    for(auto const & l:h_data){
      if (is_stopped()) return GObj::FILL_NONE;
      cr->mkpath_smline(l,0,0);
    }
    cr->stroke();
  }

  // draw peaks
  if (peaks && sc < maxscv) {
    auto p_data = find_peaks(wgs_range, peaks_dh, peaks_ps);
    cr->set_color(peaks_color);
    cr->set_line_width(peaks_th);
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

