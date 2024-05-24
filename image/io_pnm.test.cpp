///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <fstream>
#include "io_pnm.h"
#include "err/assert_err.h"
#include "image_colors.h"
#include "image_test.h"

int
main(){
  try{

    // size
    assert_err(image_size_pnm("test_pnm/missing"),
      "Can't open file: test_pnm/missing");
    assert_err(image_size_pnm("test_pnm/Readme.md"),
      "bad magic number, not a PNM file: 456d: test_pnm/Readme.md");

    // load
    assert_err(image_load_pnm("test_pnm/missing"),
      "Can't open file: test_pnm/missing");
    assert_err(image_load_pnm("test_pnm/Readme.md"),
      "bad magic number, not a PNM file: 456d: test_pnm/Readme.md");

    /*********************************************/

    // Work with PNM format is defferent from PNG and TIFF:
    // There we can say which format (gray, palette, rgb, argb)
    // do we want, the library fill convert the image if needed
    // (e.g by reducing number of colors). Here we just give an
    // image and save it in a proper form (rgb24, rgb48, grey8, grey16, 1bpp).
    // This is because PNM is considered as a lossless intermediate format,
    // final converted images will be saved somewhere else.

    /*********************************************/
    { // IMAGE_64RGBA -> 48RGB
      ImageR img = mk_test_64();
      image_save_pnm(img, "test_pnm/img_64.pnm");
      assert_eq(image_size_pnm("test_pnm/img_64.pnm"), iPoint(256,128));
      ImageR img1 = image_load_pnm("test_pnm/img_64.pnm", 1);
      assert_eq(img1.type(), IMAGE_48RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);

      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get48(x,y), color_rem_transp64(img.get64(x,y), false));
    }

    /*********************************************/
    { // IMAGE_48RGB
      ImageR img = mk_test_48();
      image_save_pnm(img, "test_pnm/img_48.pnm");
      assert_eq(image_size_pnm("test_pnm/img_48.pnm"), iPoint(256,128));
      ImageR img1 = image_load_pnm("test_pnm/img_48.pnm", 1);
      assert_eq(img1.type(), IMAGE_48RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_rgb(x,y), img.get_rgb(x,y));

    }
    /*********************************************/
    { // IMAGE_36ARGB -> 24RGB
      ImageR img = mk_test_32();
      image_save_pnm(img, "test_pnm/img_32.pnm");
      assert_eq(image_size_pnm("test_pnm/img_32.pnm"), iPoint(256,128));
      ImageR img1 = image_load_pnm("test_pnm/img_32.pnm", 1);
      assert_eq(img1.type(), IMAGE_24RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_rgb(x,y), img.get_rgb(x,y));
    }

    /*********************************************/
    { // IMAGE_24RGB
      ImageR img = mk_test_24();
      image_save_pnm(img, "test_pnm/img_24.pnm");
      assert_eq(image_size_pnm("test_pnm/img_24.pnm"), iPoint(256,128));
      ImageR img1 = image_load_pnm("test_pnm/img_24.pnm", 1);
      assert_eq(img1.type(), IMAGE_24RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_rgb(x,y), img.get_rgb(x,y));
    }

    /*********************************************/
    { // IMAGE_16
      ImageR img = mk_test_16();
      image_save_pnm(img, "test_pnm/img_16.pnm");
      ImageR img1 = image_load_pnm("test_pnm/img_16.pnm", 1);
      assert_eq(img1.type(), IMAGE_16);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_argb(x,y), img.get_argb(x,y));
    }

    /*********************************************/
    { // IMAGE_8
      ImageR img = mk_test_8();
      image_save_pnm(img, "test_pnm/img_08.pnm");
      ImageR img1 = image_load_pnm("test_pnm/img_08.pnm", 1);
      assert_eq(img1.type(), IMAGE_8);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_argb(x,y), img.get_argb(x,y));
    }

    /*********************************************/
    { // IMAGE_1
      ImageR img = mk_test_1();
      image_save_pnm(img, "test_pnm/img_01.pnm");
      ImageR img1 = image_load_pnm("test_pnm/img_01.pnm", 1);
      assert_eq(img1.type(), IMAGE_1);
      assert_eq(img1.width(), 250);
      assert_eq(img1.height(), 125);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_argb(x,y), img.get_argb(x,y));
    }

    /*********************************************/
    { //scale tests
      ImageR I0 = image_load_pnm("test_pnm/img_24.pnm", 1);
      for (double sc=1.1; sc<10; sc+=0.8){
        ImageR I1 = image_load_pnm("test_pnm/img_24.pnm", sc);
        assert_eq(I1.width(), floor((I0.width()-1)/sc+1));
        assert_eq(I1.height(), floor((I0.height()-1)/sc+1));
        Point<size_t> pt(101,32);
        Point<size_t> pt1(pt.x/sc,pt.y/sc);
        assert_eq(I1.get_rgb(pt1.x, pt1.y), I0.get_rgb(rint(pt1.x*sc), rint(pt1.y*sc)));

        pt.x = I0.width()-1;   pt.y = I0.height()-1;
        pt1.x = pt.x/sc; pt1.y = pt.y/sc;
        assert(pt.x/sc < I1.width());
        assert(pt.y/sc < I1.height());
        assert_eq(I1.get_rgb(pt1.x, pt1.y), I0.get_rgb(rint(pt1.x*sc), rint(pt1.y*sc)));
      }
      assert_err(image_load_pnm("test_pnm/img_24.pnm", 0),
        "image_load_pnm: wrong scale: 0: test_pnm/img_24.pnm");
    }

    { //load from std::istream
      ImageR img1 = image_load_pnm("test_pnm/img_24.pnm", 1);
      std::ifstream str("test_pnm/img_24.pnm");
      assert_err(image_load_pnm(str, 0),
        "image_load_pnm: wrong scale: 0");
      ImageR img2 = image_load_pnm(str, 1);
      assert_eq(img2.type(), img1.type());
      assert_eq(img2.width(), img1.width());
      assert_eq(img2.height(), img1.height());
      for (size_t x=0; x<img1.width(); x+=8)
        for (size_t y=0; y<img1.height(); y+=8)
          assert_eq(img1.get_argb(x,y), img2.get_rgb(x,y));

    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond