#include "gobj_pano.h"

/***********************************************************/
// options

void
ms2opt_add_drawpano(GetOptSet & opts){
  ms2opt_add_srtm(opts);
  const char *g = "DRAWSRTM";
  opts.add("pano_origin",1,0,g,  "Origin coordinates, default [0,0].");
  opts.add("pano_alt",1,0,g, "Altitude of the viewpoint above terrain, meters, default 20.");
  opts.add("pano_rmax",1,0,g, "Max distance, km, default 100.");
}

Opt
GObjPano::get_def_opt() {
  Opt o;
  o.put(SRTMSurf::get_def_opt());
  o.put("pano_origin",  dPoint());
  o.put("pano_alt",     20.0);
  o.put("pano_rmax",   100.0);
  return o;
}

void
GObjPano::set_opt(const Opt & o){
  SRTMSurf::set_opt(o);
  p0 = o.get<dPoint>("pano_origin");
  dh = o.get<double>("pano_alt", 20.0);
  max_r = o.get<double>("pano_rmax", 100) * 1000; // convert km->m
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


std::vector<GObjPano::ray_data>
GObjPano::get_ray(int x){

  if (!srtm) return std::vector<GObjPano::ray_data>();

  // convert coordinate to angle, [0..2pi]
  while (x<0) x+=width;
  while (x>=width) x-=width;
  double a = 2.0*M_PI*x/width;
  size_t key=round(a*1e6); // key for the cache

  if (ray_cache.contains(key)){
     std::lock_guard<std::mutex> lk(cache_mutex);
     return ray_cache.get(key);
  }

  size_t srtmw = srtm->get_srtm_width()-1;

  double sa=sin(a), ca=cos(a), cx=cos(p0.y * M_PI/180.0);
  dPoint pt = p0*srtmw; // viewpoint coordinates in srtm points
  double m2srtm = srtmw * 180/M_PI/ 6380000;
  double rM = max_r*m2srtm; // max distance in srtm points

  // Intersections or ray with x and y lines of srtm grid goes
  // with constant steps. But we need to sort them by r.
  double rx,drx,ry,dry; // Initial values and steps
  if      (sa>1/rM) { drx=cx/sa;  rx=(ceil(pt.x)-pt.x)*cx/sa;  }
  else if (sa<-1/rM){ drx=-cx/sa; rx=(floor(pt.x)-pt.x)*cx/sa; }
  else              { drx=rx=rM; }
  if      (ca>1/rM) { dry=1/ca;  ry=(ceil(pt.y)-pt.y)/ca;  }
  else if (ca<-1/rM){ dry=-1/ca; ry=(floor(pt.y)-pt.y)/ca; }
  else                 { dry=ry=rM; }

  std::vector<GObjPano::ray_data> ret;
  auto srtm_lock = srtm->get_lock();
  while (rx<rM || ry<rM){ // Go from zero to rM

    while (rx <= ry && rx<rM){ // step in x
      int x = round(pt.x+rx*sa/cx); // x is a round number
      double y = pt.y+rx*ca;        // y is between two grid points.
      // assert(abs(pt.x+rx*sa/cx - x) < 1e-9); // check this

      // Interpolate altitude and slope between two points:
      int y1 = floor(y);
      int y2 = ceil(y);
      double h=SRTM_VAL_UNDEF,s;
      if (y1!=y2){
        int h1 = srtm->get_val(x,y1, false);
        int h2 = srtm->get_val(x,y2, false);
        double s1 = srtm->get_slope(x,y1, false);
        double s2 = srtm->get_slope(x,y2, false);
        if ((h1>SRTM_VAL_MIN) && (h2>SRTM_VAL_MIN))
          h = h1 + (h2-h1)*(y-y1)/(y2-y1);
        s = s1 + (s2-s1)*(y-y1)/(y2-y1);
      }
      else {
        h = srtm->get_val(x,y1, false);
        s = srtm->get_slope(x,y1, false);
      }
      // Save data and do step:
      if  (h>SRTM_VAL_MIN) ret.emplace_back(rx/m2srtm, h, s);
      rx+=drx;
    }
    while (ry <= rx && ry<rM){ // step in y; the same but rx->ry, y is on the grid
      double x = pt.x+ry*sa/cx;
      int y = round(pt.y+ry*ca);
      // assert(abs(pt.y+ry*ca - y) < 1e-9);

      int x1 = floor(x);
      int x2 = ceil(x);
      double h=SRTM_VAL_UNDEF,s;
      if (x1!=x2){
        int h1 = srtm->get_val(x1,y, false);
        int h2 = srtm->get_val(x2,y, false);
        double s1 = srtm->get_slope(x1,y, false);
        double s2 = srtm->get_slope(x2,y, false);
        if ((h1>SRTM_VAL_MIN) && (h2>SRTM_VAL_MIN))
          h = h1 + (h2-h1)*(x-x1)/(x2-x1);
        s = s1 + (s2-s1)*(x-x1)/(x2-x1);
      }
      else {
        h = srtm->get_val(x1,y, false);
        s = srtm->get_slope(x1,y, false);
      }
      if  (h>SRTM_VAL_MIN) ret.push_back(ray_data(ry/m2srtm, h, s));
      ry+=dry;
    }
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
    h0 = srtm->get_val_int4(p0) + dh; // altitude of observation point
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
      if (r>max_r) break;

      double b = atan2(hn-h0, r); // vertical angle
      ssize_t yn = (1 - 2*b/M_PI) * width/4.0 - box.y; // y-coord

      if (yn<0)  {i=ray.size();}     // above image -- draw the rest of segment and exit
      if (yn>=yo) {yp=yn; continue;} // point below image -- skip segment

      for (ssize_t y = yn; y < yp; y++){
        if (y<0 || y>=yo) continue; // select visible points
        double s = sp + (sn-sp)*(y-yp)/double(yn-yp); // Interpolate slope and altitude
        double h = hp + (hn-hp)*(y-yp)/double(yn-yp); //  and calculate point color.
        uint32_t color = color_shade(SRTMSurf::get_color(h,s), (1-r/max_r));
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


