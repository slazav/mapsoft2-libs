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

    /******************************************************/
    // check functions

    // Check if coordinates are valid
    virtual bool check_crd(const size_t x, const size_t y) const {return true;}

    // Check if coordinate range is valid.
    // x1<=x2, y1<=y2
    virtual bool check_rng(const size_t x1, const size_t y1,
                           const size_t x2, const size_t y2) const {return true;}

    /******************************************************/
    // get_argb functions

    // Get ARGB color for a given (x,y) coordinate.
    // This is fast version, without range checks.
    // Should be used after check_crd/check_rng checks.
    virtual uint32_t get_argb(const size_t x, const size_t y) const {
      return bgcolor; }

    // Get ARGB color for a given iPoint coordinate.
    // This is fast version, without range checks.
    // Should be used after check_crd/check_rng checks.
    uint32_t get_argb(const iPoint & p) const {
      return get_argb(p.x, p.y); }

    // Check coordinates and get ARGB color.
    // Returns bgcolor if check_crd() fails.
    uint32_t get_argb_safe(const size_t x, const size_t y) const {
      return check_crd(x,y) ? get_argb(x,y) : bgcolor;}

    // Check coordinates and get ARGB color.
    // Returns bgcolor if check_crd() fails.
    uint32_t get_argb_safe(const iPoint & p) const {
      return check_crd(p.x, p.y) ? get_argb(p.x,p.y) : bgcolor; }

    // Check coordinates and get ARGB color using 4-point interpolation.
    virtual uint32_t get_argb_int4(const dPoint & p) const {
      int x1 = floor(p.x), x2 = x1+1;
      int y1 = floor(p.y), y2 = y1+1;
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      uint32_t v1 = get_argb(x1,y1);
      uint32_t v2 = get_argb(x1,y2);
      uint32_t v3 = get_argb(x2,y1);
      uint32_t v4 = get_argb(x2,y2);
      uint32_t v0 = 0;
      for (int sh = 0; sh<32; sh+=8){
        double c1 = (v1>>sh) & 0xff;
        double c2 = (v2>>sh) & 0xff;
        double c3 = (v3>>sh) & 0xff;
        double c4 = (v4>>sh) & 0xff;
        double dy = p.y-y1, dx = p.x-x1;
        double c0 = c1*(1.-dx)*(1.-dy) + c2*(1.-dx)*dy +
                    c3*dx*(1.-dy) + c4*dx*dy;
        v0 += ((int)c0 & 0xff) << sh;
      }
      return v0;
    }

    // Check coordinates and get ARGB averaged color
    // (with Gaussian weight) in radius `rad`.
    uint32_t get_argb_avrg(const dPoint & p, double rad=2.0) const {
      int x1 = floor(p.x-rad), x2 = ceil(p.x+rad);
      int y1 = floor(p.y-rad), y2 = ceil(p.y+rad);
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      double sc[4] = {0,0,0,0};
      double s0[4] = {0,0,0,0};
      if (x1==x2 || y1==y2) return bgcolor;
      for (int y=y1; y<=y2; ++y){
        for (int x=x1; x<=x2; ++x){
          uint32_t v = get_argb(x,y);
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
          v0 += ((uint32_t)(sc[i]/s0[i]) & 0xff) << (8*i);
      }
      return v0;
    }

    /******************************************************/
    // get_aarrggbb functions

    // Get AARRGGBB color (2 bytes per component) for a given (x,y) coordinate.
    // This is fast version, without range checks.
    // Should be used after check_crd/check_rng checks.
    virtual uint64_t get_aarrggbb(const size_t x, const size_t y) const {
      return bgcolor; }

    // Get AARRGGBB color (2 bytes per component) for a given iPoint coordinate.
    // This is fast version, without range checks.
    // Should be used after check_crd/check_rng checks.
    uint64_t get_aarrggbb(const iPoint & p) const {
      return get_aarrggbb(p.x, p.y); }

    // Check coordinates and get AARRGGBB color (2 bytes per component).
    // Returns bgcolor if check_crd() fails.
    uint64_t get_aarrggbb_safe(const size_t x, const size_t y) const {
      return check_crd(x,y) ? get_aarrggbb(x,y) : bgcolor;}

    // Check coordinates and get AARRGGBB color (2 bytes per component).
    // Returns bgcolor if check_crd() fails.
    uint64_t get_aarrggbb_safe(const iPoint & p) const {
      return check_crd(p.x, p.y) ? get_aarrggbb(p.x,p.y) : bgcolor; }

    // Check coordinates and get AARRGGBB color (2 bytes per component)
    //  using 4-point interpolation.
    virtual uint64_t get_aarrggbb_int4(const dPoint & p) const {
      int x1 = floor(p.x), x2 = x1+1;
      int y1 = floor(p.y), y2 = y1+1;
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      uint64_t v1 = get_aarrggbb(x1,y1);
      uint64_t v2 = get_aarrggbb(x1,y2);
      uint64_t v3 = get_aarrggbb(x2,y1);
      uint64_t v4 = get_aarrggbb(x2,y2);
      uint64_t v0 = 0;
      for (int sh = 0; sh<64; sh+=16){
        double c1 = (v1>>sh) & 0xffff;
        double c2 = (v2>>sh) & 0xffff;
        double c3 = (v3>>sh) & 0xffff;
        double c4 = (v4>>sh) & 0xffff;
        double dy = p.y-y1, dx = p.x-x1;
        double c0 = c1*(1.-dx)*(1.-dy) + c2*(1.-dx)*dy +
                    c3*dx*(1.-dy) + c4*dx*dy;
        v0 += ((int)c0 & 0xffff) << sh;
      }
      return v0;
    }

    // Check coordinates and get AARRGGBB averaged color
    // (with Gaussian weight) in radius `rad`.
    uint64_t get_aarrggbb_avrg(const dPoint & p, double rad=2.0) const {
      int x1 = floor(p.x-rad), x2 = ceil(p.x+rad);
      int y1 = floor(p.y-rad), y2 = ceil(p.y+rad);
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      double sc[4] = {0,0,0,0};
      double s0[4] = {0,0,0,0};
      if (x1==x2 || y1==y2) return bgcolor;
      for (int y=y1; y<=y2; ++y){
        for (int x=x1; x<=x2; ++x){
          uint64_t v = get_aarrggbb(x,y);
          double d = dist2d(p, dPoint(x,y));
          double w = exp(-d/rad*2);
          for (int i = 0; i<4; ++i){
            sc[i] += ((v>>(16*i)) & 0xffff) * w;
            s0[i] += w;
          }
        }
      }
      uint64_t v0 = 0;
      for (int i = 0; i<4; ++i){
        if (s0[i]>0)
          v0 += ((uint64_t)(sc[i]/s0[i]) & 0xffff) << (16*i);
      }
      return v0;
    }

    /******************************************************/
    // get_double functions

    // Get floating-point value for a given (x,y) coordinate.
    // This is fast version, without range checks.
    // Should be used after check_crd/check_rng checks.
    virtual double get_double(const size_t x, const size_t y) const {
      return NAN; }

    // Get floating-point value for a given iPoint coordinate.
    // This is fast version, without range checks.
    // Should be used after check_crd/check_rng checks.
    double get_double(const iPoint & p) const {
      return get_double(p.x, p.y); }

    // Check coordinates and get floating-point value.
    // Returns bgcolor if check_crd() fails.
    double get_double_safe(const size_t x, const size_t y) const {
      return check_crd(x,y) ? get_double(x,y) : bgcolor;}

    // Check coordinates and get floating-point value.
    // Returns bgcolor if check_crd() fails.
    double get_double_safe(const iPoint & p) const {
      return check_crd(p.x, p.y) ? get_double(p.x,p.y) : bgcolor; }

    // Check coordinates and get floating-point value
    //  using 4-point interpolation.
    virtual double get_double_int4(const dPoint & p) const {
      int x1 = floor(p.x), x2 = x1+1;
      int y1 = floor(p.y), y2 = y1+1;
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      double v1 = get_double(x1,y1);
      double v2 = get_double(x1,y2);
      double v3 = get_double(x2,y1);
      double v4 = get_double(x2,y2);
      double dy = p.y-y1, dx = p.x-x1;
      return v1*(1.-dx)*(1.-dy) + v2*(1.-dx)*dy +
             v3*dx*(1.-dy) + v4*dx*dy;
    }

    // Check coordinates and get RGB averaged color
    // (with Gaussian weight) in radius `rad`.
    double get_double_avrg(const dPoint & p, double rad=2.0) const {
      int x1 = floor(p.x-rad), x2 = ceil(p.x+rad);
      int y1 = floor(p.y-rad), y2 = ceil(p.y+rad);
      if (!check_rng(x1,y1,x2,y2)) return bgcolor;
      double s0 = 0, s1 = 0;
      if (x1==x2 || y1==y2) return bgcolor;
      for (int y=y1; y<=y2; ++y){
        for (int x=x1; x<=x2; ++x){
          double v = get_double(x,y);
          double d = dist2d(p, dPoint(x,y));
          double w = exp(-d/rad*2);
          s0 += w;
          s1 += v*w;
        }
      }
      return s1/s0;
    }

    /******************************************************/

    virtual std::ostream & print (std::ostream & s) const {
      s << "Image(0x" << std::hex << bgcolor << ")";
      return s;
    }

};

std::ostream & operator<< (std::ostream & s, const Image & i);

#endif
