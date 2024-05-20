#ifndef IMAGE_R_H
#define IMAGE_R_H

#include <stdint.h>

#include <vector>
#include <memory>
#include <cstring>
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
  IMAGE_64ARGB, // 8 bytes per pixel data: AARRGGBB colors
  IMAGE_48RGB,  // 6 bytes per pixel data: RRGGBB colors
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
        case IMAGE_64ARGB: return w*h*8;
        case IMAGE_48RGB:  return w*h*6;
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

    // Fast get function for image type IMAGE_64ARGB
    uint64_t get64(const size_t x, const size_t y) const{
      return ((uint64_t*)data_.get())[w*y+x]; }

    // Fast get function for image type IMAGE_48RGB
    uint64_t get48(const size_t x, const size_t y) const{
      auto d = data_.get() + 3*(w*y+x);
      return 0xFFFF000000000000l
       + ((uint64_t)d[0]<<40) + ((uint64_t)d[1]<<32) + (d[2]<<24)
       + (d[3]<<16) + (d[4]<<8) + d[5];
    }

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
      return get1(x,y) ? 0xFF000000:0xFFFFFFFF;
    }

    // Fast get function for image type IMAGE_1.
    bool get1(const size_t x, const size_t y) const{
      size_t b = (w*y+x)/8; // byte
      size_t o = 7-(w*y+x)%8; // offset
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
        case IMAGE_64ARGB: return color_rgb_64to32(get64(x,y));
        case IMAGE_48RGB:  return color_rgb_64to32(get48(x,y));
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
      if (t==IMAGE_48RGB) return color_rgb_64to32(get48(x,y));
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

    bool check_crd(const size_t x, const size_t y) const override{
      return x>=0 && x<w && y>=0 && y<h;
    }

    bool check_rng(const size_t x1, const size_t y1,
                   const size_t x2, const size_t y2) const override{
      return x1>=0 && x2<w && y1>=0 && y2<h;
    }

    // redefine Image::get_color:
    uint32_t get_color_fast(const size_t x, const size_t y) override { return get_argb(x,y); }

    /******************************************************/
    // Fast set functions for different image types.
    // Image type and coordinate range should be checked before.

    // Fast set function for image type IMAGE_64ARGB.
    void set64(const size_t x, const size_t y, const uint64_t v){
      ((uint64_t*)data_.get())[w*y+x] = v; }

    // Fast set function for image type IMAGE_48RGB
    void set48(const size_t x, const size_t y, const uint64_t v){
      data_.get()[6*(w*y+x)]   = (v>>40) & 0xFF;
      data_.get()[6*(w*y+x)+1] = (v>>32) & 0xFF;
      data_.get()[6*(w*y+x)+2] = (v>>24) & 0xFF;
      data_.get()[6*(w*y+x)+3] = (v>>16) & 0xFF;
      data_.get()[6*(w*y+x)+4] = (v>>8) & 0xFF;
      data_.get()[6*(w*y+x)+5] = v & 0xFF;
    }
    // Fast set function for image type IMAGE_48RGB
    void set48(const size_t x, const size_t y, const uint8_t * p){
      memcpy(data_.get() + 6*(w*y+x), p, 6);
    }

    // Fast set function for image type IMAGE_32ARGB.
    void set32(const size_t x, const size_t y, const uint32_t v){
      ((uint32_t*)data_.get())[w*y+x] = v; }

    // Fast set function for image type IMAGE_24RGB
    void set24(const size_t x, const size_t y, const uint32_t v){
      data_.get()[3*(w*y+x)]   = (v>>16) & 0xFF;
      data_.get()[3*(w*y+x)+1] = (v>>8)  & 0xFF;
      data_.get()[3*(w*y+x)+2] = v & 0xFF;
    }
    // Fast set function for image type IMAGE_24RGB
    void set24(const size_t x, const size_t y, const uint8_t * p){
      memcpy(data_.get() + 3*(w*y+x), p, 3);
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
      size_t o = 7-(w*y+x)%8; // offset
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
    void fill64(const uint64_t v) const{
      for (size_t i=0; i<w*h; i++)
        ((uint64_t*)data_.get())[i] = v;
    }

    // Fill function for image type IMAGE_24RGB
    void fill48(const uint64_t v) const{
      for (size_t i=0; i<w*h; i++) {
        data_.get()[6*i]   = (v>>40) & 0xFF;
        data_.get()[6*i+1] = (v>>32) & 0xFF;
        data_.get()[6*i+2] = (v>>24) & 0xFF;
        data_.get()[6*i+3] = (v>>16) & 0xFF;
        data_.get()[6*i+4] = (v>>8)  & 0xFF;
        data_.get()[6*i+5] = v & 0xFF;
      }
    }

    // Fill function for image type IMAGE_32ARGB.
    void fill32(const uint32_t v) const{
      for (size_t i=0; i<w*h; i++)
        ((uint32_t*)data_.get())[i] = v;
    }

    // Fill function for image type IMAGE_24RGB
    void fill24(const uint32_t v) const{
      for (size_t i=0; i<w*h; i++) {
        data_.get()[3*i]   = (v>>16) & 0xFF;
        data_.get()[3*i+1] = (v>>8)  & 0xFF;
        data_.get()[3*i+2] = v & 0xFF;
      }
    }

    // Fill function for image type IMAGE_16
    void fill16(const uint16_t v) const{
      for (size_t i=0; i<w*h; i++)
        ((uint16_t *)data_.get())[i] = v;
    }

    // Fill function for image types IMAGE_8 and IMAGE_8PAL
    void fill8(const uint8_t v) const{
      for (size_t i=0; i<w*h; i++)
        data_.get()[i] = v;
    }

    // Fill function for image type IMAGE_1.
    void fill1(const bool v) const{
      for (size_t i=0; i<dsize(); i++)
        data_.get()[i] = v? 0xFF:0x00;
    }

    // Fill function for image type IMAGE_FLOAT
    void fillF(const float v) const{
      for (size_t i=0; i<w*h; i++)
        ((float*)data_.get())[i] = v;
    }


    // Fill function for image type IMAGE_DOUBLE
    void fillD(const double v) const{
      for (size_t i=0; i<w*h; i++)
        ((double*)data_.get())[i] = v;
    }

    std::ostream & print (std::ostream & s) const override{
      if (width()==0 || height()==0) {
        s << "ImageR(empty)";
      }
      else {
        s << "ImageR(" << width() << "x" << height() << ", ";

        switch (type()){
          case IMAGE_32ARGB:  s << "ARGB, 32bpp"; break;
          case IMAGE_24RGB:   s << "RGB, 24bpp"; break;
          case IMAGE_16:      s << "Grey, 16bpp"; break;
          case IMAGE_8:       s << "Grey, 8bpp"; break;
          case IMAGE_8PAL:    s << "Palette, 8bpp"; break;
          case IMAGE_1:       s << "B/W, 1bpp"; break;
          case IMAGE_FLOAT:   s << "float"; break;
          case IMAGE_DOUBLE:  s << "double"; break;
          case IMAGE_64ARGB:  s << "ARGB, 64bpp"; break;
          case IMAGE_48RGB:   s << "RGB, 48bpp"; break;
          default: s << "Unknown data format"; break;
        }
        s << ")";
      }
      return s;
    }

};

#endif
