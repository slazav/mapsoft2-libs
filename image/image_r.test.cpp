///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <sstream>
#include "image_r.h"
#include "err/assert_err.h"

int
main(){
  try{
    {
      ImageR im1;
      assert_eq(im1.width(), 0);
      assert_eq(im1.height(), 0);
      assert_eq(im1.type(), IMAGE_UNKNOWN);
      assert(im1.is_empty());
      assert(!im1);

      ImageR im2(100,100, IMAGE_32ARGB);

      assert_eq(type_to_str(im1), "ImageR(empty)");

      assert_eq(type_to_str(im2), "ImageR(100x100, ARGB, 32bpp)");

      assert_eq(im2.width(), 100);
      assert_eq(im2.height(), 100);
      assert_eq(im2.type(), IMAGE_32ARGB);
      assert(!im2.is_empty());
      assert(im2);
    }

    {
      assert_err(ImageR im2(100,0,  IMAGE_32ARGB),    "non-positive image dimension: 100x0");
      assert_err(ImageR im2(0,1,    IMAGE_32ARGB),    "non-positive image dimension: 0x1");
      //// arch-specific
      //assert_err(ImageR im2(100,-1, IMAGE_32ARGB),
      //    "ImageR: can't allocate memory for ImageR(100x18446744073709551615, ARGB, 32bpp): std::bad_alloc");
    }



    { // 32bpp image
      ImageR im(640,480, IMAGE_32ARGB);
      im.fill32(0xFF000010);
      assert_eq(im.width(), 640);
      assert_eq(im.height(), 480);
      assert_eq(im.type(), IMAGE_32ARGB);
      assert_eq(im.get32(0,0), 0xFF000010);
      assert_eq(im.get32(639,479), 0xFF000010);

      im.set32(0,9, 1);
      im.set32(2,9, 5);
      im.set32(2,0, 2);
      assert_eq(im.get32(0,9), 1);
      assert_eq(im.get32(2,9), 5);
      assert_eq(im.get32(2,0), 2);
      assert_eq(im.get32(0,0), 0xFF000010);
      assert_eq(im.get32(639,479), 0xFF000010);
    }

    { // 24bpp image
      ImageR im(640,480, IMAGE_24RGB);
      im.fill24(0x10);
      assert_eq(im.width(), 640);
      assert_eq(im.height(), 480);
      assert_eq(im.type(), IMAGE_24RGB);
      assert_eq(im.get24(0,0), 0xFF000010);
      assert_eq(im.get24(639,479), 0xFF000010);

      im.set24(0,9, 1);
      im.set24(2,9, 5);
      im.set24(2,0, 2);
      assert_eq(im.get24(0,9), 0xFF000001);
      assert_eq(im.get24(2,9), 0xFF000005);
      assert_eq(im.get24(2,0), 0xFF000002);
      assert_eq(im.get24(0,0), 0xFF000010);
      assert_eq(im.get24(639,479), 0xFF000010);
    }

    { // 8bpp image
      ImageR im(100,100, IMAGE_8);
      im.fill8(11);
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_8);
      assert_eq(im.get8(0,0), 11);
      assert_eq(im.get8(99,99), 11);

      im.set8(0,9, 1);
      im.set8(2,9, 5);
      im.set8(2,0, 255);
      im.set8(2,2, -1);
      im.set8(2,3, -2);
      assert_eq(im.get8(0,9), 1);
      assert_eq(im.get8(2,9), 5);
      assert_eq(im.get8(2,0), 255);
      assert_eq(im.get8(2,2), 255);
      assert_eq(im.get8(2,3), 254);
    }

    { // 1bpp image, w*h % 8 = 0
      ImageR im(100,100, IMAGE_1);
      im.fill1(1);
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_1);
      assert_eq(im.get1(0,0), 1);
      assert_eq(im.get1(99,99), 1);
      assert_eq(im.dsize(), 1250);
      im.fill1(0);
      assert_eq(im.get1(0,0), 0);
      assert_eq(im.get1(99,99), 0);

      im.set1(0,9, 1);
      im.set1(2,9, 5);
      im.set1(2,0, 2);
      im.set1(2,1, 0);
      im.set1(99,99, 1);
      assert_eq(im.get1(0,9), 1);
      assert_eq(im.get1(2,9), 1);
      assert_eq(im.get1(2,0), 1);
      assert_eq(im.get1(2,1), 0);
      assert_eq(im.get1(99,99), 1);
    }

    { // 1bpp image, w*h % 8 != 0
      ImageR im(99,101, IMAGE_1);
      im.fill1(1);
      assert_eq(im.width(), 99);
      assert_eq(im.height(), 101);
      assert_eq(im.type(), IMAGE_1);
      assert_eq(im.get1(0,0), 1);
      assert_eq(im.get1(98,100), 1);
      assert_eq(im.dsize(), 1250);
      im.fill1(0);
      assert_eq(im.get1(0,0), 0);
      assert_eq(im.get1(98,100), 0);

      im.set1(0,9, 1);
      im.set1(2,9, 5);
      im.set1(2,0, 2);
      im.set1(2,1, 0);
      im.set1(98,100, 1);
      assert_eq(im.get1(0,9), 1);
      assert_eq(im.get1(2,9), 1);
      assert_eq(im.get1(2,0), 1);
      assert_eq(im.get1(2,1), 0);
      assert_eq(im.get1(98,100), 1);
    }

    { // double image
      ImageR im(100,100, IMAGE_DOUBLE);
      im.fillD(0.123);
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_DOUBLE);
      assert_eq(im.getD(0,0), 0.123);
      assert_eq(im.getD(99,99), 0.123);

      im.setD(0,1, 1);
      im.setD(0,2, 1e-8);

      assert_eq(im.getD(0,1), 1);
      assert_eq(im.getD(0,2), 1e-8);
    }



  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
