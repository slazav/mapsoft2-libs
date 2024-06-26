///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <fstream>
#include "err/assert_err.h"
#include "io_tiff.h"
#include "image_colors.h"
#include "image_test.h"

int
main(){
  try{

    // size
    assert_err(image_size_tiff("test_tiff/missing"),
      "Can't open file: test_tiff/missing");
    assert_err(image_size_tiff("test_tiff/Readme.md"),
      "TIFF error: Not a TIFF or MDI file, bad magic number 27973 (0x6d45): test_tiff/Readme.md");

    // load
    assert_err(image_load_tiff("test_tiff/missing"),
      "Can't open file: test_tiff/missing");
    assert_err(image_load_tiff("test_tiff/Readme.md"),
      "TIFF error: Not a TIFF or MDI file, bad magic number 27973 (0x6d45): test_tiff/Readme.md");

    // * Create all types of images (32ARGB, 24RGB, 16, 8, 1, PAL).
    // * Save them with different image_save_tiff() options.
    // * Read saved file and check result.

    { // IMAGE_32ARGB
      ImageR img = mk_test_32();
      image_save_tiff(img, "test_tiff/img_32_def.tif");
      assert_eq(image_size_tiff("test_tiff/img_32_def.tif"), iPoint(256,128));

      ImageR I = image_load_tiff("test_tiff/img_32_def.tif", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0), 0x00000000);
      assert_eq(I.get_argb(255,127), 0xFEFC0000);
      assert_eq(I.get_argb(64,64), 0xFF808000);
      assert_eq(I.get_argb(192,64), 0x803F0000);

      Opt o;
      o.put("tiff_format", "rgb");
      image_save_tiff(img, "test_tiff/img_32_rgb.tif", o);
      I = image_load_tiff("test_tiff/img_32_rgb.tif", 1);

      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFFFD0000);
      assert_eq(I.get_argb(64,64), 0xFF808000);
      assert_eq(I.get_argb(192,64), 0xFF7F0000);

      o.put("tiff_format", "grey");
      image_save_tiff(img, "test_tiff/img_32_grey.tif", o);
      I = image_load_tiff("test_tiff/img_32_grey.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "none");
      image_save_tiff(img, "test_tiff/img_32_pal.tif", o);
      I = image_load_tiff("test_tiff/img_32_pal.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xfff30000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF890000);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "full"); // ignored
      image_save_tiff(img, "test_tiff/img_32_apal.tif", o);
      I = image_load_tiff("test_tiff/img_32_apal.tif", 1);

      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xFFF30000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF890000);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "gif"); // ignored
      image_save_tiff(img, "test_tiff/img_32_gpal.tif", o);
      I = image_load_tiff("test_tiff/img_32_gpal.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xFFF30000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF890000);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 300); // too many colors
      assert_err( image_save_tiff(img, "test_tiff/img_32_xpal.tif", o),
        "image_remap: palette length is out of range: test_tiff/img_32_xpal.tif");

    }

    /*********************************************/
    { // IMAGE_24RGB
      ImageR img = mk_test_24();
      image_save_tiff(img, "test_tiff/img_24_def.tif");
      ImageR I = image_load_tiff("test_tiff/img_24_def.tif", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFFFE0000);
      assert_eq(I.get_argb(64,64), 0xFF808000);
      assert_eq(I.get_argb(192,64), 0xFF800000);

      Opt o;
      o.put("tiff_format", "argb");
      image_save_tiff(img, "test_tiff/img_24_argb.tif", o);
      I = image_load_tiff("test_tiff/img_24_argb.tif", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFFFE0000);
      assert_eq(I.get_argb(64,64), 0xFF808000);
      assert_eq(I.get_argb(192,64), 0xFF800000);

      o.put("tiff_format", "grey");
      image_save_tiff(img, "test_tiff/img_24_grey.tif", o);
      I = image_load_tiff("test_tiff/img_24_grey.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 120);
      image_save_tiff(img, "test_tiff/img_24_pal.tif", o);
      I = image_load_tiff("test_tiff/img_24_pal.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF0E0000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff0E0000);
      assert_eq(I.get_argb(255,127), 0xFFF60000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF8C0000);
    }

    /*********************************************/
    { // IMAGE_16
      ImageR img = mk_test_16();
      image_save_tiff(img, "test_tiff/img_16_def.tif");
      ImageR I = image_load_tiff("test_tiff/img_16_def.tif", 1);
      assert_eq(I.type(), IMAGE_16);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get16(0,0),     img.get16(0,0));
      assert_eq(I.get16(127,127), img.get16(127,127));
      assert_eq(I.get16(128,0),   img.get16(128,0));
      assert_eq(I.get16(255,127), img.get16(255,127));
      assert_eq(I.get16(64,64),   img.get16(64,64));
      assert_eq(I.get16(192,64),  img.get16(192,64));

      Opt o;
      o.put("tiff_format", "argb");
      image_save_tiff(img, "test_tiff/img_16_argb.tif", o);
      I = image_load_tiff("test_tiff/img_16_argb.tif", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "rgb");
      image_save_tiff(img, "test_tiff/img_16_rgb.tif", o);
      I = image_load_tiff("test_tiff/img_16_rgb.tif", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "grey");
      image_save_tiff(img, "test_tiff/img_16_grey.tif", o);
      I = image_load_tiff("test_tiff/img_16_grey.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 32);
      image_save_tiff(img, "test_tiff/img_16_pal.tif", o);
      I = image_load_tiff("test_tiff/img_16_pal.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF030303);
      assert_eq(I.get_argb(127,127), 0xFFDBDBDB);
      assert_eq(I.get_argb(128,0), 0xFF030303);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF737373);
      assert_eq(I.get_argb(192,64), 0xFF242424);

    }

    /*********************************************/
    { // IMAGE_8
      ImageR img = mk_test_8();
      image_save_tiff(img, "test_tiff/img_8_def.tif");
      ImageR I = image_load_tiff("test_tiff/img_8_def.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFE1E1E1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      Opt o;
      o.put("tiff_format", "argb");
      image_save_tiff(img, "test_tiff/img_8_argb.tif", o);
      I = image_load_tiff("test_tiff/img_8_argb.tif", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFE1E1E1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "rgb");
      image_save_tiff(img, "test_tiff/img_8_rgb.tif", o);
      I = image_load_tiff("test_tiff/img_8_rgb.tif", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFE1E1E1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "grey");
      image_save_tiff(img, "test_tiff/img_8_grey.tif", o);
      I = image_load_tiff("test_tiff/img_8_grey.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 32);
      image_save_tiff(img, "test_tiff/img_8_pal.tif", o);
      I = image_load_tiff("test_tiff/img_8_pal.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF030303);
      assert_eq(I.get_argb(127,127), 0xFFDCDCDC);
      assert_eq(I.get_argb(128,0), 0xFF030303);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF737373);
      assert_eq(I.get_argb(192,64), 0xFF252525);
    }


    { // IMAGE_8PAL
      ImageR img = mk_test_8p();
      image_save_tiff(img, "test_tiff/img_8p_def.tif");
      ImageR I = image_load_tiff("test_tiff/img_8p_def.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xFF090000);
      assert_eq(I.get_argb(255,127), 0xFFF30000);
      assert_eq(I.get_argb(64,64), 0xff848600);
      assert_eq(I.get_argb(192,64), 0xFF810000);


      Opt o;
      o.put("tiff_format", "argb");
      image_save_tiff(img, "test_tiff/img_8p_argb.tif", o);
      I = image_load_tiff("test_tiff/img_8p_argb.tif", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xFF090000);
      assert_eq(I.get_argb(255,127), 0xFFF30000);
      assert_eq(I.get_argb(64,64), 0xff848600);
      assert_eq(I.get_argb(192,64), 0xFF810000);

      o.put("tiff_format", "rgb");
      image_save_tiff(img, "test_tiff/img_8p_rgb.tif", o);
      I = image_load_tiff("test_tiff/img_8p_rgb.tif", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xFF090000);
      assert_eq(I.get_argb(255,127), 0xFFF30000);
      assert_eq(I.get_argb(64,64), 0xff848600);
      assert_eq(I.get_argb(192,64), 0xFF810000);

      o.put("tiff_format", "grey");
      image_save_tiff(img, "test_tiff/img_8p_grey.tif", o);
      I = image_load_tiff("test_tiff/img_8p_grey.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF030303);
      assert_eq(I.get_argb(127,127), 0xFFDADADA);
      assert_eq(I.get_argb(128,0), 0xFF030303);
      assert_eq(I.get_argb(255,127), 0xFF494949);
      assert_eq(I.get_argb(64,64), 0xFF767676);
      assert_eq(I.get_argb(192,64), 0xFF272727);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 100);
      image_save_tiff(img, "test_tiff/img_8p_pal.tif", o);
      I = image_load_tiff("test_tiff/img_8p_pal.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF090000);
      assert_eq(I.get_argb(127,127), 0xFFEEEE00);
      assert_eq(I.get_argb(128,0), 0xFF090000);
      assert_eq(I.get_argb(255,127), 0xFFEB0000);
      assert_eq(I.get_argb(64,64), 0xFF8B8B00);
      assert_eq(I.get_argb(192,64), 0xFF880000);

    }


    { // IMAGE_1
      ImageR img = mk_test_1();
      image_save_tiff(img, "test_tiff/img_1_def.tif");
      ImageR I = image_load_tiff("test_tiff/img_1_def.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(img.get1(0,0), 1);
      assert_eq(img.get1(15,45), 1);
      assert_eq(img.get1(43,123), 0);
      assert_eq(img.get1(203,27), 1);
      assert_eq(I.get8(0,0), 0x00);
      assert_eq(I.get8(15,45), 0x00);
      assert_eq(I.get8(43,123), 0xFF);
      assert_eq(I.get8(203,27), 0x00);

      Opt o;
      o.put("tiff_format", "argb");
      image_save_tiff(img, "test_tiff/img_1_argb.tif", o);
      I = image_load_tiff("test_tiff/img_1_argb.tif", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);

      o.put("tiff_format", "rgb");
      image_save_tiff(img, "test_tiff/img_1_rgb.tif", o);
      I = image_load_tiff("test_tiff/img_1_rgb.tif", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);

      o.put("tiff_format", "grey");
      image_save_tiff(img, "test_tiff/img_1_grey.tif", o);
      I = image_load_tiff("test_tiff/img_1_grey.tif", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);

      o.put("tiff_format", "pal");
      o.put("cmap_colors", 32);
      image_save_tiff(img, "test_tiff/img_1_pal.tif", o);
      I = image_load_tiff("test_tiff/img_1_pal.tif", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);
    }

    { //scale tests
      ImageR I0 = image_load_tiff("test_tiff/img_32_def.tif", 1);
      iPoint pt(101,32);
      for (double sc=1; sc<10; sc+=0.8){
        ImageR I1 = image_load_tiff("test_tiff/img_32_def.tif", sc);
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
      assert_err(image_load_tiff("test_tiff/img_32_def.tif", 0),
        "image_load_tiff: wrong scale: 0: test_tiff/img_32_def.tif");
    }

    { // loading from std::istring
      ImageR I0 = image_load_tiff("test_tiff/img_32_def.tif", 1);

      std::ifstream in("test_tiff/img_32_def.tif");
      ImageR I1 = image_load_tiff(in, 1);
      assert_eq(I0.width(),  I1.width());
      assert_eq(I0.height(), I1.height());

      for (size_t y=0;y<I0.height();y++)
        for (size_t x=0;x<I0.width();x++)
          assert_eq(I0.get_argb(x,y), I1.get_argb(x,y));
    }

    { // loading from std::istring
      iPoint p0 = image_size_tiff("test_tiff/img_32_def.tif");
      std::ifstream in("test_tiff/img_32_def.tif");
      iPoint p1 = image_size_tiff(in);
      assert_eq(p0,p1);
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