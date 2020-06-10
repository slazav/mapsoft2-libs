#include "image_jpeg.h"
#include <iostream>
#include <fstream>
#include <jpeglib.h>
#include <stdio.h>
#include <cstring>

std::string jpeg_error_message;

/**********************************************************/

// error prefix
std::string error_pref = "jpeg";

// custom error handler
void
my_error_exit (j_common_ptr cinfo) {
  const char buffer[JMSG_LENGTH_MAX] = "";
  (*cinfo->err->format_message) (cinfo, (char *)buffer);
  throw Err() << error_pref << ": " << buffer;
}

/**********************************************************/
// reading JPEG from std::istream
// see example in jdatasrc.c from libjpeg

#define INPUT_BUF_SIZE 4096

typedef struct {
  struct jpeg_source_mgr pub; /* public fields */
  std::istream * str;
  JOCTET * buf;
} my_source_mgr;


void
init_source(j_decompress_ptr cinfo) {
  auto data = (my_source_mgr *)(cinfo->src);
  data->str->seekg(std::ios_base::beg);
}

boolean
fill_input_buffer(j_decompress_ptr cinfo) {
  auto data = (my_source_mgr *)(cinfo->src);
  data->str->read((char *)data->buf, INPUT_BUF_SIZE*sizeof(JOCTET));
  size_t n = data->str->gcount();
  if ( n == 0){
    /* Insert a fake EOI marker */
    data->buf[0] = (JOCTET) 0xFF;
    data->buf[1] = (JOCTET) JPEG_EOI;
    n = 2;
  }
  data->pub.next_input_byte = data->buf;
  data->pub.bytes_in_buffer = n;
//  data->start_of_file = false;
  return true;
}

void
skip_input_data(j_decompress_ptr cinfo, long count) {
  if (count<1) return;
  auto data = (my_source_mgr *)(cinfo->src);
  if (count<data->pub.bytes_in_buffer){
    data->pub.bytes_in_buffer -= count;
    data->pub.next_input_byte += count;
    return;
  }
  count -= data->pub.bytes_in_buffer;
  data->str->seekg(count, std::ios_base::cur);
  data->pub.bytes_in_buffer = 0;
  data->pub.next_input_byte = data->buf;
  return;
}

void
term_source (j_decompress_ptr cinfo) { }

void
jpeg_stream_src (j_decompress_ptr cinfo, std::istream* str){
  my_source_mgr * src;

  if (cinfo->src == NULL) { /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
      sizeof(my_source_mgr));
    src = (my_source_mgr *) cinfo->src;
    src->buf = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
      INPUT_BUF_SIZE * sizeof(JOCTET));
  }
  src = (my_source_mgr *) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data   = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* default */
  src->pub.term_source = term_source;
  src->str = str;
  src->pub.bytes_in_buffer = 0;
  src->pub.next_input_byte = NULL;
}

/**********************************************************/
iPoint
image_size_jpeg(std::istream & str){

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  // open file, get image size
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = my_error_exit;
  error_pref = "image_size_jpeg";
  // note: it is an error to do jpeg_destroy_decompress
  // before jpeg_create_decompress.
  jpeg_create_decompress(&cinfo);

  try {
    jpeg_stream_src(&cinfo, &str);
    jpeg_read_header(&cinfo, TRUE);
    throw Err();
  }
  catch (Err e){
    jpeg_destroy_decompress(&cinfo);
    if (e.str() != "") throw e;
  }
  return iPoint(cinfo.image_width, cinfo.image_height);
}

/**********************************************************/

ImageR
image_load_jpeg(std::istream & str, const double scale){

  if (scale < 1)
    throw Err() << "image_load_jpeg: wrong scale: " << scale;

  unsigned char *buf = NULL;
  ImageR img;
  struct jpeg_decompress_struct cinfo;
  jpeg_create_decompress(&cinfo);
  try {

    jpeg_stream_src(&cinfo, &str);

    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_error_exit;
    error_pref = "image_load_jpeg";

    jpeg_read_header(&cinfo, TRUE);

    int denom;
    if (scale <2) denom = 1;
    else if (scale <4)  denom = 2;
    else if (scale <8)  denom = 4;
    else denom = 8;
    double sc = scale/denom;

    cinfo.out_color_space = JCS_RGB; // always load in RGB mode
    cinfo.scale_denom = denom; // set denominator
    jpeg_start_decompress(&cinfo);

    // scaled image
    int w = cinfo.output_width;
    int h = cinfo.output_height;
    int w1 = floor((cinfo.image_width-1)/scale+1);
    int h1 = floor((cinfo.image_height-1)/scale+1);
    img = ImageR(w1,h1, IMAGE_24RGB);
    // adjust scale
    sc = std::min((double)(w-1)/(w1-1), (double)(h-1)/(h1-1));

    // main loop

    if (0 && w==w1 && h==h1){
      for (int y=0; y<h; ++y){
        JSAMPLE *sbuf = img.data() + 3*y*w1;
        jpeg_read_scanlines(&cinfo, &sbuf, 1);
      }
    }
    else {
      // memory buffer
      buf  = new unsigned char[(w+1)*3];

      int line = 0;
      for (int y=0; y<h1; ++y){
        while (line<=rint(y*sc) && line<h){
          jpeg_read_scanlines(&cinfo, (JSAMPLE**)&buf, 1);
          line++;
        }
        uint8_t *dst_buf = img.data() + 3*y*w1;
        for (int x=0; x<w1; ++x){
          int xs3 = 3*rint(x*sc);
          memcpy(dst_buf + 3*x, buf + xs3, 3);
        }
      }
    }
    throw Err();
  }
  catch (Err e){
    if (buf) delete[] buf;
    jpeg_abort_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    if (e.str() != "") throw e;
  }
  return img;
}

