///\cond HIDDEN (do not show this in Doxyden)

#include "err/assert_err.h"
#include <iostream>
#include <iomanip>
#include "rainbow.h"


int
main(){

/************************************************/
// user-defined rainbow data


  {
    // empty data -> always return 0
    std::vector<rainbow_data> RD;
    Rainbow R(RD);
    assert_eq(R.get(1.2), 0);
    assert_eq(R.get(0.5), 0);
  }

  {
    std::vector<rainbow_data> RD ={
      {0.1, 0x000000},
      {0.5, 0xFF0000}, // 0.1 - 0.5 black -> blue
      {0.5, 0xFF00FF}, // - color step
      {0.9, 0x000000} // 0.5 - 0.9 magenta -> black
    };
    Rainbow R(RD);

    assert_eq(R.get(0.0), 0u);
    assert_eq(R.get(0.2), 0x400000u);
    assert_eq(R.get(0.4999), 0xFF0000u);
    assert_eq(R.get(0.5000), 0xFF0000u);
    assert_eq(R.get(0.5001), 0xFF00FFu);
    assert_eq(R.get(0.9), 0u);
    assert_eq(R.get(1.0), 0u);

    R.set_limits(0x111111, -1);
    assert_eq(R.get(0.0), 0x111111u);
    assert_eq(R.get(1.0), 0u);

    R.set_limits(0x111111, 0x222222u);
    assert_eq(R.get(0.0), 0x111111u);
    assert_eq(R.get(1.0), 0x222222u);
  }

/************************************************/
// same tests but opposite sorting of the data + alpha
  {
    std::vector<rainbow_data> RD = {
      {0.9, 0x00000000u},
      {0.5, 0xFFFF00FFu},
      {0.5, 0xFFFF0000u},
      {0.1, 0xFF000000u}
    };

    Rainbow R(RD);

    assert_eq(R.get(0.0), 0xFF000000u);
    assert_eq(R.get(0.2), 0xFF400000u);
    assert_eq(R.get(0.4999), 0xFFFF0000u);
    // the step point is different!
    assert_eq(R.get(0.5000), 0xFFFF00FFu);
    assert_eq(R.get(0.5001), 0xFFFF00FFu);
    assert_eq(R.get(0.7),    0x80800080u);
    assert_eq(R.get(0.9), 0u);
    assert_eq(R.get(1.0), 0u);

    R.set_limits(0x111111, -1);
    assert_eq(R.get(0.0), 0x111111u);
    assert_eq(R.get(1.0), 0u);

    R.set_limits(0x111111, 0x222222u);
    assert_eq(R.get(0.0), 0x111111u);
    assert_eq(R.get(1.0), 0x222222u);
  }

/************************************************/
// two-color gradient and limits
  {
    Rainbow R(1, 2, 0x00, 0x4040u);
    assert_eq(R.get(0.5), 0x0000u);
    assert_eq(R.get(1), 0x0000u);
    assert_eq(R.get(1.5), 0x2020u);
    assert_eq(R.get(2), 0x4040u);
    assert_eq(R.get(2.5), 0x4040u);

    R.set_limits(1, 2);
    assert_eq(R.get(0.5), 1u);
    assert_eq(R.get(1), 0u);
    assert_eq(R.get(1.5), 0x2020u);
    assert_eq(R.get(2), 0x4040u);
    assert_eq(R.get(2.5), 2u);
  }

  // same, inversed order
  {
    Rainbow R(2, 1, 0x4040u, 0x0000u);
    assert_eq(R.get(0.5), 0x0000u);
    assert_eq(R.get(1),   0x0000u);
    assert_eq(R.get(1.5), 0x2020u);
    assert_eq(R.get(2),   0x4040u);
    assert_eq(R.get(2.5), 0x4040u);

    R.set_limits(1, 2);
    assert_eq(R.get(0.5), 1u);
    assert_eq(R.get(1),   0u);
    assert_eq(R.get(1.5), 0x2020u);
    assert_eq(R.get(2),   0x4040u);
    assert_eq(R.get(2.5), 2u);
  }

  // normal rainbow b-c-g-y-r-m
  {
    Rainbow R(0, 5, RAINBOW_NORMAL);
    assert_eq(R.get(0.0), 0xff0000ffu);
    assert_eq(R.get(0.5), 0xff0080ffu);
    assert_eq(R.get(1.0), 0xff00ffffu);
    assert_eq(R.get(1.5), 0xff00ff7fu);
    assert_eq(R.get(2.0), 0xff00ff00u);
    assert_eq(R.get(2.5), 0xff80ff00u);
    assert_eq(R.get(3.0), 0xffffff00u);
    assert_eq(R.get(3.5), 0xffff7f00u);
    assert_eq(R.get(4.0), 0xffff0000u);
    assert_eq(R.get(4.5), 0xffff0080u);
    assert_eq(R.get(5.0), 0xffff00ffu);
  }

  {
    Rainbow R(0, 5, RAINBOW_BURNING);
    assert_eq(R.get(0), 0xffffffffu);
    assert_eq(R.get(1), 0xffffff00u);
    assert_eq(R.get(2), 0xffff0000u);
    assert_eq(R.get(3), 0xffff00ffu);
    assert_eq(R.get(4), 0xff0000ffu);
    assert_eq(R.get(5), 0xff000040u);
  }

  {
    Rainbow R(0, 3, RAINBOW_BURNING1);
    assert_eq(R.get(0), 0xff000000u);
    assert_eq(R.get(1), 0xffff0000u);
    assert_eq(R.get(2), 0xffffff00u);
    assert_eq(R.get(3), 0xffffffffu);
  }

  {
    //opposite direction
    Rainbow R(3, 0, RAINBOW_BURNING1);
    assert_eq(R.get(3), 0xff000000u);
    assert_eq(R.get(2), 0xffff0000u);
    assert_eq(R.get(1), 0xffffff00u);
    assert_eq(R.get(0), 0xffffffffu);
  }

  {
    //string
    Rainbow R(0, 14, "RrGgBbCcMmYyKWw");
    assert_eq(R.get(0),  0xffff0000u);
    assert_eq(R.get(1),  0xff400000u);
    assert_eq(R.get(2),  0xff00ff00u);
    assert_eq(R.get(3),  0xff004000u);
    assert_eq(R.get(4),  0xff0000ffu);
    assert_eq(R.get(5),  0xff000040u);
    assert_eq(R.get(6),  0xff00ffffu);
    assert_eq(R.get(7),  0xff004040u);
    assert_eq(R.get(8),  0xffff00ffu);
    assert_eq(R.get(9),  0xff400040u);
    assert_eq(R.get(10), 0xffffff00u);
    assert_eq(R.get(11), 0xff404000u);
    assert_eq(R.get(12), 0xff000000u);
    assert_eq(R.get(13), 0xffffffffu);
    assert_eq(R.get(14), 0xff404040u);
  }

  {
    //empty constructor
    Rainbow R;
    assert_eq(R.get(-10), 0);
    assert_eq(R.get(0),   0);
    assert_eq(R.get(10),  0);
  }


  // color shade
  {
    assert_eq(color_shade(0xFFFFFF, 0),   0x000000u);
    assert_eq(color_shade(0xFFFFFF, 0.5), 0x808080u);
    assert_eq(color_shade(0xFFFFFF, 1),   0xffffffu);

    assert_eq(color_shade(0xFFFFFFFF, 0),   0xFF000000u);
    assert_eq(color_shade(0xFFFFFFFF, 0.5), 0xFF808080u);
    assert_eq(color_shade(0xFFFFFFFF, 1),   0xFFffffffu);
  }

  return 0;
}

///\endcond
