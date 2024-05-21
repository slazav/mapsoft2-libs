///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <fstream>
#include "io_pnm.h"
#include "err/assert_err.h"
#include "image_colors.h"

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

    // RGBA image
    ImageR img32(256,128, IMAGE_32ARGB);
    for (size_t y=0; y<128; ++y){
      for (size_t x=0; x<128; ++x){
        img32.set32(x,y,     color_argb(0xFF, 2*x, 2*y, 0));
        img32.set32(128+x,y, color_argb(2*x,  2*y, 0,   0));
      }
    }

    ImageR img64(256,128, IMAGE_64ARGB);
    for (size_t y=0; y<128; ++y){
      for (size_t x=0; x<128; ++x){
        img64.set64(x,y,     color_argb64(0xFFFF, 400*x, 400*y, 0));
        img64.set64(128+x,y, color_argb64(400*x,  400*y, 0,   0));
      }
    }

    /*********************************************/
    { // IMAGE_48RGB, IMAGE_64RGBA

      ImageR img(256,128, IMAGE_48RGB);
      for (size_t y=0; y<img.height(); ++y)
        for (size_t x=0; x<img.width(); ++x)
          img.set48(x,y, img64.get64(x,y));


      image_save_pnm(img64, "test_pnm/img_64.pnm");
      assert_eq(image_size_pnm("test_pnm/img_64.pnm"), iPoint(256,128));
      ImageR img1 = image_load_pnm("test_pnm/img_64.pnm", 1);
image_save_pnm(img1, "test_pnm/img_64_.pnm");
      assert_eq(img1.type(), IMAGE_48RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);

      for (size_t x=0; x<img.width(); x+=8){
        for (size_t y=0; y<img.height(); y+=8){
          assert_eq(img1.get_rgb(x,y), img.get_rgb(x,y));
        }
      }

      image_save_pnm(img, "test_pnm/img_48.pnm");
      assert_eq(image_size_pnm("test_pnm/img_48.pnm"), iPoint(256,128));
      img1 = image_load_pnm("test_pnm/img_48.pnm", 1);
      assert_eq(img1.type(), IMAGE_48RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_rgb(x,y), img.get_rgb(x,y));

    }


    /*********************************************/
    { // IMAGE_24RGB, IMAGE_32RGBA
      ImageR img(256,128, IMAGE_24RGB);
      for (size_t y=0; y<img.height(); ++y)
        for (size_t x=0; x<img.width(); ++x)
          img.set24(x,y, img32.get_rgb(x,y));


      image_save_pnm(img32, "test_pnm/img_32.pnm");
      assert_eq(image_size_pnm("test_pnm/img_32.pnm"), iPoint(256,128));
      ImageR img1 = image_load_pnm("test_pnm/img_32.pnm", 1);
      assert_eq(img1.type(), IMAGE_24RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);

      for (size_t x=0; x<img.width(); x+=8){
        for (size_t y=0; y<img.height(); y+=8){
          assert_eq(img1.get_rgb(x,y), img.get_rgb(x,y));
        }
      }

      image_save_pnm(img, "test_pnm/img_24.pnm");
      assert_eq(image_size_pnm("test_pnm/img_24.pnm"), iPoint(256,128));
      img1 = image_load_pnm("test_pnm/img_24.pnm", 1);
      assert_eq(img1.type(), IMAGE_24RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img.width(); x+=8)
        for (size_t y=0; y<img.height(); y+=8)
          assert_eq(img1.get_rgb(x,y), img.get_rgb(x,y));

    }

    /*********************************************/
    { // IMAGE_16
      ImageR img(256,128, IMAGE_16);
      for (size_t y=0; y<img.height(); ++y)
        for (size_t x=0; x<img.width(); ++x)
          img.set16(x,y, color_rgb_to_grey16(img32.get_rgb(x,y)));

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
      ImageR img(256,128, IMAGE_8);
      for (size_t y=0; y<img.height(); ++y)
        for (size_t x=0; x<img.width(); ++x)
          img.set8(x,y, color_rgb_to_grey8(img32.get_rgb(x,y)));

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
     ImageR img(256,128, IMAGE_1);
      for (size_t y=0; y<img.height(); ++y){
        for (size_t x=0; x<img.width(); ++x){
          img.set1(x,y, 1-(int)fabs(600*sin(2*M_PI*x/255)*sin(2*M_PI*y/255))%2);
        }
      }

      image_save_pnm(img, "test_pnm/img_01.pnm");
      ImageR img1 = image_load_pnm("test_pnm/img_01.pnm", 1);
      assert_eq(img1.type(), IMAGE_1);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
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
      std::ifstream str("test_pnm/img_24.pnm");
      assert_err(image_load_pnm(str, 0),
        "image_load_pnm: wrong scale: 0");
      ImageR img1 = image_load_pnm(str, 1);
      assert_eq(img1.type(), IMAGE_24RGB);
      assert_eq(img1.width(), 256);
      assert_eq(img1.height(), 128);
      for (size_t x=0; x<img1.width(); x+=8)
        for (size_t y=0; y<img1.height(); y+=8)
          assert_eq(img1.get_argb(x,y), img32.get_rgb(x,y));

    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond