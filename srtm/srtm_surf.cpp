#include "srtm_surf.h"

void
ms2opt_add_srtm_surf(GetOptSet & opts){
  ms2opt_add_srtm(opts);
  const char *g = "DRAWSRTM";
  opts.add("srtm_draw_mode", 1,0,g,
    "SRTM surface drawing mode (slopes, heights, shades, default - shades).");
  opts.add("srtm_hmin", 1,0,g,
    "Min height [m] for heights and shades modes (default - 0).");
  opts.add("srtm_hmax", 1,0,g,
    "Max height [m] for heights and shades modes (default - 5000).");
  opts.add("srtm_smin", 1,0,g,
    "Min slope [deg] for slopes mode (default - 35).");
  opts.add("srtm_smax", 1,0,g,
    "Max slope [deg] for slopes mode (default - 50).");
  opts.add("srtm_bgcolor", 1,0,g,
    "Color to draw no-data and out-of-scale areas (default 0x60FF0000).");
}

uint32_t
SRTMSurf::get_color(const double h, const double s){
  switch (draw_mode){
    case SRTM_DRAW_SLOPES:  return R.get(s);
    case SRTM_DRAW_HEIGHTS: return R.get(h);
    case SRTM_DRAW_SHADES:  return color_shade(R.get(h), 1-s/90.0);
  }
  return bgcolor;
}


/// Get color for a point (lon-lat coords), according with drawing options.
uint32_t
SRTMSurf::get_color(const dPoint & p) {
  switch (draw_mode){
    case SRTM_DRAW_SLOPES:  return R.get(SRTM::get_s(p));
    case SRTM_DRAW_HEIGHTS: return R.get(SRTM::get_h(p));
    case SRTM_DRAW_SHADES:
      return color_shade(R.get(SRTM::get_h(p)), 1-SRTM::get_s(p)/90.0);
  }
  return bgcolor;
}


Opt
SRTMSurf::get_def_opt(){
  Opt o = SRTM::get_def_opt();
  o.put("srtm_draw_mode", "shades");
  o.put("srtm_hmin", 0);
  o.put("srtm_hmax", 5000);
  o.put("srtm_smin", 35);
  o.put("srtm_smax", 50);
  o.put("srtm_bgcolor", 0x60FF0000);
  return o;
}

void
SRTMSurf::set_opt(const Opt & o){
  SRTM::set_opt(o);

  // surface parameters
  auto     m = o.get("srtm_draw_mode", "shades");
  if      (m == "heights") { draw_mode = SRTM_DRAW_HEIGHTS; }
  else if (m == "shades")  { draw_mode = SRTM_DRAW_SHADES; }
  else if (m == "slopes")  { draw_mode = SRTM_DRAW_SLOPES; }
  else throw Err() << "unknown value of srtm_draw_mode parameter "
    << "(heights, shades, or slopes expected): " << m;

  // color limits
  double hmin = o.get("srtm_hmin", 0.0);
  double hmax = o.get("srtm_hmax", 5000.0);
  double smin = o.get("srtm_smin", 35.0);
  double smax = o.get("srtm_smax", 50.0);

  // set rainbow converters:
  if (draw_mode == SRTM_DRAW_HEIGHTS ||
      draw_mode == SRTM_DRAW_SHADES)
    R = Rainbow(hmin,hmax, RAINBOW_NORMAL);
  else if (draw_mode == SRTM_DRAW_SLOPES)
    R = Rainbow(smin,smax, RAINBOW_BURNING);

  bgcolor = o.get<int>("srtm_bgcolor", 0x60FF0000);
}
