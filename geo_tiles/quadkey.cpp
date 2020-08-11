#include "quadkey.h"
#include <string>

// see https://github.com/linz/basemaps/blob/master/packages/geo/src/quad.key.ts

void
quadkey_to_tile(const std::string & q, int &x, int &y, int &z){
  x = y = 0;
  z = q.size();
  for (int i = z; i>0; i--) {
    int mask = 1 << (i - 1);
    switch (q[z-i]){
      case '0': break;
      case '1': x |= mask; break;
      case '2': y |= mask; break;
      case '3': x |= mask;
                y |= mask;
                break;
      default:
        throw Err() << "bad quadkey: " << q;
    }
  }
}

std::string
tile_to_quadkey(const int x, const int y, const int z){
  std::string q;
  for (int i=z; i>0; i--) {
    char c = '0';
    int mask = 1 << (i-1);
    if (x & mask) c+=1;
    if (y & mask) c+=2;
    q += c;
  }
  return q;
}

