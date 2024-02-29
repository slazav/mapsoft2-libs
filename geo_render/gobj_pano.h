#ifndef GOBJ_PANO_H
#define GOBJ_PANO_H

#include "rainbow/rainbow.h"
#include "viewer/gobj.h"
#include "srtm/srtm.h"
#include <vector>
#include <mutex>


/********************************************************************/
#include "getopt/getopt.h"

// add DRAWPANO group of options
void ms2opt_add_drawpano(GetOptSet & opts);


/********************************************************************/

class GObjPano : public GObj {
private:

  std::shared_ptr<ConvBase> cnv; // used only for rescaling
  SRTM *srtm;

  dPoint p0;
  double max_r;
  double dh;
  Rainbow R;

  static const size_t width0 = 4096;
  int width;

  struct ray_data{
    double r, h, s; // distance, height, slope
    ray_data(double r_, double h_, double s_):r(r_),h(h_),s(s_){}
  };

  Cache<size_t, std::vector<ray_data> > ray_cache;
  std::mutex cache_mutex;

public:

/*
  // Horizontal range is 720deg, -width..width
  // Vertical range is 90, 0..width/4
  iRect range() const {return iRect(0,0, width, width/2);}

*/
   // central point, wgs84 lonlat
  void set_origin(const dPoint & p) {p0=p; ray_cache.clear(); redraw_me();}
  dPoint get_origin(void) const {return p0;}

  // 360deg width
  int get_width(void) const {return width;}

  // find segments of the ray brocken by srtm grid
  // these segments must have linear height and slope dependence
  std::vector<ray_data> get_ray(int x);

  public:

  // Convert geo coordinates to the image point.
  // y coordinate is negative for invisible points.
  iPoint geo2xy(const dPoint & pt);

  // Convert image point to geo coordinates.
  dPoint xy2geo(const iPoint & pt);


  GObjPano(SRTM * s, const Opt & o): srtm(s),
      width(width0), ray_cache(512) { set_opt(o); }

  static Opt get_def_opt();

  void set_opt(const Opt & o) override;

  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  ret_t draw(const CairoWrapper & cr, const dRect &box) override;

};

#endif
