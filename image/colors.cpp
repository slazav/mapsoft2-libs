#include "colors.h"
#include <cmath>
#include <iomanip>

// distance between two colors
double
color_dist(const uint32_t c1, const uint32_t c2, const bool prescaled){
  if (c1 == c2) return 0.0;

  double a1 = (c1>>24)&0xFF, a2 = (c2>>24)&0xFF;
  double r1 = (c1>>16)&0xFF, r2 = (c2>>16)&0xFF;
  double g1 = (c1>>8)&0xFF,  g2 = (c2>>8)&0xFF;
  double b1 = c1&0xFF,       b2 = c2&0xFF;

  if (!prescaled || (a1==0xFF && a2==0xFF)){
    double da = a1-a2;
    double dr = r1-r2;
    double dg = g1-g2;
    double db = b1-b2;
    return sqrt( da*da + dr*dr + dg*dg + db*db );
  }

  if (r1>a1 || g1>a1 || b1>a1)
    throw Err() << "color_dist: non-prescaled color: 0x"
                << std::hex << std::setfill('0') << std::setw(8) << c1;
  if (r2>a2 || g2>a2 || b2>a2)
    throw Err() << "color_dist: non-prescaled color: 0x"
                << std::hex << std::setfill('0') << std::setw(8) << c2;

  // note: two almost transparent colors can have up to 0xFF distance
  // in each component; we want distance sqrt(3)*0xFF between fully transparent and
  // any almost transparent color, and distance 0 between same colors.
  if (a1==0 && a2==0) return 0;
  if (a1==0 || a2==0) return sqrt((a1-a2)*(a1-a2)+ 3*0xFF*0xFF);

  double da = a1-a2;
  double dr = r1*255.0/a1 - r2*255.0/a2;
  double dg = g1*255.0/a1 - g2*255.0/a2;
  double db = b1*255.0/a1 - b2*255.0/a2;

  return sqrt( da*da + dr*dr + dg*dg + db*db );
}


// Assemble 32-bit color from a,r,g,b components.
// Prescaled semi-transparent colors are used
uint32_t color_argb(const uint8_t a, const uint8_t r,
                    const uint8_t g, const uint8_t b){
  if (a==0) return 0;
  if (a==0xFF) return 0xFF000000 + ((uint32_t)r<<16) + ((uint32_t)g<<8) + b;
  return ((uint32_t)a<<24) + ((uint32_t)r*a/255<<16) +
         ((uint32_t)g*a/255<<8) + ((uint32_t)b*a/255);
}

// Assemble 64-bit color from a,r,g,b components.
// Prescaled semi-transparent colors are used
uint64_t color_argb64(const uint16_t a, const uint16_t r,
                     const uint16_t g, const uint16_t b){
  if (a==0) return 0;
  if (a==0xFFFF) return (0xFFFFull<<48) + ((uint64_t)r<<32) + ((uint64_t)g<<16) + b;
  return ((uint64_t)a<<48) + ((uint64_t)r*a/0xFFFF<<32) +
         ((uint64_t)g*a/0xFFFF<<16) + ((uint64_t)b*a/0xFFFF);
}

// Convert to prescale color
uint32_t color_prescale(const uint32_t c){
  uint32_t a = c>>24;
  if (a==0xFF) return c;
  if (a==0) return 0;
  uint32_t r = (c>>16)&0xFF;
  uint32_t g = (c>>8)&0xFF;
  uint32_t b = c&0xFF;
  return (a<<24) + (r*a/255<<16) + (g*a/255<<8) + (b*a/255);
}


// remove transparency (for scaled colors)
uint32_t color_rem_transp(const uint32_t c, const bool gifmode){
  int a = (c>>24)&0xFF;
  if (a==255) return c;

  int r = (c>>16)&0xFF;
  int g = (c>>8)&0xFF;
  int b = c&0xFF;
  if (r>a || g>a || b>a)
    throw Err() << "color_rem_transp: non-prescaled color: 0x"
                << std::hex << std::setfill('0') << std::setw(8) << c;
  if (a==0) return gifmode? 0 : 0xFF000000;

  r = (r*255)/a;
  g = (g*255)/a;
  b = (b*255)/a;
  if (r>0xFF) r=0xFF;
  if (g>0xFF) g=0xFF;
  if (b>0xFF) b=0xFF;
  return (0xFF<<24)+(r<<16)+(g<<8)+b;
}

