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
  IMAGE_64ARGB, // 8 bytes per pixel data: AaRrGgBb colors
  IMAGE_48RGB,  // 6 bytes per pixel data: RrGgBb colors
  IMAGE_UNKNOWN,// unknown data format
};

/*********************************************************************/
class ImageR : public Image {
  private:
    std::shared_ptr<unsigned char[]> data_;
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
        case IMAGE_1:      return h*((w+7)/8);
        case IMAGE_FLOAT:  return (w*h)*sizeof(float);
        case IMAGE_DOUBLE: return (w*h)*sizeof(double);
        case IMAGE_64ARGB: return w*h*8;
        case IMAGE_48RGB:  return w*h*6;
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

      if (type==IMAGE_8PAL) cmap.resize(256);

      try{
        data_ = std::shared_ptr<unsigned char[]>(new unsigned char[dsize()]);
      }
      catch (const std::bad_alloc & e) {
        throw Err() << "ImageR: can't allocate memory for "
                    << *this << ": " << e.what();
      }
    }


    /******************************************************/
    // Fast get/set/fill functions for different image types.
    // Return raw data or uint32_t ARGB color (*col versions)
    // Image type and coordinate range should be checked before.
    // Static functions define internal byte order!
    // We want IMAGE_32 to match with Cairo format


    static uint64_t get64(unsigned char *p){ return *(uint64_t*)p; }
    static void set64(unsigned char *p, const uint64_t v){ *(uint64_t*)p = v; }

    // Fast get function for image type IMAGE_64ARGB
    uint64_t get64(const size_t x, const size_t y) const{
      return get64(data_.get() + 8*(w*y+x));
    }
    // Fast set function for image type IMAGE_64ARGB.
    void set64(const size_t x, const size_t y, const uint64_t v){
      set64(data_.get() + 8*(w*y+x), v);
    }
    // Fill function for image type IMAGE_32ARGB.
    void fill64(const uint64_t v) const{
      for (size_t i=0; i<w*h; i++) set64(data_.get() + 8*i, v);
    }


    static uint64_t get48(unsigned char *p){
      uint64_t v = 0xFFFFull << 48;
      std::memcpy(&v, p, 6);
      return v;
    }
    static void set48(unsigned char *p, const uint64_t v){
      std::memcpy(p, &v, 6);
    }
    // Fast get function for image type IMAGE_48RGB
    uint64_t get48(const size_t x, const size_t y) const{
      return get48(data_.get() + 6*(w*y+x));
    }
    // Fast set function for image type IMAGE_48RGB
    void set48(const size_t x, const size_t y, const uint64_t v){
      set48(data_.get() + 6*(w*y+x), v);
    }
    // Fill function for image type IMAGE_24RGB
    void fill48(const uint64_t v) const{
      for (size_t i=0; i<w*h; i++) set48(data_.get() + 6*i, v);
    }


    static uint32_t get32(unsigned char *p){ return *(uint32_t*)p; }
    static void set32(unsigned char *p, const uint32_t v){ *(uint32_t*)p = v; }

    // Fast get function for image type IMAGE_32ARGB
    uint32_t get32(const size_t x, const size_t y) const{
      return get32(data_.get() + 4*(w*y+x));
    }
    // Fast set function for image type IMAGE_32ARGB.
    void set32(const size_t x, const size_t y, const uint32_t v){
      set32(data_.get() + 4*(w*y+x), v);
    }
    // Fill function for image type IMAGE_32ARGB.
    void fill32(const uint32_t v) const{
      for (size_t i=0; i<w*h; i++) set32(data_.get() + 4*i, v);
    }


    static uint32_t get24(unsigned char *p){
      uint32_t v = 0xFF << 24;
      std::memcpy(&v, p, 3);
      return v;
    }
    static void set24(unsigned char *p, const uint32_t v){
      std::memcpy(p, &v, 3);
    }
    // Fast get function for image type IMAGE_24RGB
    uint32_t get24(const size_t x, const size_t y) const{
      return get24(data_.get() + 3*(w*y+x));
    }
    // Fast set function for image type IMAGE_24RGB
    void set24(const size_t x, const size_t y, const uint32_t v){
      set24(data_.get() + 3*(w*y+x), v);
    }
    // Fill function for image type IMAGE_24RGB
    void fill24(const uint32_t v) const{
      for (size_t i=0; i<w*h; i++) set24(data_.get() + 3*i, v);
    }


    static uint16_t get16(unsigned char *p){ return *(uint16_t*)p; }
    static void set16(unsigned char *p, const uint16_t v){ *(uint16_t*)p = v; }

    // Fast get function for image type IMAGE_16
    uint16_t get16(const size_t x, const size_t y) const{
      return get16(data_.get() + 2*(w*y+x));
    }
    // Fast color get function for image type IMAGE_16
    // To convert 16-bit data into color the less significant byte is skipped.
    uint32_t get16col(const size_t x, const size_t y) const{
      uint8_t c = get16(data_.get() + 2*(w*y+x)) >> 8;
      return 0xFF000000 + (c<<16) + (c<<8) + c;
    }
    // Fast set function for image type IMAGE_16
    void set16(const size_t x, const size_t y, const uint16_t v){
      set16(data_.get() + 2*(w*y+x), v);
    }
    // Fill function for image type IMAGE_16
    void fill16(const uint16_t v) const{
      for (size_t i=0; i<w*h; i++) set16(data_.get() + 2*i, v);
    }


    // Fast get function for image type IMAGE_8
    uint8_t get8(const size_t x, const size_t y) const{
      return data_.get()[w*y+x];
    }
    // Fast color get function for image type IMAGE_8
    // To convert 8-bit data into color the less significant byte is skipped.
    uint32_t get8col(const size_t x, const size_t y) const{
      uint8_t c = data_.get()[w*y+x];
      return 0xFF000000 + (c<<16) + (c<<8) + c;
    }
    // Fast color get function for image type IMAGE_8PAL
    uint32_t get8pal(const size_t x, const size_t y) const{
      return cmap[data_.get()[w*y+x]];
    }
    // Fast set function for image type IMAGE_8
    void set8(const size_t x, const size_t y, const uint8_t v){
      data_.get()[w*y+x] = v;
    }
    // Fill function for image type IMAGE_8
    void fill8(const uint8_t v) const{
      for (size_t i=0; i<w*h; i++) data_.get()[i] = v;
    }

    // Fast get function for image type IMAGE_1.
    // Same format as in raw PBM file: packed bits restarted on each line.
    bool get1(const size_t x, const size_t y) const{
      size_t b = y*((w+7)/8) + x/8; // byte
      size_t o = 7-x%8; // offset
      uint8_t v = data_.get()[b];
      return (v >> o) & 1;
    }
    // Fast color get function for image type IMAGE_1
    // 1=black as in PBM format
    uint32_t get1col(const size_t x, const size_t y) const{
      return get1(x,y) ? 0xFF000000:0xFFFFFFFF;
    }
    // Fast set function for image type IMAGE_1.
    void set1(const size_t x, const size_t y, const bool v){
      size_t b = y*((w+7)/8) + x/8; // byte
      size_t o = 7-x%8; // offset
      uint8_t old = data_.get()[b];
      data_.get()[b] = v? old|(1<<o) : old&~(1<<o);
    }
    // Fill function for image type IMAGE_1.
    void fill1(const bool v) const{
      for (size_t i=0; i<dsize(); i++)
        data_.get()[i] = v? 0xFF:0x00;
    }


    // Fast get function for image type IMAGE_FLOAT
    float getF(const size_t x, const size_t y) const{
      return ((float *)data_.get())[w*y+x]; }
    // Fast set function for image type IMAGE_FLOAT
    void setF(const size_t x, const size_t y, const float v){
      ((float*)data_.get())[w*y+x] = v; }
    // Fill function for image type IMAGE_FLOAT
    void fillF(const float v) const{
      for (size_t i=0; i<w*h; i++)
        ((float*)data_.get())[i] = v;
    }


    // Fast get function for image type IMAGE_DOUBLE
    double getD(const size_t x, const size_t y) const{
      return ((double *)data_.get())[w*y+x]; }
    // Fast set function for image type IMAGE_DOUBLE
    void setD(const size_t x, const size_t y, const double v){
      ((double*)data_.get())[w*y+x] = v; }
    // Fill function for image type IMAGE_DOUBLE
    void fillD(const double v) const{
      for (size_t i=0; i<w*h; i++)
        ((double*)data_.get())[i] = v;
    }


    // Color get function for IMAGE_UNKNOWN.
    virtual uint32_t getUcol(const size_t x, const size_t y) const{
      return bgcolor;
    }
    // Color set function for IMAGE_UNKNOWN.
    // To be redefined in non-standard image classes.
    virtual void setUcol(const size_t x, const size_t y){
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

    // Get 64-bit ARGB (prescaled) color for any image type.
    uint64_t get_argb64(const size_t x, const size_t y) const {
      if (t== IMAGE_64ARGB) return get64(x,y);
      if (t== IMAGE_48RGB)  return get48(x,y);
      return color_rgb_32to64(get_argb(x,y));
    }

    // Get 48-bit RGB color for any image type.
    uint64_t get_rgb64(const size_t x, const size_t y) const {
      if (t== IMAGE_48RGB)  return get48(x,y);
      if (t== IMAGE_64ARGB) return get64(x,y);
      return color_rgb_32to64(get_rgb(x,y));
    }


    // Get 8-bit grey color for any image type.
    uint8_t get_grey8(const size_t x, const size_t y) const{
      if (t==IMAGE_8)  return get8(x,y);
      if (t==IMAGE_16) return get16(x,y)>>8;
      if (t==IMAGE_48RGB)  return color_rgb64_to_grey8(get48(x,y));
      if (t==IMAGE_64ARGB) return color_rgb64_to_grey8(get64(x,y));
      return color_rgb_to_grey8(get_rgb(x,y));
    }

    // Get alpha channel + 8-bit grey color for any image type.
    uint16_t get_agrey8(const size_t x, const size_t y) const{
      if (t==IMAGE_8)  return 0xFF00 + get8(x,y);
      if (t==IMAGE_16) return 0xFF00 + (get16(x,y)>>8);
      if (t==IMAGE_48RGB)  return 0xFF00 + color_rgb64_to_grey8(get48(x,y));
      if (t==IMAGE_64ARGB) return 0xFF00 + color_rgb64_to_grey8(get64(x,y)); // fixme
      uint32_t c = get_argb(x,y);
      return ((c>>16) & 0xFF00) +
             color_rgb_to_grey8(color_rem_transp(c,false));
    }

    // Get 16-bit grey color for any image type.
    uint16_t get_grey16(const size_t x, const size_t y) const{
      if (t==IMAGE_8)  return get8(x,y)<<8;
      if (t==IMAGE_16) return get16(x,y);
      if (t==IMAGE_48RGB)  return color_rgb64_to_grey16(get48(x,y));
      if (t==IMAGE_64ARGB) return color_rgb64_to_grey16(get64(x,y));
      return color_rgb_to_grey16(get_rgb(x,y));
    }

    // get double value for any image type.
    double get_double(const size_t x, const size_t y) const {
      if (t==IMAGE_DOUBLE) return getD(x,y);
      if (t==IMAGE_FLOAT)  return (double)getF(x,y);
      if (t==IMAGE_16) return (double)get16(x,y);
      if (t==IMAGE_8)  return (double)get8(x,y);
      if (t==IMAGE_1)  return (double)get1(x,y);
      throw Err() << "Image::get_double: unsupported image type";
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
