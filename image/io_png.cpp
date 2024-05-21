#include <cstring>
#include <fstream>
#include <png.h>
#include "io_png.h"
#include "image_colors.h"

/**********************************************************/

void
png_read_data_fn(png_structp png_ptr, png_bytep data, size_t length){
  auto str = (std::istream *) png_get_io_ptr(png_ptr);
  str->read((char*)data, length);
}

static const char* libpng_err_msg;
void user_error_fn(png_structp png_ptr, png_const_charp error_msg){
  libpng_err_msg = error_msg;
  longjmp(png_jmpbuf(png_ptr), 0);
}

/**********************************************************/

iPoint
image_size_png(std::istream & str){

  if (!str)  throw Err() << "image_size_png: can't open file";

  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL, end_info = NULL;

  png_uint_32 w, height;

  try {

    png_byte sign[8];
    const char sign_size = 8;
    str.read((char*)sign, sign_size);
    if (str.gcount()!=sign_size ||
        png_sig_cmp(sign, 0, sign_size)!=0)
      throw Err() << "image_size_png: not a PNG file";

    png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
    if (!png_ptr) throw Err() << "image_size_png: can't make png_read_struct";

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) throw Err() << "image_size_png: can't make png_info_struct";

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) throw Err() << "image_size_png: can't make png_info_struct";

    png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), user_error_fn, NULL);

    if (setjmp(png_jmpbuf(png_ptr)))
      throw Err() << "image_size_png: " << libpng_err_msg;

    png_set_read_fn(png_ptr, &str, png_read_data_fn);


    png_set_sig_bytes(png_ptr, sign_size);
    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &w, &height,
       &bit_depth, &color_type, &interlace_type ,NULL,NULL);

  }
  catch(Err & e) {
    if (png_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    throw;
  }
  if (png_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  return iPoint(w, height);
}

/**********************************************************/

ImageR
image_load_png(std::istream & str, const double scale){

  if (!str)  throw Err() << "image_load_png: can't open file";
  if (scale < 1) throw Err() << "image_load_png: wrong scale: " << scale;

  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL, end_info = NULL;

  png_bytep row_buf = NULL;
  ImageR img;

  try {

    png_byte sign[8];
    const char sign_size = 8;
    str.read((char*)sign, sign_size);
    if (str.gcount()!=sign_size ||
        png_sig_cmp(sign, 0, sign_size)!=0)
      throw Err() << "image_load_png: not a PNG file";

    png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
    if (!png_ptr) throw Err() << "image_load_png: can't make png_read_struct";

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) throw Err() << "image_load_png: can't make png_info_struct";

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) throw Err() << "image_load_png: can't make png_info_struct";

    png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), user_error_fn, NULL);

    if (setjmp(png_jmpbuf(png_ptr)))
      throw Err() << "image_load_png: " << libpng_err_msg;

    png_set_read_fn(png_ptr, &str, png_read_data_fn);
    png_set_sig_bytes(png_ptr, sign_size);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 w=0, h=0;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &w, &h,
       &bit_depth, &color_type, &interlace_type ,NULL,NULL);


    // scaled image size
    int w1 = floor((w-1)/scale+1);
    int h1 = floor((h-1)/scale+1);


    // Make image of the correct type
    int cnum = 0;
    if (color_type & PNG_COLOR_MASK_PALETTE){

      if (bit_depth == 16)
        png_set_strip_16(png_ptr);

      if (bit_depth < 8)
        png_set_packing(png_ptr);

      img = ImageR(w1,h1, IMAGE_8PAL);
      png_color *palette;
      png_get_PLTE(png_ptr, info_ptr, &palette, &cnum);

      img.cmap.resize(cnum);
      for (int i=0; i<cnum; i++){
        img.cmap[i] = 0xFF000000 +
          (palette[i].red << 16) +
          (palette[i].green << 8) +
          palette[i].blue;
          //std::cerr << "PAL: " << i << " " << std::hex << img.cmap[i] << "\n";
      }

      // transparent entries for palette
      int num_trans = 0;
      png_byte *trans;
      png_color_16_struct *trans_values;
      png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values);
      if (num_trans > cnum)
        throw Err() << "image_load_png: bad tRNS";
      for (int i=0; i<num_trans; i++){
        int a = trans[i];
        int r = (img.cmap[i] >> 16) & 0xFF;
        int g = (img.cmap[i] >> 8) & 0xFF;
        int b = img.cmap[i] & 0xFF;
        img.cmap[i] = color_argb(a,r,g,b);
        //std::cerr << "TRNS: " << i << " " << std::hex << img.cmap[i] << "\n";
      }
    }

    else if (color_type == PNG_COLOR_TYPE_RGB){
      if (bit_depth == 16)
        png_set_strip_16(png_ptr);

      img = ImageR(w1,h1, IMAGE_24RGB);
    }

    else if (color_type == PNG_COLOR_TYPE_GRAY){
      if (bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

      if (bit_depth <= 8)
        img = ImageR(w1,h1, IMAGE_8);
      else
        img = ImageR(w1,h1, IMAGE_16);
    }

    else {
      img = ImageR(w1,h1, IMAGE_32ARGB);

      if (bit_depth == 16)
        png_set_strip_16(png_ptr);

      if (color_type == PNG_COLOR_TYPE_GRAY ||
          color_type == PNG_COLOR_TYPE_GRAY_ALPHA){
        if (bit_depth < 8)
          png_set_expand_gray_1_2_4_to_8(png_ptr);
        png_set_gray_to_rgb(png_ptr);
      }

      if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
          png_set_tRNS_to_alpha(png_ptr);

      if (!(color_type & PNG_COLOR_MASK_ALPHA))
        png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

    }


    png_read_update_info(png_ptr, info_ptr);
    row_buf = (png_bytep)png_malloc(png_ptr,
        png_get_rowbytes(png_ptr, info_ptr));
    if (!row_buf) throw Err() << "PNG: malloc error";


    /// Main loop

    int line = 0;
    for (int y=0; y<h1; ++y){

      while (line<=rint(y*scale)){
        png_read_row(png_ptr, row_buf, NULL);
        line++;
      }

      if (img.type() == IMAGE_8PAL || img.type() == IMAGE_8){
        for (int x=0; x<w1; ++x){
          int xs = scale==1.0? x:rint(x*scale);
          img.set8(x,y, row_buf[xs]);
        }
      }

      else if (img.type() == IMAGE_24RGB){
        for (int x=0; x<w1; ++x){
          int xs = scale==1.0? x:rint(x*scale);
          uint8_t r = row_buf[3*xs+0];
          uint8_t g = row_buf[3*xs+1];
          uint8_t b = row_buf[3*xs+2];
          img.set24(x,y, (r<<16)+(g<<8)+b);
        }
      }

      else if (img.type() == IMAGE_16){
        for (int x=0; x<w1; ++x){
          int xs = scale==1.0? x:rint(x*scale);
          img.set16(x,y, (row_buf[2*xs]<<8) + row_buf[2*xs+1]);
        }
      }

      else {
        for (int x=0; x<w1; ++x){
          int xs = scale==1.0? x:rint(x*scale);
          uint8_t r = row_buf[4*xs+0];
          uint8_t g = row_buf[4*xs+1];
          uint8_t b = row_buf[4*xs+2];
          uint8_t a = row_buf[4*xs+3];
          img.set32(x,y, color_argb(a,r,g,b));
        }
      }
    }
  }
  catch(Err & e) {
    if (row_buf) png_free(png_ptr, row_buf);
    if (png_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    throw;
  }
  if (row_buf) png_free(png_ptr, row_buf);
  if (png_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  return img;
}

/*
  TODO: Interlaced images support?
  if (interlace_type == PNG_INTERLACE_ADAM7){
    // Interlaced images
    // pass 1: scale 1/8
    // passes 1..3: scale 1/4
    // passes 1..5: scale 1/2

    // PNG_INTERLACE_ADAM7:
    // Xooooooo ooooXooo oooooooo ooXoooXo oooooooo oXoXoXoX oooooooo
    // oooooooo oooooooo oooooooo oooooooo oooooooo oooooooo XXXXXXXX
    // oooooooo oooooooo oooooooo oooooooo XoXoXoXo oXoXoXoX oooooooo
    // oooooooo oooooooo oooooooo oooooooo oooooooo oooooooo XXXXXXXX
    // oooooooo oooooooo XoooXooo ooXoooXo oooooooo oXoXoXoX oooooooo
    // oooooooo oooooooo oooooooo oooooooo oooooooo oooooooo XXXXXXXX
    // oooooooo oooooooo oooooooo oooooooo XoXoXoXo oXoXoXoX oooooooo
    // oooooooo oooooooo oooooooo oooooooo oooooooo oooooooo XXXXXXXX
  }
*/


/**********************************************************/

void
png_write_data_fn(png_structp png_ptr, png_bytep data, size_t length){
  auto str = (std::ostream *) png_get_io_ptr(png_ptr);
  str->write((char*)data, length);
}

void
png_flush_data_fn(png_structp png_ptr){
  auto str = (std::ostream *) png_get_io_ptr(png_ptr);
  str->flush();
}

/**********************************************************/

void
image_save_png(const ImageR & im, std::ostream & str,
               const Opt & opt){

  if (!str) throw Err() << "image_save_png: can't open file";

  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_bytep buf = NULL;

  try {

    // Choose default PNG color type
    int color_type;
    int bits = 8; // only 8-bit colors are supported now
    switch (im.type()){
      case IMAGE_32ARGB: color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
      case IMAGE_24RGB:  color_type = PNG_COLOR_TYPE_RGB;  break;
      case IMAGE_16:     color_type = PNG_COLOR_TYPE_GRAY; bits = 16; break;
      case IMAGE_8:      color_type = PNG_COLOR_TYPE_GRAY; break;
      case IMAGE_1:      color_type = PNG_COLOR_TYPE_PALETTE; break;
      case IMAGE_8PAL:   color_type = PNG_COLOR_TYPE_PALETTE; break;
      case IMAGE_FLOAT:  color_type = PNG_COLOR_TYPE_RGB;  break;
      case IMAGE_DOUBLE: color_type = PNG_COLOR_TYPE_RGB;  break;
      case IMAGE_UNKNOWN: color_type = PNG_COLOR_TYPE_RGB;  break;
    }

    // set PNG color type from options
    std::string s;
    s = opt.get("png_format", "");
    if (s != "") {
      if      (s == "argb")  {color_type = PNG_COLOR_TYPE_RGB_ALPHA;  bits=8;}
      else if (s == "rgb")   {color_type = PNG_COLOR_TYPE_RGB;        bits=8;}
      else if (s == "grey")  {color_type = PNG_COLOR_TYPE_GRAY;       bits=8;}
      else if (s == "agrey") {color_type = PNG_COLOR_TYPE_GRAY_ALPHA; bits=8;}
      else if (s == "pal")   {color_type = PNG_COLOR_TYPE_PALETTE;    bits=8;}
      else throw Err() << "image_save_png: unknown png_format setting: " << s << "\n";
    }

    // png palette
    ImageR im8 = im;
    if (s == "pal"){
      std::vector<uint32_t> colors = image_colormap(im, opt);
      im8 = image_remap(im, colors);
    }

    png_ptr = png_create_write_struct
      (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
      throw Err() << "image_save_png: can't make png_read_struct";

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
      throw Err() << "image_save_png: can't make png_info_struct";

    png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), user_error_fn, NULL);

    if (setjmp(png_jmpbuf(png_ptr)))
      throw Err() << "image_save_png: " << libpng_err_msg;

    png_set_write_fn(png_ptr, &str, png_write_data_fn, png_flush_data_fn);

    png_set_IHDR(png_ptr, info_ptr, im.width(), im.height(),
       bits, color_type, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (color_type == PNG_COLOR_TYPE_PALETTE){

      if (im8.type()==IMAGE_1){
        im8.cmap.resize(2);
        im8.cmap[0]=0xFFFFFFFF;
        im8.cmap[1]=0xFF000000;
      }
      else if (im8.type()!=IMAGE_8PAL)
        throw Err() << "image_save_png: inconsistent palette setting";
      int cnum = im8.cmap.size();

      png_color pcolors[cnum];
      for (int i=0; i<cnum; i++){
       uint32_t c = color_rem_transp(im8.cmap[i], false);
        pcolors[i].red   = (c >>16) & 0xFF;
        pcolors[i].green = (c >>8)  & 0xFF;
        pcolors[i].blue  =  c & 0xFF;
      }
      png_set_PLTE(png_ptr, info_ptr, pcolors, cnum);

      //transparent palette colors
      png_byte trans[cnum];
      bool is_transp = false;
      for (int i=0; i<cnum; i++){
        trans[i] = (im8.cmap[i]>>24) & 0xFF;
        if (trans[i] != 0xFF) is_transp = true;
      }
      if (is_transp) png_set_tRNS(png_ptr, info_ptr, trans, cnum, 0);
    }

    png_write_info(png_ptr, info_ptr);
    // for 16-bit RGB this will not be enough!
    png_bytep buf = (png_bytep)png_malloc(png_ptr, im.width()*4);
    if (!buf) throw Err() << "PNG: malloc error";

    for (size_t y=0; y<im.height(); y++){
      for (size_t x=0; x<im.width(); x++){
        uint32_t c;
        switch (color_type){
        case PNG_COLOR_TYPE_GRAY:
          if (bits==8)
            buf[x] = im.get_grey8(x, y);
          else {
            // swap bytes
            uint16_t c = im.get_grey16(x, y);
            ((uint16_t*)buf)[x] = ((c>>8)&0xFF) | ((c<<8)&0xFF00);
          }
          break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
          ((uint16_t*)buf)[x]  = im.get_agrey8(x, y);
          break;
        case PNG_COLOR_TYPE_RGB:
          c = im.get_rgb(x, y);
          buf[3*x]   = (c >> 16) & 0xFF;
          buf[3*x+1] = (c >> 8) & 0xFF;
          buf[3*x+2] = c & 0xFF;
          break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
          c = im.get_argb(x, y);
          buf[4*x+3] = (c>>24) & 0xFF;
          c = color_rem_transp(c, false); // unscale color!
          buf[4*x]   = (c >> 16) & 0xFF;
          buf[4*x+1] = (c >> 8) & 0xFF;
          buf[4*x+2] = c & 0xFF;
          break;
        case PNG_COLOR_TYPE_PALETTE:
          if (im8.type() == IMAGE_8PAL)
            buf[x] = im8.get8(x,y);
          else if (im8.type() == IMAGE_1)
            buf[x] = (uint8_t)im8.get1(x,y);
          break;
        }
      }
      png_write_row(png_ptr, buf);
    }

    png_write_end(png_ptr, info_ptr);
  }
  catch (Err & e) {
    if (buf)     png_free(png_ptr, buf);
    if (png_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
    throw;
  }
  if (buf)     png_free(png_ptr, buf);
  if (png_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
}

/**********************************************************/

iPoint
image_size_png(const std::string & fname){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  iPoint ret;
  try { ret = image_size_png(str); }
  catch(Err & e){ e << ": " << fname; throw;}
  return ret;
}

ImageR
image_load_png(const std::string & fname, const double scale){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  ImageR ret;
  try { ret = image_load_png(str, scale); }
  catch(Err & e){ e << ": " << fname; throw;}
  return ret;
}

void
image_save_png(const ImageR & im, const std::string & fname,
               const Opt & opt){
  std::ofstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  try { image_save_png(im, str, opt); }
  catch(Err & e){ e << ": " << fname; throw;}
}
