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
    assert_feq(color_dist(0xFFFFFFFF, 0xFFFFFFFe), 1, 1e-6);
    assert_feq(color_dist(0x80808080, 0xFFFFFFFF), 0x80, 1);
    assert_feq(color_dist(0x80808080, 0x80008080), 0xFF, 1);
    assert_feq(color_dist(0x0, 0x87654321), sqrt(0x87*0x87 + 3*0xff*0xff), 1e-6);
    assert_feq(color_dist(0x0, 0x0), 0, 1e-6);
    assert_feq(color_dist(0x01010101, 0x01010101), 0, 1e-6);
    assert_feq(color_dist(0x01010000, 0x01000000), 0xff, 1e-6);

    assert_err(color_dist(0x01020000, 0x01000000), "color_dist: non-prescaled color: 0x01020000");
    assert_err(color_dist(0x01000000, 0x01020000), "color_dist: non-prescaled color: 0x01020000");

    // non-prescaled colors
    assert_feq(color_dist(0xFFFFFFFF, 0xFeFeFeFe, false), 2, 1e-6);
    assert_feq(color_dist(0xFFFFFFFF, 0xFFFFFFFe, false), 1, 1e-6);
    assert_feq(color_dist(0xFeFFFFFF, 0xFFFFFFFF, false), 1, 1e-6);

    assert_err(color_rem_transp(0x80FFFF00, 0), "color_rem_transp: non-prescaled color: 0x80ffff00");
    assert_err(color_rem_transp(0x00FFFF00, 0), "color_rem_transp: non-prescaled color: 0x00ffff00");

    assert_eq(color_rem_transp(0x80808000, 0), 0xFFFFFF00);
    assert_eq(color_rem_transp(0xFF00FFFF, 0), 0xFF00FFFF);
    assert_eq(color_rem_transp(0x0, 0), 0xFF000000);
    assert_eq(color_rem_transp(0x01000000, 0), 0xFF000000);

    assert_err(color_rem_transp(0x80FFFF00, 1), "color_rem_transp: non-prescaled color: 0x80ffff00");
    assert_err(color_rem_transp(0x00FFFF00, 1), "color_rem_transp: non-prescaled color: 0x00ffff00");

    assert_eq(color_rem_transp(0x80808000, 1), 0xFFFFFF00);
    assert_eq(color_rem_transp(0xFF00FFFF, 1), 0xFF00FFFF);
    assert_eq(color_rem_transp(0x0, 1), 0); // the only difference with gif=false
    assert_eq(color_rem_transp(0x01000000, 0), 0xFF000000);

    assert_eq(color_argb(0,1,2,3), 0x00000000);
    assert_eq(color_argb(0xFF,1,2,3), 0xFF010203);
    assert_eq(color_argb(0x80,2,4,6), 0x80010203);

    assert_eq(color_rgb_to_grey8(0xFF101010), 0x10);
    assert_eq(color_rgb_to_grey16(0xFF101010), 0x1000);
    assert_eq(color_rgb_to_grey8(0xFF000010), 0x2);
    assert_eq(color_rgb_to_grey16(0xFF000010), 0x1d5);

    assert_eq(color_rgb_invert(0xFF000000), 0xFFFFFFFF);
    assert_eq(color_rgb_invert(0x80000000), 0x80808080);
    assert_eq(color_rgb_invert(0xFF010101), 0xFFFEFEFE);

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
