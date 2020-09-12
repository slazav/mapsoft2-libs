#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#include <vector>
#include <memory>
#include "err/err.h"
#include "geom/rect.h"

#include "colors.h"

/*
Image interface: get ARGB color for x,y coordinates.
*/

/*********************************************************************/
class Image {
  protected:
    uint32_t bgcolor;

  public:

    // constructor
    Image(uint32_t bgcolor = 0xFF000000): bgcolor(bgcolor) {}

    // set background color
    void set_bgcolor(const uint32_t c) { bgcolor = c; }

    // get background color
    uint32_t get_bgcolor() const { return bgcolor; }

    // Check if coordinates are valid
    virtual bool check_crd(const size_t x, const size_t y) const {return true;}

    // Check if coordinate range is valid.
    // x1<=x2, y1<=y2
    virtual bool check_rng(const size_t x1, const size_t y1,
                           const size_t x2, const size_t y2) const {return true;}

    // Get color for a given point. To be redefined.
    // This is fast version, without range checks. Should be
    // used only after check_crd/check_rng checks.
    virtual uint32_t get_color_fast(const size_t x, const size_t y) {
      return bgcolor; }

    // Get color value for a dPoint<double>
    // Should be used only after check_crd/check_rng checks.
    uint32_t get_color_fast(const dPoint & p) {
      return get_color_fast(rint(p.x), rint(p.y)); }


    // Get color for a given point. Should work for any
    // coordinates, returns bgcolor if check_crd() fails.
    uint32_t get_color(const size_t x, const size_t y) {
      return check_crd(x,y) ? get_color_fast(x,y) : bgcolor;}

    // Get color value for a dPoint<double>. Should work for any
    // coordinates, returns bgcolor if check_crd() fails.
    uint32_t get_color(const dPoint & p) {
      int x = rint(p.x), y = rint(p.y);
      return check_crd(x,y) ? get_color_fast(x,y) : bgcolor;
    }

    // Get color value using 4-point interpolation.
    virtual uint32_t get_color_int4(const dPoint & p){
      int x1 = floor(p.x), x2 = x1+1;
      int y1 = floor(p.y), y2 = y1+1;
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      uint32_t v1 = get_color_fast(x1,y1);
      uint32_t v2 = get_color_fast(x1,y2);
      uint32_t v3 = get_color_fast(x2,y1);
      uint32_t v4 = get_color_fast(x2,y2);
      uint32_t v0 = 0;
      for (int sh = 0; sh<32; sh+=8){
        double c1 = (v1>>sh) & 0xff;
        double c2 = (v2>>sh) & 0xff;
        double c3 = (v3>>sh) & 0xff;
        double c4 = (v4>>sh) & 0xff;
        double c12 = c1+(c2-c1)*(p.y-y1);
        double c34 = c3+(c4-c3)*(p.y-y1);
        double c0 = c12+(c34-c12)*(p.x-x1);
        v0 += ((int)c0 & 0xff) << sh;
      }
      return v0;
    }

    // Get averaged color value in radius `rad`.
    uint32_t get_color_avrg(const dPoint & p, double rad=2.0){
      int x1 = floor(p.x-rad), x2 = ceil(p.x+rad);
      int y1 = floor(p.y-rad), y2 = ceil(p.y+rad);
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      double sc[4] = {0,0,0,0};
      double s0[4] = {0,0,0,0};
      if (x1==x2 || y1==y2) return bgcolor;
      for (int y=y1; y<=y2; ++y){
        for (int x=x1; x<=x2; ++x){
          uint32_t v = get_color_fast(x,y);
          double d = dist2d(p, dPoint(x,y));
          double w = exp(-d/rad*2);
          for (int i = 0; i<4; ++i){
            sc[i] += ((v>>(8*i)) & 0xff) * w;
            s0[i] += w;
          }
        }
      }
      uint32_t v0 = 0;
      for (int i = 0; i<4; ++i){
        if (s0[i]>0)
          v0 += ((int)(sc[i]/s0[i]) & 0xff) << (8*i);
      }
      return v0;
    }

    virtual std::ostream & print (std::ostream & s) const{
      s << "Image(0x" << std::hex << bgcolor << ")";
      return s;
    }

};

std::ostream & operator<< (std::ostream & s, const Image & i);

#endif
