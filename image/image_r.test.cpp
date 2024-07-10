///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <sstream>
#include "image_r.h"
#include "err/assert_err.h"

int
main(){
  try{

    // Static functions for reading/writing 24,32,48,64 (a)rgb
    // values to a memory buffer:
    {
       unsigned char buf[8];
       ImageR::set64(buf, 0x1234567812345678ull);
       assert_eq(ImageR::get64(buf), 0x1234567812345678ull);

       ImageR::set48(buf, 0x1234567890123456ull);
       assert_eq(ImageR::get48(buf), 0xFFFF567890123456ull);
       assert_eq(ImageR::get64(buf), 0x1234567890123456ull);

       ImageR::set32(buf, 0x12345678);
       assert_eq(ImageR::get32(buf), 0x12345678);

       ImageR::set24(buf, 0x01234567);
       assert_eq(ImageR::get24(buf), 0xFF234567);
       assert_eq(ImageR::get32(buf), 0x12234567);

       ImageR::set16(buf, 0x1234);
       assert_eq(ImageR::get16(buf), 0x1234);
    }

    {
      assert_err(ImageR im2(100,0,  IMAGE_32ARGB),    "non-positive image dimension: 100x0");
      assert_err(ImageR im2(0,1,    IMAGE_32ARGB),    "non-positive image dimension: 0x1");
      //// arch-specific
      //assert_err(ImageR im2(100,-1, IMAGE_32ARGB),
      //    "ImageR: can't allocate memory for ImageR(100x18446744073709551615, ARGB, 32bpp): std::bad_alloc");
    }

    // empty image
    {
      ImageR im;
      assert_eq(type_to_str(im), "ImageR(empty)");
      assert_eq(im.width(), 0);
      assert_eq(im.height(), 0);
      assert_eq(im.type(), IMAGE_UNKNOWN);
      assert(im.is_empty());
      assert(!im);
    }

    // UNKNOWN type
    {
      ImageR im(100,100, IMAGE_UNKNOWN);
      assert_eq(type_to_str(im), "ImageR(100x100, Unknown data format)");
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_UNKNOWN);
      assert(!im.is_empty());
      assert(im);
//      assert_err(im.get_argb(5,5), "Image::get_argb: unsupported image type");
//      assert_err(im.get_rgb(5,5),  "Image::get_argb: unsupported image type");
      assert_err(im.get_double(5,5), "Image::get_double: unsupported image type");
    }

    { // 32bpp image
      ImageR im(640,480, IMAGE_32ARGB);
      assert_eq(type_to_str(im), "ImageR(640x480, ARGB, 32bpp)");
      uint32_t bg = 0xFE121314;
      im.fill32(bg);
      assert_eq(im.width(), 640);
      assert_eq(im.height(), 480);
      assert_eq(im.type(), IMAGE_32ARGB);
      assert_eq(im.get32(0,0), bg);
      assert_eq(im.get32(639,479), bg);

      im.set32(0,9, 1);
      im.set32(2,9, 5);
      im.set32(2,0, 2);
      assert_eq(im.get32(0,9), 1);
      assert_eq(im.get32(2,9), 5);
      assert_eq(im.get32(2,0), 2);

      assert_eq(im.get_argb(5,5), bg);
      assert_eq(im.get_rgb(5,5),  0xFF121314);
      assert_err(im.get_double(5,5), "Image::get_double: unsupported image type");
    }

    { // 24bpp image
      ImageR im(640,480, IMAGE_24RGB);
      assert_eq(type_to_str(im), "ImageR(640x480, RGB, 24bpp)");
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

      assert_eq(im.get_argb(5,5), 0xFF000010);
      assert_eq(im.get_rgb(5,5),  0xFF000010);
      assert_err(im.get_double(5,5), "Image::get_double: unsupported image type");
    }

    { // 64bpp image
      ImageR im(640,480, IMAGE_64ARGB);
      assert_eq(type_to_str(im), "ImageR(640x480, ARGB, 64bpp)");
      uint64_t bg=0xFEDC000000001012ll;
      im.fill64(bg);
      assert_eq(im.width(), 640);
      assert_eq(im.height(), 480);
      assert_eq(im.type(), IMAGE_64ARGB);
      assert_eq(im.get64(0,0), bg);
      assert_eq(im.get64(639,479), bg);

      im.set64(0,9, 1);
      im.set64(2,9, 5);
      im.set64(2,0, 2);
      assert_eq(im.get64(0,9), 1);
      assert_eq(im.get64(2,9), 5);
      assert_eq(im.get64(2,0), 2);

      assert_eq(im.get_argb(5,5), 0xFE000010);
      assert_eq(im.get_rgb(5,5),  0xFF000010);
      assert_eq(im.get_argb64(5,5), bg);
      assert_eq(im.get_rgb64(5,5),  0xFFFF000000001024ll); // scaling!
      assert_err(im.get_double(5,5), "Image::get_double: unsupported image type");
    }

    { // 48bpp image
      ImageR im(640,480, IMAGE_48RGB);
      assert_eq(type_to_str(im), "ImageR(640x480, RGB, 48bpp)");
      uint64_t bg=0x1012;
      im.fill48((0xFEDCll<<48) + bg);
      assert_eq(im.width(), 640);
      assert_eq(im.height(), 480);
      assert_eq(im.type(), IMAGE_48RGB);
      assert_eq(im.get48(0,0), (0xFFFFll<<48) + bg);
      assert_eq(im.get48(639,479), (0xFFFFll<<48) + bg);

      im.set48(0,9, 1);
      im.set48(2,9, 5);
      im.set48(2,0, 2);
      assert_eq(im.get48(0,9), (0xFFFFll<<48) + 1);
      assert_eq(im.get48(2,9), (0xFFFFll<<48) + 5);
      assert_eq(im.get48(2,0), (0xFFFFll<<48) + 2);

      assert_eq(im.get_argb(5,5), 0xFF000010);
      assert_eq(im.get_rgb(5,5),  0xFF000010);
      assert_eq(im.get_argb64(5,5), 0xFFFF000000001012ll);
      assert_eq(im.get_rgb64(5,5),  0xFFFF000000001012ll);
      assert_err(im.get_double(5,5), "Image::get_double: unsupported image type");
    }


    { // 8bpp image
      ImageR im(100,100, IMAGE_8);
      assert_eq(type_to_str(im), "ImageR(100x100, Grey, 8bpp)");
      im.fill8(0x11);
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_8);
      assert_eq(im.get8(0,0), 0x11);
      assert_eq(im.get8(99,99), 0x11);

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

      assert_eq(im.get_argb(5,5), 0xFF111111);
      assert_eq(im.get_rgb(5,5),  0xFF111111);
      assert_feq(im.get_double(2,9), 5.0, 1e-6);
    }

    { // 16bpp image
      ImageR im(100,100, IMAGE_16);
      assert_eq(type_to_str(im), "ImageR(100x100, Grey, 16bpp)");
      im.fill16(0x1123);
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_16);
      assert_eq(im.get16(0,0), 0x1123);
      assert_eq(im.get16(99,99), 0x1123);

      im.set16(0,9, 1);
      im.set16(2,9, 350);
      im.set16(2,0, 256*256-1);
      im.set16(2,2, -1);
      im.set16(2,3, -2);
      assert_eq(im.get16(0,9), 1);
      assert_eq(im.get16(2,9), 350);
      assert_eq(im.get16(2,0), 256*256-1);
      assert_eq(im.get16(2,2), 256*256-1);
      assert_eq(im.get16(2,3), 256*256-2);

      assert_eq(im.get_argb(5,5), 0xFF111111);
      assert_eq(im.get_rgb(5,5),  0xFF111111);
      assert_feq(im.get_double(2,9), 350.0, 1e-6);
    }

    { // 1bpp image, w*h % 8 = 0
      ImageR im(256,128, IMAGE_1);
      assert_eq(type_to_str(im), "ImageR(256x128, B/W, 1bpp)");
      im.fill1(1);
      assert_eq(im.width(), 256);
      assert_eq(im.height(), 128);
      assert_eq(im.type(), IMAGE_1);
      assert_eq(im.get1(0,0), 1);
      assert_eq(im.get1(99,99), 1);
      assert_eq(im.dsize(), 128*256/8);
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

      assert_eq(im.get_argb(5,5), 0xFFFFFFFF);
      assert_eq(im.get_rgb(5,5),  0xFFFFFFFF);
      assert_eq(im.get_argb(0,9), 0xFF000000);
      assert_eq(im.get_rgb(0,9),  0xFF000000);
      assert_feq(im.get_double(2,9), 1, 1e-6);
    }

    { // 1bpp image, w*h % 8 != 0
      ImageR im(99,101, IMAGE_1);
      assert_eq(type_to_str(im), "ImageR(99x101, B/W, 1bpp)");
      im.fill1(1);
      assert_eq(im.width(), 99);
      assert_eq(im.height(), 101);
      assert_eq(im.type(), IMAGE_1);
      assert_eq(im.get1(0,0), 1);
      assert_eq(im.get1(98,100), 1);
      assert_eq(im.dsize(), 1313);
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

      assert_feq(im.get_double(2,9), 1, 1e-6);
    }

    { // double image
      ImageR im(100,100, IMAGE_DOUBLE);
      assert_eq(type_to_str(im), "ImageR(100x100, double)");
      im.fillD(0.123);
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_DOUBLE);
      assert_feq(im.getD(0,0), 0.123, 1e-8);
      assert_feq(im.getD(99,99), 0.123, 1e-8);

      im.setD(0,1, 1);
      im.setD(0,2, 1e-8);

      assert_feq(im.getD(0,1), 1, 1e-8);
      assert_feq(im.getD(0,2), 1e-8, 1e-8);

      assert_feq(im.get_double(0,1), 1, 1e-6);
    }

    { // float image
      ImageR im(100,100, IMAGE_FLOAT);
      assert_eq(type_to_str(im), "ImageR(100x100, float)");
      im.fillF(0.123);
      assert_eq(im.width(), 100);
      assert_eq(im.height(), 100);
      assert_eq(im.type(), IMAGE_FLOAT);
      assert_feq(im.getF(0,0), 0.123, 1e-8);
      assert_feq(im.getF(99,99), 0.123, 1e-8);

      im.setF(0,1, 1);
      im.setF(0,2, 1e-8);

      assert_feq(im.getF(0,1), 1, 1e-8);
      assert_feq(im.getF(0,2), 1e-8, 1e-8);

      assert_feq(im.get_double(0,1), 1, 1e-6);
    }


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
