///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include "err/assert_err.h"
#include "io_gif.h"
#include "image_colors.h"
#include "image_test.h"

int
main(){
  try{

    // size
    assert_err(image_size_gif("test_gif/missing"),
      "image_size_gif: can't open file: test_gif/missing");
    assert_err(image_size_gif("test_gif/Readme.md"),
      "image_size_gif: can't open file: test_gif/Readme.md");

    // load
    assert_err(image_load_gif("test_gif/missing"),
      "image_load_gif: can't open file: test_gif/missing");
    assert_err(image_load_gif("test_gif/Readme.md"),
      "image_load_gif: can't open file: test_gif/Readme.md");

    /*********************************************/
    // * Create all types of images (32ARGB, 24RGB, 16, 8, 1, PAL).
    // * Save them with different image_save_gif() options.
    // * Read saved file and check result.

    { // IMAGE_32ARGB
      ImageR img = mk_test_32();
      image_save_gif(img, "test_gif/img_32_def.gif");
      assert_eq(image_size_gif("test_gif/img_32_def.gif"), iPoint(256,128));

      ImageR I = image_load_gif("test_gif/img_32_def.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff0a0000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0x00000000);
      assert_eq(I.get_argb(255,127), 0xfff40000);
      assert_eq(I.get_argb(64,64), 0xFF848600);
      assert_eq(I.get_argb(192,64), 0xFF820000);
      Opt o;

      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "none");
      image_save_gif(img, "test_gif/img_32_120n.gif", o);
      I = image_load_gif("test_gif/img_32_120n.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xfff30000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF890000);

      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "full"); // same as default, same as gif
      image_save_gif(img, "test_gif/img_32_120f.gif", o);
      I = image_load_gif("test_gif/img_32_120f.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff0a0000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0x00000000);
      assert_eq(I.get_argb(255,127), 0xfff40000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF8A0000);

      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "gif");
      image_save_gif(img, "test_gif/img_32_120g.gif", o);
      I = image_load_gif("test_gif/img_32_120g.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff0A0000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0x00000000);
      assert_eq(I.get_argb(255,127), 0xfff40000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF8A0000);
    }

    /*********************************************/
    { // IMAGE_24RGB
      ImageR img = mk_test_24();
      image_save_gif(img, "test_gif/img_24_def.gif");
      ImageR I = image_load_gif("test_gif/img_24_def.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff060000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xFF060000);
      assert_eq(I.get_argb(255,127), 0xfff60000);
      assert_eq(I.get_argb(64,64), 0xFF848600);
      assert_eq(I.get_argb(192,64), 0xFF840000);
    }

    /*********************************************/
    { // IMAGE_16
      ImageR img = mk_test_16();
      image_save_gif(img, "test_gif/img_16_def.gif");
      ImageR I = image_load_gif("test_gif/img_16_def.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);

      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

    }

    /*********************************************/

    { // IMAGE_8
      ImageR img = mk_test_8();
      image_save_gif(img, "test_gif/img_8_def.gif");
      ImageR I = image_load_gif("test_gif/img_8_def.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFE1E1E1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);
    }


    { // IMAGE_8PAL
      ImageR img = mk_test_8p();
      image_save_gif(img, "test_gif/img_8p_def.gif");
      ImageR I = image_load_gif("test_gif/img_8p_def.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xfff30000);
      assert_eq(I.get_argb(64,64), 0xff848600);
      assert_eq(I.get_argb(192,64), 0xff810000);
    }


    { // IMAGE_1
      ImageR img = mk_test_1();
      image_save_gif(img, "test_gif/img_1_def.gif");
      ImageR I = image_load_gif("test_gif/img_1_def.gif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(img.get1(0,0), 1);
      assert_eq(img.get1(15,45), 1);
      assert_eq(img.get1(43,123), 0);
      assert_eq(img.get1(203,27), 1);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);
    }

    { //scale tests
      ImageR I0 = image_load_gif("test_gif/img_32_def.gif", 1);
      for (double sc=1; sc<10; sc+=0.8){
        ImageR I1 = image_load_gif("test_gif/img_32_def.gif", sc);
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
      assert_err(image_load_gif("test_gif/img_32_def.gif", 0),
        "image_load_gif: wrong scale: 0");
    }

/*
std::cerr << std::hex << I.get_argb(0,0) << "\n";
std::cerr << std::hex << I.get_argb(127,127) << "\n";
std::cerr << std::hex << I.get_argb(128,0) << "\n";
std::cerr << std::hex << I.get_argb(255,127) << "\n";
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