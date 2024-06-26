///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <fstream>
#include "err/assert_err.h"
#include "io_jpeg.h"
#include "image_colors.h"
#include "image_test.h"

int
main(){
  try{

    // size
    assert_err(image_size_jpeg("test_jpeg/missing"),
      "Can't open file: test_jpeg/missing");
    assert_err(image_size_jpeg("test_jpeg/Readme.md"),
      "image_size_jpeg: Not a JPEG file: starts with 0x45 0x6d: test_jpeg/Readme.md");

    // load
    assert_err(image_load_jpeg("test_jpeg/missing"),
      "Can't open file: test_jpeg/missing");
    assert_err(image_load_jpeg("test_jpeg/Readme.md"),
      "image_load_jpeg: Not a JPEG file: starts with 0x45 0x6d: test_jpeg/Readme.md");

    /*********************************************/
    // * Create all types of images (32ARGB, 24RGB, 16, 8, 1, PAL).
    // * Save them with different image_save_jpeg() options.
    // * Read saved file and check result.

    { // IMAGE_32ARGB
      ImageR img = mk_test_32();
      image_save_jpeg(img, "test_jpeg/img_32_def.jpg");
      assert_eq(image_size_jpeg("test_jpeg/img_32_def.jpg"), iPoint(256,128));
      ImageR I = image_load_jpeg("test_jpeg/img_32_def.jpg", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      // check far from edges (smaller jpeg artifacts)
      assert(color_dist(I.get_argb(10,10),   0xff141400) < 5);
      assert(color_dist(I.get_argb(117,117), 0xffeaea00) < 5);
      assert(color_dist(I.get_argb(138,10),  0xFF090000) < 5);
      assert(color_dist(I.get_argb(245,117), 0xffE90000) < 5);
      assert(color_dist(I.get_argb(64,64),   0xFF808000) < 5);
      assert(color_dist(I.get_argb(192,64),  0xFF800000) < 5);

      Opt o;
      o.put("jpeg_quality", 120);

      assert_err(image_save_jpeg(img, "test_jpeg/img_32_x.jpg", o),
         "image_save_jpeg: quality 120 not in range 0..100: test_jpeg/img_32_x.jpg");

      o.put("jpeg_quality", 100);
      image_save_jpeg(img, "test_jpeg/img_32_100.jpg", o);
      I = image_load_jpeg("test_jpeg/img_32_100.jpg", 1);
      assert(color_dist(I.get_argb(10,10),   0xff141400) < 5);
      assert(color_dist(I.get_argb(117,117), 0xffeaea00) < 5);
      assert(color_dist(I.get_argb(138,10),  0xFF090000) < 5);
      assert(color_dist(I.get_argb(245,117), 0xffE90000) < 5);
      assert(color_dist(I.get_argb(64,64),   0xFF808000) < 5);
      assert(color_dist(I.get_argb(192,64),  0xFF800000) < 5);
    }

    /*********************************************/
    { // IMAGE_24RGB
      ImageR img = mk_test_24();
      image_save_jpeg(img, "test_jpeg/img_24_def.jpg");
      ImageR I = image_load_jpeg("test_jpeg/img_24_def.jpg", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert(color_dist(I.get_argb(10,10),   0xff141400) < 5);
      assert(color_dist(I.get_argb(117,117), 0xffeaea00) < 5);
      assert(color_dist(I.get_argb(138,10),  0xFF140000) < 5);
      assert(color_dist(I.get_argb(245,117), 0xffE90000) < 5);
      assert(color_dist(I.get_argb(64,64),   0xFF808000) < 5);
      assert(color_dist(I.get_argb(192,64),  0xFF800000) < 5);
    }

    /*********************************************/
    { // IMAGE_16
      ImageR img = mk_test_16();
      image_save_jpeg(img, "test_jpeg/img_16_def.jpg");
      ImageR I = image_load_jpeg("test_jpeg/img_16_def.jpg", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert(color_dist(I.get_argb(10,10),   0xff111111) < 2);
      assert(color_dist(I.get_argb(117,117), 0xffcfcfcf) < 2);
      assert(color_dist(I.get_argb(138,10),  0xFF060606) < 2);
      assert(color_dist(I.get_argb(245,117), 0xff454545) < 2);
      assert(color_dist(I.get_argb(64,64),   0xFF717171) < 2);
      assert(color_dist(I.get_argb(192,64),  0xFF262626) < 2);
    }

    /*********************************************/
    { // IMAGE_8
      ImageR img = mk_test_8();
      image_save_jpeg(img, "test_jpeg/img_8_def.jpg");
      ImageR I = image_load_jpeg("test_jpeg/img_8_def.jpg", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert(color_dist(I.get_argb(10,10),   0xff111111) < 5);
      assert(color_dist(I.get_argb(117,117), 0xffcfcfcf) < 5);
      assert(color_dist(I.get_argb(138,10),  0xFF060606) < 5);
      assert(color_dist(I.get_argb(245,117), 0xff454545) < 5);
      assert(color_dist(I.get_argb(64,64),   0xFF717171) < 5);
      assert(color_dist(I.get_argb(192,64),  0xFF262626) < 5);
    }

    { // IMAGE_8PAL
      ImageR img = mk_test_8p();
      image_save_jpeg(img, "test_jpeg/img_8p_def.jpg");
      ImageR I = image_load_jpeg("test_jpeg/img_8p_def.jpg", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert(color_dist(I.get_argb(10,10),   0xff101801) < 5);
      assert(color_dist(I.get_argb(117,117), 0xffe6e600) < 5);
      assert(color_dist(I.get_argb(138,10),  0xFF0a0000) < 5);
      assert(color_dist(I.get_argb(245,117), 0xffe60001) < 5);
      assert(color_dist(I.get_argb(64,64),   0xFF858501) < 5);
      assert(color_dist(I.get_argb(192,64),  0xFF820201) < 5);
    }

    { // IMAGE_1
      ImageR img = mk_test_1();
      image_save_jpeg(img, "test_jpeg/img_1_def.jpg");
      ImageR I = image_load_jpeg("test_jpeg/img_1_def.jpg", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(img.get1(0,0), 1);
      assert_eq(img.get1(15,45), 1);
      assert_eq(img.get1(43,123), 0);
      assert_eq(img.get1(203,27), 1);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFDFDFD);
      assert_eq(I.get_argb(203,27), 0xFF000000);
    }

    { //scale tests
      ImageR I0 = image_load_jpeg("test_jpeg/img_32_def.jpg", 1);
      for (double sc=1; sc<10; sc+=0.8){
        ImageR I1 = image_load_jpeg("test_jpeg/img_32_def.jpg", sc);
        assert_eq(I1.width(), floor((I0.width()-1)/sc+1));
        assert_eq(I1.height(), floor((I0.height()-1)/sc+1));

        Point<size_t> pt(101,32);
        Point<size_t> pt1(pt.x/sc,pt.y/sc);
        assert(color_dist(I1.get_rgb(pt1.x, pt1.y),
                          I0.get_rgb(rint(pt1.x*sc), rint(pt1.y*sc))) < 20);

        pt.x = I0.width()-1;   pt.y = I0.height()-1;
        pt1.x = pt.x/sc; pt1.y = pt.y/sc;
        assert(pt.x/sc < I1.width());
        assert(pt.y/sc < I1.height());
        assert(color_dist(I1.get_rgb(pt1.x, pt1.y),
                          I0.get_rgb(rint(pt1.x*sc), rint(pt1.y*sc))) < 20);
      }
      assert_err(image_load_jpeg("test_jpeg/img_32_def.jpg", 0),
        "image_load_jpeg: wrong scale: 0: test_jpeg/img_32_def.jpg");
    }

    { // loading from stream -- IMAGE_32ARGB
      std::ifstream str("test_jpeg/img_32_def.jpg");
      assert_eq(image_size_jpeg(str), iPoint(256,128));
    }
    { // loading from stream -- IMAGE_32ARGB
      std::ifstream str("test_jpeg/img_32_def.jpg");
      ImageR I = image_load_jpeg(str, 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      // check far from edges (smaller jpeg artifacts)
      assert(color_dist(I.get_argb(10,10),   0xff141400) < 5);
      assert(color_dist(I.get_argb(117,117), 0xffeaea00) < 5);
      assert(color_dist(I.get_argb(138,10),  0xFF090000) < 5);
      assert(color_dist(I.get_argb(245,117), 0xffE90000) < 5);
      assert(color_dist(I.get_argb(64,64),   0xFF808000) < 5);
      assert(color_dist(I.get_argb(192,64),  0xFF800000) < 5);
    }


/*
std::cerr << std::hex << I.get_argb(10,10) << "\n";
std::cerr << std::hex << I.get_argb(117,117) << "\n";
std::cerr << std::hex << I.get_argb(138,10) << "\n";
std::cerr << std::hex << I.get_argb(245,117) << "\n";
std::cerr << std::hex << I.get_argb(64,64) << "\n";
std::cerr << std::hex << I.get_argb(192,64) << "\n";
*/

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond