#include "gobj_pano.h"
#include "geo_data/geo_utils.h"

/***********************************************************/
// options

void
ms2opt_add_drawpano(GetOptSet & opts){
  ms2opt_add_srtm_surf(opts);
  const char *g = "DRAWSRTM";
  opts.add("pano_origin",1,0,g,  "Origin coordinates, default [0,0].");
  opts.add("pano_alt",1,0,g, "Altitude of the viewpoint above terrain, meters, default 20.");
  opts.add("pano_rmax",1,0,g, "Max distance, km, default 100.");
}

Opt
GObjPano::get_def_opt() {
  Opt o = SRTM::get_def_opt();
  o.put("pano_origin",  dPoint());
  o.put("pano_alt",     20.0);
  o.put("pano_rmax",   100.0);
  return o;
}

void
GObjPano::set_opt(const Opt & o){
  if (srtm){
    auto srtm_lock = srtm->get_lock();
    srtm->set_opt(o);
  }
  p0 = o.get<dPoint>("pano_origin");
  dh = o.get<double>("pano_alt", 20.0);
  mr = o.get<double>("pano_rmax", 100) * 1000; // convert km->m
  std::lock_guard<std::mutex> lk(cache_mutex);
  ray_cache.clear();
  redraw_me();
}

/***********************************************************/

// set_cnv is only used for rescaling.
void
GObjPano::set_cnv(const std::shared_ptr<ConvBase> c) {
  width = width0/c->get_scale_src().x;
  std::lock_guard<std::mutex> lk(cache_mutex);
  ray_cache.clear();
  redraw_me();
}

/***********************************************************/

void
GObjPano::set_p0(const dPoint & p0_) {
  p0=p0_;

  dPoint d;
  {
    auto srtm_lock = srtm->get_lock();
    // data step in degrees in the central point
    st = srtm->get_step(p0);
  }

  // convert to m
  st *= M_PI*6380e3/180.0;
  st.x *= cos(p0.y * M_PI/180.0);

  {
    std::lock_guard<std::mutex> lk(cache_mutex);
    ray_cache.clear();
  }
  redraw_me();
}

void
GObjPano::set_mr(const double mr_) {
  mr=mr_;
  {
    std::lock_guard<std::mutex> lk(cache_mutex);
    ray_cache.clear();
  }
  redraw_me();
}

void
GObjPano::set_dh(const double dh_) {
  dh=dh_;
  redraw_me();
}



std::vector<GObjPano::ray_data>
GObjPano::get_ray(int x){

  if (!srtm) return std::vector<GObjPano::ray_data>();

  // convert coordinate to angle, [0..2pi]
  while (x<0) x+=width;
  while (x>=width) x-=width;
  double da = 2.0*M_PI/width; // pixel size in rad
  double a = x*da;            // angle in rad
  double ad = 360.0*x/width;  // andle in deg
  size_t key=round(a*1e6);    // key for the cache

  if (ray_cache.contains(key)){
     std::lock_guard<std::mutex> lk(cache_mutex);
     return ray_cache.get(key);
  }

  // step along the ray (angle is counted from axis y), m:
  auto dr = 1.0/sqrt(pow(sin(a)/st.x, 2) + pow(cos(a)/st.y, 2));

  std::vector<GObjPano::ray_data> ret;
  auto srtm_lock = srtm->get_lock();
  for (double r = 0; r<mr; r+=dr){
    bool raw = dr/r < da;
    auto p2 = geo_bearing_2d(p0, ad, r);
    auto h = srtm->get_h(p2, raw);
    auto s = srtm->get_s(p2, raw);
    if (h>SRTM_VAL_MIN) ret.emplace_back(r, h, s);
  }
  std::lock_guard<std::mutex> lk(cache_mutex);
  ray_cache.add(key,ret);
  return ret;
}

/***********************************************************/

iPoint
GObjPano::geo2xy(const dPoint & pt){

  iPoint ret;
  double cx=cos(p0.y*M_PI/180);
  ret.x = width * atan2((pt.x-p0.x)*cx, pt.y-p0.y)/2.0/M_PI;

  double r0 = hypot((pt.x-p0.x)*cx, pt.y-p0.y)*6380000/180.0*M_PI;

  auto ray = get_ray(ret.x);
  if (!ray.size()) return iPoint();

  int yp, yo;
  yo=yp=width/2.0;
  double h0 = ray[0].h+dh;
  double rp = 0;

  for (size_t i=0; i<ray.size(); i++){
    double hn=ray[i].h;
    double rn=ray[i].r;

    double b = atan2(hn-h0, rn); // vertical angle
    int yn = (1 - 2*b/M_PI) * width/4.0; // y-coord

    if (rn>r0){
      ret.y = yp + (yn-yp)*(rn-r0)/(rn-rp);
      if (ret.y > yo) ret.y = -ret.y; //invisible
      return ret;
    }
    if (yn<yo) yo=yn;
    yp=yn;
    rp=rn;
  }

  ret.y = 0;
  return ret;
}


dPoint
GObjPano::xy2geo(const iPoint & pt){

  auto ray = get_ray(pt.x);
  if (!ray.size()) return dPoint(0.0,180.0);

  double h0 = ray[0].h+dh;
  int yp, yo;
  yo=yp=width/2.0;
  double rp = 0;
  for (size_t i=0; i<ray.size(); i++){
    double hn=ray[i].h;
    double rn=ray[i].r;

    double b = atan2(hn-h0,rn); // vertical angle
    int yn = (1 - 2*b/M_PI) * width/4.0; // y-coord

    if (yn<pt.y){
      double r = rp + (rn-rp)*(pt.y-yn)/double(yp-yn);
      double a = pt.x/double(width)*2*M_PI;
      double cx=cos(p0.y*M_PI/180.0);
      return p0 + dPoint(sin(a)/cx, cos(a))*r / 6380000.0 / M_PI * 180.0;
    }

    if (yn<yo) yo=yn;
    yp=yn;
    rp=rn;
  }
  return dPoint(0,180);
}

/***********************************************************/

GObj::ret_t
GObjPano::draw(const CairoWrapper & cr, const dRect &box){
  if (is_stopped()) return GObj::FILL_NONE;
  if (!srtm) return GObj::FILL_NONE;

  double h0;
  {
    auto srtm_lock = srtm->get_lock();
    h0 = srtm->get_h(p0) + dh; // altitude of observation point
  }
  ImageR image(box.w, box.h, IMAGE_32ARGB);
  image.fill32(0xFF000000);

  for (size_t x=0; x < image.width(); x++){
    if (is_stopped()) return GObj::FILL_NONE;

    // get ray data -- r,h,s values for a giver x-coord
    auto ray = get_ray(x+box.x);
    if (!ray.size()) continue;

    ssize_t yo = image.height(); // Old y-coord, previously painted point.
                             // It is used to skip hidden parts.
    ssize_t yp = width/2.0-box.y;// Previous value, differs from yo on hidden 
                             // and partially hydden segments.
                             // It is used to interpolate height and slope.
                             // Y axis goes from top to buttom!
    for (size_t i=1; i<ray.size(); i++){
      double hp=ray[i-1].h;  // altitudes and slopes at the end of segment
      double sp=ray[i-1].s;
      double hn=ray[i].h;
      double sn=ray[i].s;
      double r=ray[i].r;
      if (r>mr) break;

      double b = atan2(hn-h0, r); // vertical angle
      ssize_t yn = (1 - 2*b/M_PI) * width/4.0 - box.y; // y-coord

      if (yn<0)  {i=ray.size();}     // above image -- draw the rest of segment and exit
      if (yn>=yo) {yp=yn; continue;} // point below image -- skip segment

      for (ssize_t y = yn; y < yp; y++){
        if (y<0 || y>=yo) continue; // select visible points
        double s = sp + (sn-sp)*(y-yp)/double(yn-yp); // Interpolate slope and altitude
        double h = hp + (hn-hp)*(y-yp)/double(yn-yp); //  and calculate point color.
        uint32_t color = color_shade(srtm->get_color(h,s), (1-r/mr));
        image.set32(x,y, color);
      }
      if (yn<yo) yo=yn;
      yp=yn;
    }

    for (ssize_t y = 0; y < yo; y++) // draw sky points
      image.set32(x,y,0xFFBBBBFF);
  }

  cr->set_source(image_to_surface(image), box.x, box.y);
  cr->paint();

  return GObj::FILL_ALL;
}

