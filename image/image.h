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

    // Get color for a given point.
    // To be redefined. Should work for any x,y values.
    // Note that there is no `const` modifier.
    virtual uint32_t get_color(const int x, const int y) {
      return bgcolor;
    }

    // Check if coordinates are valid
    virtual bool check_crd(const int x, const int y) const {return true;}

    // Check if coordinate range is valid.
    // x1<=x2, y1<=y2
    virtual bool check_rng(const int x1, const int y1,
                           const int x2, const int y2) const {return true;}

    // set background color
    void set_bgcolor(const uint32_t c) {
      bgcolor = c;
    }

    // get background color
    uint32_t get_bgcolor() {
      return bgcolor;
    }

};

#endif
