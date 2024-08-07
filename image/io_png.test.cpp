///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <fstream>
#include "io_png.h"
#include "err/assert_err.h"
#include "image_colors.h"
#include "image_test.h"

int
main(){
  try{

    // size
    assert_err(image_size_png("test_png/missing"),
      "Can't open file: test_png/missing");
    assert_err(image_size_png("test_png/Readme.md"),
      "image_size_png: not a PNG file: test_png/Readme.md");

    // load
    assert_err(image_load_png("test_png/missing"),
      "Can't open file: test_png/missing");
    assert_err(image_load_png("test_png/Readme.md"),
      "image_load_png: not a PNG file: test_png/Readme.md");

    /*********************************************/
    // * Create all types of images (32ARGB, 24RGB, 16, 8, 1, PAL).
    // * Save them with different image_save_png() options.
    // * Read saved file and check result.

    { // IMAGE_32ARGB
      ImageR img = mk_test_32();
      image_save_png(img, "test_png/img_32_def.png");
      assert_eq(image_size_png("test_png/img_32_def.png"), iPoint(256,128));
      // because of color prescaling we do not have exact match with
      // original colors...
      ImageR I = image_load_png("test_png/img_32_def.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0),     0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0),   0x0);
      assert_eq(I.get_argb(255,127), 0xFEFC0000);
      assert_eq(I.get_argb(64,64),   0xFF808000);
      assert_eq(I.get_argb(192,64),  0x803F0000);

      Opt o;
      o.put("png_format", "rgb");
      image_save_png(img, "test_png/img_32_rgb.png", o);
      I = image_load_png("test_png/img_32_rgb.png", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFFFD0000);
      assert_eq(I.get_argb(64,64), 0xFF808000);
      assert_eq(I.get_argb(192,64), 0xFF7F0000);

      o.put("png_format", "grey");
      image_save_png(img, "test_png/img_32_grey.png", o);
      I = image_load_png("test_png/img_32_grey.png", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "agrey");
      image_save_png(img, "test_png/img_32_agrey.png", o);
      I = image_load_png("test_png/img_32_agrey.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0x00000000);
      assert_eq(I.get_argb(255,127), 0xFE4b4b4b);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0x80131313);

      o.put("png_format", "pal");
      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "none");
      image_save_png(img, "test_png/img_32_pal.png", o);
      I = image_load_png("test_png/img_32_pal.png", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xfff30000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF890000);

      o.put("png_format", "pal");
      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "full");
      image_save_png(img, "test_png/img_32_apal.png", o);
      I = image_load_png("test_png/img_32_apal.png", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xF0080000);
      assert_eq(I.get_argb(127,127), 0xFFEEEE00);
      assert_eq(I.get_argb(128,0), 0x0E050000);
      assert_eq(I.get_argb(255,127), 0xF1E50000);
      assert_eq(I.get_argb(64,64), 0xFF8C8C00);
      assert_eq(I.get_argb(192,64), 0x874C0000);

      o.put("png_format", "pal");
      o.put("cmap_colors", 120);
      o.put("cmap_alpha", "gif");
      image_save_png(img, "test_png/img_32_gpal.png", o);
      I = image_load_png("test_png/img_32_gpal.png", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff0A0000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0x00000000);
      assert_eq(I.get_argb(255,127), 0xFFF40000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF8A0000);

      o.put("png_format", "pal");
      o.put("cmap_colors", 300); // too many colors
      assert_err( image_save_png(img, "test_png/img_32_xpal.png", o),
        "image_remap: palette length is out of range: test_png/img_32_xpal.png");

    }

    /*********************************************/
    { // IMAGE_24RGB
      ImageR img = mk_test_24();
      image_save_png(img, "test_png/img_24_def.png");
      ImageR I = image_load_png("test_png/img_24_def.png", 1);
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
      o.put("png_format", "argb");
      image_save_png(img, "test_png/img_24_argb.png", o);
      I = image_load_png("test_png/img_24_argb.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFFFE0000);
      assert_eq(I.get_argb(64,64), 0xFF808000);
      assert_eq(I.get_argb(192,64), 0xFF800000);

      o.put("png_format", "grey");
      image_save_png(img, "test_png/img_24_grey.png", o);
      I = image_load_png("test_png/img_24_grey.png", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "agrey"); // same as grey
      image_save_png(img, "test_png/img_24_agrey.png", o);
      I = image_load_png("test_png/img_24_agrey.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "pal");
      o.put("cmap_colors", 120);
      image_save_png(img, "test_png/img_24_pal.png", o);
      I = image_load_png("test_png/img_24_pal.png", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff0E0000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff0E0000);
      assert_eq(I.get_argb(255,127), 0xfff60000);
      assert_eq(I.get_argb(64,64), 0xFF8C7600);
      assert_eq(I.get_argb(192,64), 0xFF8C0000);
    }

    /*********************************************/
    { // IMAGE_16
      ImageR img = mk_test_16();
      image_save_png(img, "test_png/img_16_def.png");
      ImageR I = image_load_png("test_png/img_16_def.png", 1);
      assert_eq(I.type(), IMAGE_16);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);

      assert_eq(I.get16(0,0),     img.get16(0,0));
      assert_eq(I.get16(127,127), img.get16(127,127));
      assert_eq(I.get16(128,0),   img.get16(128,0));
      assert_eq(I.get16(255,127), img.get16(255,127));
      assert_eq(I.get16(64,64),   img.get16(64,64));
      assert_eq(I.get16(192,64),  img.get16(192,64));

      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFE1E1E1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      Opt o;
      o.put("png_format", "argb");
      image_save_png(img, "test_png/img_16_argb.png", o);
      I = image_load_png("test_png/img_16_argb.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "rgb");
      image_save_png(img, "test_png/img_16_rgb.png", o);
      I = image_load_png("test_png/img_16_rgb.png", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "grey");
      image_save_png(img, "test_png/img_16_grey.png", o);
      I = image_load_png("test_png/img_16_grey.png", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "agrey");
      image_save_png(img, "test_png/img_16_agrey.png", o);
      I = image_load_png("test_png/img_16_agrey.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "pal");
      o.put("cmap_colors", 32);
      image_save_png(img, "test_png/img_16_pal.png", o);
      I = image_load_png("test_png/img_16_pal.png", 1);
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
      image_save_png(img, "test_png/img_8_def.png");
      ImageR I = image_load_png("test_png/img_8_def.png", 1);
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
      o.put("png_format", "argb");
      image_save_png(img, "test_png/img_8_argb.png", o);
      I = image_load_png("test_png/img_8_argb.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFE1E1E1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "rgb");
      image_save_png(img, "test_png/img_8_rgb.png", o);
      I = image_load_png("test_png/img_8_rgb.png", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFE1E1E1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4C4C4C);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "grey");
      image_save_png(img, "test_png/img_8_grey.png", o);
      I = image_load_png("test_png/img_8_grey.png", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "agrey");
      image_save_png(img, "test_png/img_8_agrey.png", o);
      I = image_load_png("test_png/img_8_agrey.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFe1e1e1);
      assert_eq(I.get_argb(128,0), 0xFF000000);
      assert_eq(I.get_argb(255,127), 0xFF4c4c4c);
      assert_eq(I.get_argb(64,64), 0xFF717171);
      assert_eq(I.get_argb(192,64), 0xFF262626);

      o.put("png_format", "pal");
      o.put("cmap_colors", 32);
      image_save_png(img, "test_png/img_8_pal.png", o);
      I = image_load_png("test_png/img_8_pal.png", 1);
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
      image_save_png(img, "test_png/img_8p_def.png");
      ImageR I = image_load_png("test_png/img_8p_def.png", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xfff30000);
      assert_eq(I.get_argb(64,64), 0xff848600);
      assert_eq(I.get_argb(192,64), 0xff810000);


      Opt o;
      o.put("png_format", "argb");
      image_save_png(img, "test_png/img_8p_argb.png", o);
      I = image_load_png("test_png/img_8p_argb.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xfff30000);
      assert_eq(I.get_argb(64,64), 0xff848600);
      assert_eq(I.get_argb(192,64), 0xff810000);

      o.put("png_format", "rgb");
      image_save_png(img, "test_png/img_8p_rgb.png", o);
      I = image_load_png("test_png/img_8p_rgb.png", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xff090000);
      assert_eq(I.get_argb(127,127), 0xfff6f600);
      assert_eq(I.get_argb(128,0), 0xff090000);
      assert_eq(I.get_argb(255,127), 0xfff30000);
      assert_eq(I.get_argb(64,64), 0xff848600);
      assert_eq(I.get_argb(192,64), 0xff810000);

      o.put("png_format", "grey");
      image_save_png(img, "test_png/img_8p_grey.png", o);
      I = image_load_png("test_png/img_8p_grey.png", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF030303);
      assert_eq(I.get_argb(127,127), 0xFFDADADA);
      assert_eq(I.get_argb(128,0), 0xFF030303);
      assert_eq(I.get_argb(255,127), 0xFF494949);
      assert_eq(I.get_argb(64,64), 0xFF767676);
      assert_eq(I.get_argb(192,64), 0xFF272727);

      o.put("png_format", "agrey");
      image_save_png(img, "test_png/img_8p_agrey.png", o);
      I = image_load_png("test_png/img_8p_agrey.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF030303);
      assert_eq(I.get_argb(127,127), 0xFFDADADA);
      assert_eq(I.get_argb(128,0), 0xFF030303);
      assert_eq(I.get_argb(255,127), 0xFF494949);
      assert_eq(I.get_argb(64,64), 0xFF767676);
      assert_eq(I.get_argb(192,64), 0xFF272727);

      o.put("png_format", "pal");
      o.put("cmap_colors", 100);
      image_save_png(img, "test_png/img_8p_pal.png", o);
      I = image_load_png("test_png/img_8p_pal.png", 1);
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
      image_save_png(img, "test_png/img_1_def.png");
      ImageR I = image_load_png("test_png/img_1_def.png", 1);
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

      Opt o;
      o.put("png_format", "argb");
      image_save_png(img, "test_png/img_1_argb.png", o);
      I = image_load_png("test_png/img_1_argb.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);

      o.put("png_format", "rgb");
      image_save_png(img, "test_png/img_1_rgb.png", o);
      I = image_load_png("test_png/img_1_rgb.png", 1);
      assert_eq(I.type(), IMAGE_24RGB);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);

      o.put("png_format", "grey");
      image_save_png(img, "test_png/img_1_grey.png", o);
      I = image_load_png("test_png/img_1_grey.png", 1);
      assert_eq(I.type(), IMAGE_8);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);

      o.put("png_format", "agrey");
      image_save_png(img, "test_png/img_1_agrey.png", o);
      I = image_load_png("test_png/img_1_agrey.png", 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);

      o.put("png_format", "pal");
      o.put("cmap_colors", 32);
      image_save_png(img, "test_png/img_1_pal.png", o);
      I = image_load_png("test_png/img_1_pal.png", 1);
      assert_eq(I.type(), IMAGE_8PAL);
      assert_eq(I.width(), 250);
      assert_eq(I.height(), 125);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(15,45), 0xFF000000);
      assert_eq(I.get_argb(43,123), 0xFFFFFFFF);
      assert_eq(I.get_argb(203,27), 0xFF000000);
    }

    { //scale tests
      ImageR I0 = image_load_png("test_png/img_32_def.png", 1);
      for (double sc=1; sc<10; sc+=0.8){
        ImageR I1 = image_load_png("test_png/img_32_def.png", sc);
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
      assert_err(image_load_png("test_png/img_32_def.png", 0),
        "image_load_png: wrong scale: 0: test_png/img_32_def.png");
    }

    { //load from std::istream
      std::ifstream str("test_png/img_32_def.png");
      assert_err(image_load_png(str, 0),
        "image_load_png: wrong scale: 0");
      ImageR I = image_load_png(str, 1);
      assert_eq(I.type(), IMAGE_32ARGB);
      assert_eq(I.width(), 256);
      assert_eq(I.height(), 128);
      assert_eq(I.get_argb(0,0), 0xFF000000);
      assert_eq(I.get_argb(127,127), 0xFFFEFE00);
      assert_eq(I.get_argb(128,0), 0x00000000);
      assert_eq(I.get_argb(255,127), 0xFeFC0000);
      assert_eq(I.get_argb(64,64), 0xFF808000);
      assert_eq(I.get_argb(192,64), 0x803F0000);
    }
    { //load from std::istream
      std::ifstream str("test_png/img_32_def.png");
      iPoint p = image_size_png(str);
      assert_eq(p.x, 256);
      assert_eq(p.y, 128);
    }

    { // blue.PNG
      ImageR I = image_load_png("test_png/blue.PNG");
      assert_eq(I.get_argb(100,100), 0xFF9ABFDD);
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