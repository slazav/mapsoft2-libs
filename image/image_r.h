#ifndef IMAGE_R_H
#define IMAGE_R_H

#include <stdint.h>

#include <vector>
#include <memory>
#include "err/err.h"
#include "geom/rect.h"

#include "image.h"
#include "colors.h"

/*
An raster image: 2d array of arbitrary data.
*/

// ImageDataType shows data type of the image.
// One should not directly access data if type
// is IMAGE_UNKNOWN.
enum ImageDataType {
  IMAGE_32ARGB, // 4 bytes per pixel data: ARGB colors
  IMAGE_24RGB,  // 3 bytes per pixel data: RGB colors
  IMAGE_16,     // 2 bytes per pixel, gryscale
  IMAGE_8,      // 1 byte per pixel, gryscale
  IMAGE_8PAL,   // 1 byte per pixel + color palette
  IMAGE_1,      // 1 bit per pixel, black and white
  IMAGE_FLOAT,  // float-value pixel
  IMAGE_DOUBLE, // double-value pixel
  IMAGE_UNKNOWN,// unknown data format
};

/*********************************************************************/
class ImageR : public Image {
  private:
    std::shared_ptr<uint8_t> data_;
    size_t w,h; // width, height
    ImageDataType t;

  public:

    std::vector<uint32_t> cmap;

    ImageDataType type() const   {return t;}
    size_t width() const  {return w;}
    size_t height() const {return h;}
    iRect bbox() const    {return iRect(0,0,w,h);}
    iPoint size() const   {return iPoint(w,h);}
    unsigned char *data() const {return data_.get();}

    bool is_empty() const {return w==0 || h==0;}
    operator bool() const{ return w!=0 && h!=0; }

    // get data size (0 for IMAGE_UNKNOWN)
    size_t dsize() const {
      switch (t){
        case IMAGE_32ARGB: return w*h*4;
        case IMAGE_24RGB:  return w*h*3;
        case IMAGE_16:     return w*h*2;
        case IMAGE_8:      return w*h;
        case IMAGE_8PAL:   return w*h;
        case IMAGE_1:      return (w*h-1)/8+1;
        case IMAGE_FLOAT:  return (w*h)*sizeof(float);
        case IMAGE_DOUBLE: return (w*h)*sizeof(double);
        case IMAGE_UNKNOWN: return 0;
      }
      return 0;
    }

    // constructor -- create an empty image
    ImageR(): w(0), h(0), t(IMAGE_UNKNOWN) {}

    // constructor -- create WxH non-initialized image
    ImageR(const size_t W, const size_t H,
          const ImageDataType type): w(W), h(H), t(type){

      if (w<=0 || h<=0)
        throw Err() << "non-positive image dimension: " << w << "x" << h;

      try{
        data_ = std::shared_ptr<unsigned char>(new uint8_t[dsize()]);
      }
      catch (const std::bad_alloc & e) {
        throw Err() << "ImageR: can't allocate memory for "
                    << *this << ": " << e.what();
      }
    }


    /******************************************************/
    // Fast get functions for different image types.
    // Return raw data or uint32_t ARGB color (*col versions)
    // Image type and coordinate range should be checked before.

    // Fast get function for image type IMAGE_32ARGB
    uint32_t get32(const size_t x, const size_t y) const{
      return ((uint32_t*)data_.get())[w*y+x]; }

    // Fast get function for image type IMAGE_24RGB
    uint32_t get24(const size_t x, const size_t y) const{
      auto d = data_.get() + 3*(w*y+x);
      return 0xFF000000 + (d[0]<<16) + (d[1]<<8) + d[2];
    }

    // Fast get function for image type IMAGE_16
    uint16_t get16(const size_t x, const size_t y) const{
      return ((uint16_t*)data_.get())[w*y+x]; }

    // Fast color get function for image type IMAGE_16
    // To convert 16-bit data into color the less significant byte is skipped.
    uint32_t get16col(const size_t x, const size_t y) const{
      uint16_t c = (((uint16_t*)data_.get())[w*y+x] >> 8) & 0xFF;
      return 0xFF000000 + (c<<16) + (c<<8) + c;
    }

    // Fast get function for image types IMAGE_8 and IMAGE_8PAL
    uint8_t get8(const size_t x, const size_t y) const{
      return data_.get()[w*y+x]; }

    // Fast color get function for image type IMAGE_8
    // Image type and coordinate range should be checked before.
    uint32_t get8col(const size_t x, const size_t y) const{
      uint8_t c = data_.get()[w*y+x];
      return 0xFF000000 + (c<<16) + (c<<8) + c;
    }

