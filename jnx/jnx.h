#ifndef JNX_H
#define JNX_H

#include <cstdint>
#include "err/err.h"

int32_t deg2jnx(const double v) {return (int32_t)rint(v / 180.0 * 0x7fffffff);}

double jnx2deg(const int32_t v) {return  180*(double)v/0x7fffffff;}

#define JNX_HEADER_SIZE      48
#define JNX_LEVEL_INFO_SIZE  12
#define JNX_TILE_INFO_SIZE   28

struct __attribute__ ((packed)) jnx_header_t {
  int32_t  ver;     // version 3
  int32_t  dev_id;  // device id
  int32_t  N,E,S,W; // bounds
  int32_t  nl;      // number of zoom levels
  int32_t  exp;     // expiration date
  int32_t  pr_id;   // product ID (1)
  int32_t  crc;     // tiles CRC32
  int32_t  sver;    // signature version
  uint32_t soff;    // signature offset

  jnx_header_t():
      ver(3), dev_id(0), exp(0), pr_id(0),
      crc(0), sver(0), soff(0),
      nl(0), W(0), S(0), E(0), N(0) {
  }
};

struct __attribute__ ((packed)) jnx_level_info_t {
  int32_t  ntiles;  // number of tiles
  uint32_t offset;  // offset of tile descriptors
  int32_t  scale;   // scale ??

  jnx_level_info_t():
      ntiles(0), offset(0), scale(0) {
  }
};

struct __attribute__ ((packed)) jnx_tile_info_t {
  int32_t  N,E,S,W; // bounds
  int16_t  image_w, image_h;
  int32_t  data_size;
  uint32_t data_offs;

  jnx_tile_info_t():
      W(0), S(0), E(0), N(0),
      image_w(256), image_h(256), data_size(0), data_offs(0) {
  }
};


#endif