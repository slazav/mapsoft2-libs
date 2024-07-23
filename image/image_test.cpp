#include "image_test.h"
#include "image_colors.h"

ImageR mk_test_64(){
  ImageR img(256,128, IMAGE_64ARGB);
  for (size_t y=0; y<128; ++y){
    for (size_t x=0; x<128; ++x){
      auto c1 = color_argb64(0xFFFF, 400*x, 400*y, 0);
      auto c2 = color_argb64(400*x, 400*y, 0, 0);
      img.set64(x,y,     c1);
      img.set64(128+x,y, c2);
    }
  }
  return img;
}

ImageR mk_test_48(){
  ImageR img(256,128, IMAGE_48RGB);
  for (size_t y=0; y<128; ++y){
    for (size_t x=0; x<128; ++x){
      auto c1 = color_argb64(0xFFFF, 400*x, 400*y, 0);
      auto c2 = color_argb64(0xFFFF, 400*y, 0, 0);
      img.set48(x,y,     c1);
      img.set48(128+x,y, c2);
    }
  }
  return img;
}

ImageR mk_test_32(){
  ImageR img(256,128, IMAGE_32ARGB);
  for (size_t y=0; y<128; ++y){
    for (size_t x=0; x<128; ++x){
      auto c1 = color_argb(0xFF, 2*x, 2*y, 0);
      auto c2 = color_argb(2*x, 2*y, 0, 0);
      img.set32(x,y,     c1);
      img.set32(128+x,y, c2);
    }
  }
  return img;
}

ImageR mk_test_24(){
  ImageR img(256,128, IMAGE_24RGB);
  for (size_t y=0; y<128; ++y){
    for (size_t x=0; x<128; ++x){
      auto c1 = color_argb(0xFF, 2*x, 2*y, 0);
      auto c2 = color_argb(0xFF, 2*x, 0, 0);
      img.set24(x,y,     c1);
      img.set24(128+x,y, c2);
    }
  }
  return img;
}

ImageR mk_test_16(){
  ImageR img(256,128, IMAGE_16);
  for (size_t y=0; y<128; ++y){
    for (size_t x=0; x<128; ++x){
      auto c1 = color_rgb_to_grey16(color_argb(0xFF, 2*x, 2*y, 0));
      auto c2 = color_rgb_to_grey16(color_argb(0xFF, 2*x, 0, 0));
      img.set16(x,y,    c1);
      img.set16(128+x,y,c2);
    }
  }
  return img;
}

ImageR mk_test_8(){
  ImageR img(256,128, IMAGE_8);
  for (size_t y=0; y<128; ++y){
    for (size_t x=0; x<128; ++x){
      auto c1 = color_rgb_to_grey8(color_argb(0xFF, 2*x, 2*y, 0));
      auto c2 = color_rgb_to_grey8(color_argb(0xFF, 2*x, 0, 0));
      img.set8(x,y,    c1);
      img.set8(128+x,y,c2);
    }
  }
  return img;
}
ImageR mk_test_8p(){
  ImageR img = mk_test_32();
  std::vector<uint32_t> colors = image_colormap(img);
  return image_remap(img, colors);
}

ImageR mk_test_1(){
  ImageR img(250,125, IMAGE_1);
  // For BW images it's not enough to fill all points,
  // to avoid "uninitialised byte(s)" errors in valgrind.
  img.fill1(0);
  for (size_t y=0; y<img.height(); ++y){
    for (size_t x=0; x<img.width(); ++x){
      img.set1(x,y, 1-(int)fabs(600*sin(2*M_PI*x/255)*sin(2*M_PI*y/255))%2);
    }
  }
  return img;
}
