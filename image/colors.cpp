#include "colors.h"
#include <cmath>
#include <iomanip>

// distance between two colors
double
color_dist(const uint32_t c1, const uint32_t c2, const bool prescaled){
  int a1 = (c1>>24)&0xFF, a2 = (c2>>24)&0xFF;
  int r1 = (c1>>16)&0xFF, r2 = (c2>>16)&0xFF;
  int g1 = (c1>>8)&0xFF,  g2 = (c2>>8)&0xFF;
  int b1 = c1&0xFF,       b2 = c2&0xFF;

  if (!prescaled || (a1==0xFF && a2==0xFF))
    return sqrt(
      pow( a1-a2, 2) + pow( r1-r2, 2) +
      pow( g1-g2, 2) + pow( b1-b2, 2)
    );

  if (r1>a1 || g1>a1 || b1>a1)
    throw Err() << "color_dist: non-prescaled color: 0x"
                << std::hex << std::setfill('0') << std::setw(8) << c1;
  if (r2>a2 || g2>a2 || b2>a2)
    throw Err() << "color_dist: non-prescaled color: 0x"
                << std::hex << std::setfill('0') << std::setw(8) << c2;

  // note: two almost trransparent colorse can have up to 0xFF distance
  // in each component; we want distance 0xFF between fully transparent and
  // any almost transparent color, and distance 0 between same colors.
  double da = a1-a2;
  double dr = (a1>0 && a2>0) ? r1*255.0/a1 - r2*255.0/a2 : (a1!=0 || a2!=0)? 0xFF:0;
  double dg = (a1>0 && a2>0) ? g1*255.0/a1 - g2*255.0/a2 : (a1!=0 || a2!=0)? 0xFF:0;
  double db = (a1>0 && a2>0) ? b1*255.0/a1 - b2*255.0/a2 : (a1!=0 || a2!=0)? 0xFF:0;

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

// Convert RGB color to 8-bit greyscale
uint8_t color_rgb_to_grey8(const uint32_t c){
  return rint(
    COLOR_LUMINR*((c>>16)&0xFF) +
    COLOR_LUMING*((c>>8)&0xFF) +
    COLOR_LUMINB*(c&0xFF) );
}

// Convert RGB color to 16-bit greyscale
uint16_t color_rgb_to_grey16(const uint32_t c){
  return rint(
    COLOR_LUMINR*((c>>8)&0xFF00) +
    COLOR_LUMING*(c&0xFF00) +
    COLOR_LUMINB*((c<<8)&0xFF00) );
}