// remove transparency (for scaled colors)
uint64_t color_rem_transp64(const uint64_t c, const bool gifmode){
  uint64_t a = (c>>48)&0xFFFF;
  if (a==0xFFFF) return c;

  uint64_t r = (c>>32)&0xFFFF;
  uint64_t g = (c>>16)&0xFFFF;
  uint64_t b = c&0xFFFF;
  if (r>a || g>a || b>a)
    throw Err() << "color_rem_transp: non-prescaled color: 0x"
                << std::hex << std::setfill('0') << std::setw(16) << c;
  if (a==0) return gifmode? 0 : (0xFFFFull<<48);

  r = (r*0xFFFF)/a;
  g = (g*0xFFFF)/a;
  b = (b*0xFFFF)/a;
  if (r>0xFFFF) r=0xFFFF;
  if (g>0xFFFF) g=0xFFFF;
  if (b>0xFFFF) b=0xFFFF;
  return (0xFFFFull<<48) + (r<<32) + (g<<16) + b;
}

// Convert RGB color to 8-bit greyscale
uint8_t color_rgb_to_grey8(const uint32_t c){
  return rint(
    COLOR_LUMINR*((c>>16)&0xFF) +
    COLOR_LUMING*((c>>8)&0xFF) +
    COLOR_LUMINB*(c&0xFF) );
}

// Convert RGB color to 8-bit greyscale
uint8_t color_rgb64_to_grey8(const uint64_t c){
  double k = 1.0*0xFF/0xFFFF;
  return rint(
    COLOR_LUMINR*((c>>32)&0xFFFF)*k +
    COLOR_LUMING*((c>>16)&0xFFFF)*k +
    COLOR_LUMINB*(c&0xFFFF)*k );
}

// Convert RGB color to 16-bit greyscale
// 0->0, FF,FF,FF -> FFFF
uint16_t color_rgb_to_grey16(const uint32_t c){
  double k = 1.0*0xFFFF/0xFF;
  return rint(
    COLOR_LUMINR*((c>>16)&0xFF)*k +
    COLOR_LUMING*((c>>8)&0xFF)*k +
    COLOR_LUMINB*(c&0xFF)*k);
}

// Convert RGB color to 16-bit greyscale
uint16_t color_rgb64_to_grey16(const uint64_t c){
  return rint(
    COLOR_LUMINR*((c>>32)&0xFFFF) +
    COLOR_LUMING*((c>>16)&0xFFFF) +
    COLOR_LUMINB*(c&0xFFFF) );
}

// Convert RGB color from 64 to 32 bpp.
// We want to keep max/min values:
//    0xFFFF -> 0xFF -> 0xFFFF
//    0x0000 -> 0x00 -> 0x0000
uint32_t color_rgb_64to32(const uint64_t c){
  // AAaaRRrrGGggBBbb
  //         AARRGGBB
  uint32_t a = (c>>48) & 0xFFFF;
  uint32_t r = (c>>32) & 0xFFFF;
  uint32_t g = (c>>16) & 0xFFFF;
  uint32_t b = c & 0xFFFF;
  a = (a*0xFF)/0xFFFF;
  r = (r*0xFF)/0xFFFF;
  g = (g*0xFF)/0xFFFF;
  b = (b*0xFF)/0xFFFF;
  return (a<<24) + (r<<16) + (g<<8) + b;
}

// Convert RGB color from 32 to 64 bpp
uint64_t color_rgb_32to64(const uint32_t c){
  uint64_t a = (c>>24) & 0xFF;
  uint64_t r = (c>>16) & 0xFF;
  uint64_t g = (c>>8) & 0xFF;
  uint64_t b = c & 0xFF;
  a = (a*0xFFFF)/0xFF;
  r = (r*0xFFFF)/0xFF;
  g = (g*0xFFFF)/0xFF;
  b = (b*0xFFFF)/0xFF;
  return (a<<48) + (r<<32) + (g<<16) + b;
}

// Invert RGB color, keep transparency
uint32_t color_rgb_invert(const uint32_t c){
  uint32_t tr=c>>24;
  return   (tr << 24)
         + ((tr - ((c>>16)&0xFF)) << 16)
         + ((tr - (c>>8)&0xFF) << 8)
         + (tr - c&0xFF);
}

// Invert RGB color, keep transparency
uint64_t color_rgb64_invert(const uint64_t c){
  uint64_t tr=c>>48;
  return   (tr << 48)
         + ((tr - ((c>>32)&0xFFFF)) << 32)
         + ((tr - (c>>16)&0xFFFF) << 16)
         + (tr - c&0xFFFF);
}
