#include "gobj_srtm.h"
#include <sstream>
#include <fstream>

void
GObjSRTM::set_opt(const Opt & o){
  SRTM::set_opt(o);

  // srtm drawing mode:
  auto m = o.get("srtm_draw_mode", "shades");
  if      (m == "none"){
    draw_mode = SRTM_DRAW_NONE;
  }
  else if (m == "heights") {
    draw_mode = SRTM_DRAW_HEIGHTS;
    R = Rainbow(
      o.get("srtm_hmin", 0.0),
      o.get("srtm_hmax", 5000.0),
      RAINBOW_NORMAL);
  }
  else if (m == "shades") {
    draw_mode = SRTM_DRAW_SHADES;
    R = Rainbow(
      o.get("srtm_hmin", 0.0),
      o.get("srtm_hmax", 5000.0),
      RAINBOW_NORMAL);
  }
  else if (m == "slopes"){
    draw_mode = SRTM_DRAW_SLOPES;
    R = Rainbow(
      o.get("srtm_smin", 30.0),
      o.get("srtm_smax", 55.0),
      RAINBOW_BURNING);
  }
  else throw Err() << "unknown value of srtm_draw_mode parameter "
    << "(none, heights, or slopes expected): " << m;

  interp_holes = o.get("srtm_interp_holes", 1);
  bgcolor = o.get<int>("srtm_bgcolor", 0);
}

int
GObjSRTM::draw(const CairoWrapper & cr, const dRect & draw_range) {

  if (draw_mode != SRTM_DRAW_NONE) {

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

  if (is_stopped()) return GObj::FILL_NONE;
  return GObj::FILL_ALL;
}