    // Fast color get function for image type IMAGE_8PAL
    uint32_t get8pal(const size_t x, const size_t y) const{
      return cmap[data_.get()[w*y+x]];
    }

    // Fast color get function for image type IMAGE_1
    uint32_t get1col(const size_t x, const size_t y) const{
      return get1(x,y) ? 0xFFFFFFFF:0xFF000000;
    }

    // Fast get function for image type IMAGE_1.
    bool get1(const size_t x, const size_t y) const{
      size_t b = (w*y+x)/8; // byte
      size_t o = (w*y+x)%8; // offset
      uint8_t v = data_.get()[b];
      return (v >> o) & 1;
    }

    // Fast get function for image type IMAGE_FLOAT
    float getF(const size_t x, const size_t y) const{
      return ((float *)data_.get())[w*y+x]; }

    // Fast get function for image type IMAGE_DOUBLE
    double getD(const size_t x, const size_t y) const{
      return ((double *)data_.get())[w*y+x]; }

    // Color get function for IMAGE_UNKNOWN.
    virtual uint32_t getUcol(const size_t x, const size_t y) const{
      return bgcolor;
    }

    /******************************************************/
    // Universal get functions, should work for any image type
    // Coordinate range still should be checked before.

    // Get ARGB (prescaled) color for any image type.
    uint32_t get_argb(const size_t x, const size_t y) const {
      switch (t){
        case IMAGE_32ARGB: return get32(x,y);
        case IMAGE_24RGB:  return get24(x,y);
        case IMAGE_16:     return get16col(x,y);
        case IMAGE_8:      return get8col(x,y);
        case IMAGE_8PAL:   return get8pal(x,y);
        case IMAGE_1:      return get1col(x,y);
        case IMAGE_FLOAT:  return 0; // todo: rainbow calculation?
        case IMAGE_DOUBLE: return 0; // todo: rainbow calculation?
        case IMAGE_UNKNOWN: return getUcol(x,y);
      }
      return 0;
    }

    // Get RGB color for any image type.
    uint32_t get_rgb(const size_t x, const size_t y) const{
      if (t==IMAGE_24RGB) return get24(x,y);
      if (t==IMAGE_16)    return get16col(x,y);
      if (t==IMAGE_8)     return get8col(x,y);
      if (t==IMAGE_1)     return get1col(x,y);
      return color_rem_transp(get_argb(x,y),false);
    }

    // Get 8-bit grey color for any image type.
    uint8_t get_grey8(const size_t x, const size_t y) const{
      if (t==IMAGE_8)  return get8(x,y);
      if (t==IMAGE_16) return get16(x,y)>>8;
      return color_rgb_to_grey8(get_rgb(x,y));
    }

    // Get alpha channel + 8-bit grey color for any image type.
    uint16_t get_agrey8(const size_t x, const size_t y) const{
      if (t==IMAGE_8)  return 0xFF00 + get8(x,y);
      if (t==IMAGE_16) return 0xFF00 + (get16(x,y)>>8);
      uint32_t c = get_argb(x,y);
      return ((c>>16) & 0xFF00) +
             color_rgb_to_grey8(color_rem_transp(c,false));
    }

    // Get 16-bit grey color for any image type.
    uint16_t get_grey16(const size_t x, const size_t y) const{
      if (t==IMAGE_8)  return get8(x,y)<<8;
      if (t==IMAGE_16) return get16(x,y);
      return color_rgb_to_grey16(get_rgb(x,y));
    }

    /******************************************************/
    // overrides for Image interface

    bool check_crd(const int x, const int y) const override{
      return x>=0 && x<w && y>=0 && y<h;
    }

    bool check_rng(const int x1, const int y1,
                   const int x2, const int y2) const override{
      return x1>=0 && x2<w && y1>=0 && y2<h;
    }

    // redefine Image::get_color:
    uint32_t get_color(const int x, const int y) override{
      if (!check_crd(x,y)) return bgcolor;
      return get_argb(x,y);
    }

    /******************************************************/
    // to be moved to Image?

    // Get color value for a dPoint<double> with range checks.
    uint32_t get_argb_safe(const dPoint & p) const{
      int x=rint(p.x), y=rint(p.y);
      if (!check_crd(x,y)) return bgcolor;
      return get_argb(x,y);
    }

    // Get color value using 4-point interpolation.
    uint32_t get_argb_int4(const dPoint & p) const{
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
        double c12 = c1+(c2-c1)*(p.y-y1);
        double c34 = c3+(c4-c3)*(p.y-y1);
        double c0 = c12+(c34-c12)*(p.x-x1);
        v0 += ((int)c0 & 0xff) << sh;
      }
      return v0;
    }

    // Get averaged color value in radius `rad`.
    uint32_t get_argb_avrg(const dPoint & p, double rad=2.0) const{
      int x1 = floor(p.x-rad), x2 = ceil(p.x+rad);
      int y1 = floor(p.y-rad), y2 = ceil(p.y+rad);
      if (x1<0) x1=0; if (x1>=w) x1=w-1;
      if (x2<0) x2=0; if (x2>=w) x2=w-1;
      if (y1<0) y1=0; if (y1>=h) y1=h-1;
      if (y2<0) y2=0; if (y2>=h) y2=h-1;
      double sc[4] = {0,0,0,0};
      double s0[4] = {0,0,0,0};
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
          v0 += ((int)(sc[i]/s0[i]) & 0xff) << (8*i);
      }
      return v0;
    }


    /******************************************************/
    // Fast set functions for different image types.
    // Image type and coordinate range should be checked before.

    // Fast set function for image type IMAGE_32ARGB.
    void set32(const size_t x, const size_t y, const uint32_t v){
      ((uint32_t*)data_.get())[w*y+x] = v; }

    // Fast set function for image type IMAGE_24RGB
    void set24(const size_t x, const size_t y, const uint32_t v){
      data_.get()[3*(w*y+x)]   = (v>>16) & 0xFF;
      data_.get()[3*(w*y+x)+1] = (v>>8)  & 0xFF;
      data_.get()[3*(w*y+x)+2] = v & 0xFF;
    }

    // Fast set function for image type IMAGE_16
    void set16(const size_t x, const size_t y, const uint16_t v){
      ((uint16_t *)data_.get())[w*y+x] = v; }

    // Fast set function for image types IMAGE_8 and IMAGE_8PAL
    void set8(const size_t x, const size_t y, const uint8_t v){
      data_.get()[w*y+x] = v; }

    // Fast set function for image type IMAGE_1.
    void set1(const size_t x, const size_t y, const bool v){
      size_t b = (w*y+x)/8; // byte
      size_t o = (w*y+x)%8; // offset
      uint8_t old = data_.get()[b];
      data_.get()[b] = v? old|(1<<o) : old&~(1<<o);
    }

    // Fast set function for image type IMAGE_FLOAT
    void setF(const size_t x, const size_t y, const float v){
      ((float*)data_.get())[w*y+x] = v; }

    // Fast set function for image type IMAGE_DOUBLE
    void setD(const size_t x, const size_t y, const double v){
      ((double*)data_.get())[w*y+x] = v; }

    // Color set function for IMAGE_UNKNOWN.
    // To be redefined in non-standard image classes.
    virtual void setUcol(const size_t x, const size_t y){
    }

    /******************************************************/
    // Fill functions for different image types.
    // Image type should be checked before.

    // Fill function for image type IMAGE_32ARGB.
    void fill32(const uint32_t v) const{
      for (int i=0; i<w*h; i++)
        ((uint32_t*)data_.get())[i] = v;
    }

    // Fill function for image type IMAGE_24RGB
    void fill24(const uint32_t v) const{
      for (int i=0; i<w*h; i++) {
        data_.get()[3*i]   = (v>>16) & 0xFF;
        data_.get()[3*i+1] = (v>>8)  & 0xFF;
        data_.get()[3*i+2] = v & 0xFF;
      }
    }

    // Fill function for image type IMAGE_16
    void fill16(const uint16_t v) const{
      for (int i=0; i<w*h; i++)
        ((uint16_t *)data_.get())[i] = v;
    }

    // Fill function for image types IMAGE_8 and IMAGE_8PAL
    void fill8(const uint8_t v) const{
      for (int i=0; i<w*h; i++)
        data_.get()[i] = v;
    }

    // Fill function for image type IMAGE_1.
    void fill1(const bool v) const{
      for (int i=0; i<dsize(); i++)
        data_.get()[i] = v? 0xFF:0x00;
    }

    // Fill function for image type IMAGE_FLOAT
    void fillF(const float v) const{
      for (int i=0; i<w*h; i++)
        ((float*)data_.get())[i] = v;
    }


    // Fill function for image type IMAGE_DOUBLE
    void fillD(const double v) const{
      for (int i=0; i<w*h; i++)
        ((double*)data_.get())[i] = v;
    }

};


std::ostream & operator<< (std::ostream & s, const ImageR & i);

#endif