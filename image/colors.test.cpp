///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <sstream>
#include "colors.h"
#include "err/assert_err.h"

int
main(){
  try{

    // color handling functions
    assert_feq(color_dist(0xFFFFFFFF, 0xFEFEFEFE), 2, 1e-6);
    assert_feq(color_dist(0xFFFFFFFF, 0xFFFFFFFE), 1, 1e-6);
    assert_feq(color_dist(0xFEFFFFFF, 0xFFFFFFFF), 1, 1e-6);

    assert_eq(color_rem_transp(0x80FFFF00, 0), 0xFFFFFF00);
    assert_eq(color_rem_transp(0x80808000, 0), 0xFFFFFF00);
    assert_eq(color_rem_transp(0x00FF00FF, 0), 0xFFFFFFFF);
    assert_eq(color_rem_transp(0xFF00FFFF, 0), 0xFF00FFFF);

    assert_eq(color_rem_transp(0x80FFFF00, 1), 0xFFFFFF00);
    assert_eq(color_rem_transp(0x80808000, 1), 0xFFFFFF00);
    assert_eq(color_rem_transp(0x00FF00FF, 1), 0x00000000);
    assert_eq(color_rem_transp(0xFF00FFFF, 1), 0xFF00FFFF);

    assert_eq(color_argb(0,1,2,3), 0x00000000);
    assert_eq(color_argb(0xFF,1,2,3), 0xFF010203);
    assert_eq(color_argb(0x80,2,4,6), 0x80010203);

    assert_eq(color_rgb_to_grey8(0xFF101010), 0x10);
    assert_eq(color_rgb_to_grey16(0xFF101010), 0x1000);
    assert_eq(color_rgb_to_grey8(0xFF000010), 0x2);
    assert_eq(color_rgb_to_grey16(0xFF000010), 0x1d5);
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