/**********************************************************/
// writing JPEG to std::istream

#define OUTPUT_BUF_SIZE 4096

typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */
  std::ostream * str;
  JOCTET * buf;
} my_dest_mgr;

void
init_dest(j_compress_ptr cinfo) {
  auto data = (my_dest_mgr *)(cinfo->dest);
  data->pub.next_output_byte = data->buf;
  data->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}


boolean
empty_output_buffer (j_compress_ptr cinfo){
  auto data = (my_dest_mgr *)(cinfo->dest);
  data->str->write((char*)data->buf, OUTPUT_BUF_SIZE*sizeof(JOCTET));
  data->pub.next_output_byte = data->buf;
  data->pub.free_in_buffer = OUTPUT_BUF_SIZE;
  return true;
}

void
term_dest (j_compress_ptr cinfo) {
  auto data = (my_dest_mgr *)(cinfo->dest);
  size_t n = OUTPUT_BUF_SIZE - data->pub.free_in_buffer;
  data->str->write((char*)data->buf, n*sizeof(JOCTET));
  data->pub.next_output_byte = data->buf;
  data->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

void
jpeg_stream_dest (j_compress_ptr cinfo, std::ostream* str){
  my_dest_mgr * dest;

  if (cinfo->dest == NULL) { /* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
      sizeof(my_dest_mgr));
    dest = (my_dest_mgr *) cinfo->dest;
    dest->buf = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
      OUTPUT_BUF_SIZE * sizeof(JOCTET));
  }
  dest = (my_dest_mgr *) cinfo->dest;
  dest->pub.init_destination = init_dest;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_dest;
  dest->str = str;
  dest->pub.next_output_byte = dest->buf;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

/**********************************************************/

void
image_save_jpeg(const ImageR & im, std::ostream & str, const Opt & opt){

  int quality = opt.get("jpeg_quality", 95);

  if ((quality<0)||(quality>100))
      throw Err() << "image_save_jpeg: quality "<< quality << " not in range 0..100";

  unsigned char *buf = NULL;
  std::string msg;

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  error_pref = "image_save_jpeg";
  jpeg_create_compress(&cinfo);

  try {

    jpeg_stream_dest(&cinfo, &str);
    cinfo.image_width = im.width();
    cinfo.image_height = im.height();
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality (&cinfo, quality, true);
    jpeg_start_compress(&cinfo, TRUE);

    buf = new unsigned char[im.width() * 3];

    for (int y = 0; y < im.height(); y++){
      for (int x = 0; x < im.width(); x++){
        int c = im.get_rgb(x, y);
        buf[3*x+2] = c & 0xFF;
        buf[3*x+1] = (c >> 8) & 0xFF;
        buf[3*x]   = (c >> 16) & 0xFF;
      }
      jpeg_write_scanlines(&cinfo, (JSAMPLE**)&buf, 1);
    }
    jpeg_finish_compress(&cinfo);

    throw Err();
  }
  catch (Err e){
    if (buf) delete [] buf;
    jpeg_destroy_compress(&cinfo);
    if (e.str() != "") throw e;
  }
}

/**********************************************************/
iPoint
image_size_jpeg(const std::string & fname){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  iPoint ret;
  try { ret = image_size_jpeg(str); }
  catch(Err e){ throw Err() << e.str() << ": " << fname; }
  return ret;
}

ImageR
image_load_jpeg(const std::string & fname, const double scale){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  ImageR ret;
  try { ret = image_load_jpeg(str, scale); }
  catch(Err e){ throw Err() << e.str() << ": " << fname; }
  return ret;
}

void
image_save_jpeg(const ImageR & im, const std::string & fname,
               const Opt & opt){
  std::ofstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  try { image_save_jpeg(im, str, opt); }
  catch(Err e){ throw Err() << e.str() << ": " << fname; }
}
