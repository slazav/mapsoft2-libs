///\cond HIDDEN (do not show this in Doxyden)

// test image_crop() from image_colors.h

#include <cassert>
#include <iostream>
#include <iomanip>
#include "image_colors.h"
#include "io_png.h"
#include "io_pnm.h"
#include "image_test.h"
#include "err/assert_err.h"

int
main(){
  try{

    // I want to check all image types
    {
      ImageR img = mk_test_64();
      image_save_png(image_crop(img, iRect(-10,-20, 128,128)), "test_crop/crop64_tl.png");
      image_save_png(image_crop(img, iRect(138,20, 128,128)), "test_crop/crop64_br.png");
      ImageR img1 = image_load_png("test_crop/crop64_tl.png");
      ImageR img2 = image_load_png("test_crop/crop64_br.png");
      assert_eq(img1.width(), 118);
      assert_eq(img1.height(), 108);
      assert_eq(img2.width(), 118);
      assert_eq(img2.height(), 108);
      assert_eq(img1.type(), IMAGE_32ARGB);
      assert_eq(img2.type(), IMAGE_32ARGB);
      assert_eq(img1.get32(0,0), color_rgb_64to32(img.get64(0,0)));
      assert_eq(img2.get32(0,0), 0x0F010000);
    }

    {
      ImageR img = mk_test_48();
      image_save_png(image_crop(img, iRect(-10,-20, 128,128)), "test_crop/crop48_tl.png");
      image_save_png(image_crop(img, iRect(138,20, 128,128)), "test_crop/crop48_br.png");
      ImageR img1 = image_load_png("test_crop/crop48_tl.png");
      ImageR img2 = image_load_png("test_crop/crop48_br.png");
      assert_eq(img1.width(), 118);
      assert_eq(img1.height(), 108);
      assert_eq(img2.width(), 118);
      assert_eq(img2.height(), 108);
      assert_eq(img1.type(), IMAGE_24RGB);
      assert_eq(img2.type(), IMAGE_24RGB);
      assert_eq(img1.get24(0,0), color_rgb_64to32(img.get48(0,0)));
      assert_eq(img2.get24(0,0), color_rgb_64to32(img.get48(138,20)));
    }

    {
      ImageR img = mk_test_32();
      image_save_png(image_crop(img, iRect(-10,-20, 128,128)), "test_crop/crop32_tl.png");
      image_save_png(image_crop(img, iRect(138,20, 128,128)), "test_crop/crop32_br.png");
      ImageR img1 = image_load_png("test_crop/crop32_tl.png");
      ImageR img2 = image_load_png("test_crop/crop32_br.png");
      assert_eq(img1.width(), 118);
      assert_eq(img1.height(), 108);
      assert_eq(img2.width(), 118);
      assert_eq(img2.height(), 108);
      assert_eq(img1.type(), IMAGE_32ARGB);
      assert_eq(img2.type(), IMAGE_32ARGB);
      assert_eq(img1.get32(0,0),img.get32(0,0));
      assert_eq(img2.get32(0,0),0x14020000);
    }

    {
      ImageR img = mk_test_24();
      image_save_png(image_crop(img, iRect(-10,-20, 128,128)), "test_crop/crop24_tl.png");
      image_save_png(image_crop(img, iRect(138,20, 128,128)), "test_crop/crop24_br.png");
      ImageR img1 = image_load_png("test_crop/crop24_tl.png");
      ImageR img2 = image_load_png("test_crop/crop24_br.png");
      assert_eq(img1.width(), 118);
      assert_eq(img1.height(), 108);
      assert_eq(img2.width(), 118);
      assert_eq(img2.height(), 108);
      assert_eq(img1.type(), IMAGE_24RGB);
      assert_eq(img2.type(), IMAGE_24RGB);
      assert_eq(img1.get24(0,0),img.get24(0,0));
      assert_eq(img2.get24(0,0),img.get24(138,20));
    }

    {
      ImageR img = mk_test_16();
      image_save_png(image_crop(img, iRect(-10,-20, 128,128)), "test_crop/crop16_tl.png");
      image_save_png(image_crop(img, iRect(138,20, 128,128)), "test_crop/crop16_br.png");
      ImageR img1 = image_load_png("test_crop/crop16_tl.png");
      ImageR img2 = image_load_png("test_crop/crop16_br.png");
      assert_eq(img1.width(), 118);
      assert_eq(img1.height(), 108);
      assert_eq(img2.width(), 118);
      assert_eq(img2.height(), 108);
      assert_eq(img1.type(), IMAGE_16);
      assert_eq(img2.type(), IMAGE_16);
      assert_eq(img1.get16(0,0),img.get16(0,0));
      assert_eq(img2.get16(0,0),img.get16(138,20));
    }

    {
      ImageR img = mk_test_8();
      image_save_png(image_crop(img, iRect(-10,-20, 128,128)), "test_crop/crop08_tl.png");
      image_save_png(image_crop(img, iRect(138,20, 128,128)), "test_crop/crop08_br.png");
      ImageR img1 = image_load_png("test_crop/crop08_tl.png");
      ImageR img2 = image_load_png("test_crop/crop08_br.png");
      assert_eq(img1.width(), 118);
      assert_eq(img1.height(), 108);
      assert_eq(img2.width(), 118);
      assert_eq(img2.height(), 108);
      assert_eq(img1.type(), IMAGE_8);
      assert_eq(img2.type(), IMAGE_8);
      assert_eq(img1.get8(0,0),img.get8(0,0));
      assert_eq(img2.get8(0,0),img.get8(138,20));
    }

    {
      ImageR img32 = mk_test_32();
      std::vector<uint32_t> colors = image_colormap(img32);
      ImageR img = image_remap(img32, colors);

      image_save_png(image_crop(img, iRect(-10,-20, 128,128)), "test_crop/crop08p_tl.png");
      image_save_png(image_crop(img, iRect(138,20, 128,128)), "test_crop/crop08p_br.png");
      ImageR img1 = image_load_png("test_crop/crop08p_tl.png");
      ImageR img2 = image_load_png("test_crop/crop08p_br.png");
      assert_eq(img1.width(), 118);
      assert_eq(img1.height(), 108);
      assert_eq(img2.width(), 118);
      assert_eq(img2.height(), 108);
      assert_eq(img1.type(), IMAGE_8PAL);
      assert_eq(img2.type(), IMAGE_8PAL);
      assert_eq(img1.get8pal(0,0),img.get8pal(0,0));
      assert_eq(img2.get8pal(0,0),img.get8pal(138,20));
    }

    {
      ImageR img = mk_test_1();
      image_save_pnm(image_crop(img, iRect(-10,-20, 125,125)), "test_crop/crop01_tl.pnm");
      image_save_pnm(image_crop(img, iRect(135,20, 125,125)), "test_crop/crop01_br.pnm");
      ImageR img1 = image_load_pnm("test_crop/crop01_tl.pnm");
      ImageR img2 = image_load_pnm("test_crop/crop01_br.pnm");
      assert_eq(img1.width(), 115);
      assert_eq(img1.height(), 105);
      assert_eq(img2.width(), 115);
      assert_eq(img2.height(), 105);
      assert_eq(img1.type(), IMAGE_1);
      assert_eq(img2.type(), IMAGE_1);
      assert_eq(img1.get1(0,0),img.get1(0,0));
      assert_eq(img1.get1(10,5),img.get1(10,5));
      assert_eq(img2.get1(0,0),img.get1(138,20));
      assert_eq(img2.get1(20,10),img.get1(158,40));
    }


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
