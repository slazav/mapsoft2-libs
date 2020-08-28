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

class GObjPano : public SRTM, public GObj {
private:

  std::shared_ptr<ConvBase> cnv; // used only for rescaling

  dPoint p0;
  double max_r;
  double dh;
  Rainbow R;

  static const int width0 = 4096;
  int width;

  struct ray_data{
    double r, h, s; // distance, height, slope
    ray_data(double r_, double h_, double s_):r(r_),h(h_),s(s_){}
  };

  Cache<int, std::vector<ray_data> > ray_cache;
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

/*
  // altitude above terrain
  void set_alt(double h) { dh=h;}
  double get_alt(void) const { return dh;}

  // rainbow limits
  void set_colors(double min, double max) {R.set(min,max);}
  double get_minh(void) const {return R.get_v1();}
  double get_maxh(void) const {return R.get_v2();}

  // max distance
  void set_maxr(double r) {max_r=r; ray_cache.clear();}
  double get_maxr(void) const {return max_r;}

  void get_width() { width=w; }
*/

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


  GObjPano(const Opt & o): ray_cache(512), width(width0) { set_opt(o); }

  /************************************************/

  bool get_xloop() const override {return true;}

  static Opt get_def_opt();

  void set_opt(const Opt & o) override;

  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  int draw(const CairoWrapper & cr, const dRect &box) override;

};

#endif
