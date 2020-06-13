#include <fstream>
#include "filename/filename.h"

// Because of setjmp.h problem, one should include
// io_png.h before image_jpeg.h
#include "io_png.h"
#include "io_jpeg.h"
#include "io_tiff.h"
#include "io_gif.h"

#include "io.h"
#include "image_colors.h"

/**********************************************************/
void ms2opt_add_image(GetOptSet & opts){
  const char *g = "IMAGE";
  opts.add("img_out_fmt", 1,0, g,
    "Explicitely set image format: jpeg, png, gif, tiff");
  opts.add("tiff_format", 1,0, g,
    "When writing TIFF, convert image to one of following forms: "
    "argb, rgb, grey, pal (default depends on the image).");
  opts.add("tiff_minwhite", 1,0, g,
    "When writing greyscale TIFF, use MINISWHITE colors (default: 0).");
  opts.add("jpeg_quality", 1,0, g,
    "Set JPEG quality (default 95).");
  opts.add("png_format", 1,0, g,
    "When writing PNG, convert image to one of following forms: "
    "argb, rgb, grey, agrey, pal (default depends on the image).");
  ms2opt_add_image_cmap(opts);
}
/**********************************************************/

std::string
image_ext_fmt(const std::string & fname){
  if      (file_ext_check(fname, ".jpeg")) return "jpeg";
  else if (file_ext_check(fname, ".jpg"))  return "jpeg";
  else if (file_ext_check(fname, ".png"))  return "png";
  else if (file_ext_check(fname, ".gif"))  return "gif";
  else if (file_ext_check(fname, ".tif"))  return "tiff";
  else if (file_ext_check(fname, ".tiff")) return "tiff";
  return "";
}

std::string
image_stream_fmt(std::istream & str){
  // Read first 3 bytes and detect format:
  // see https://en.wikipedia.org/wiki/List_of_file_signatures
  /// tiff 49 49 2A 00
  /// tiff 4D 4D 00 2A
  /// jpeg FF D8 FF
  /// png  89 50 4E 47
  /// gif 47 49 46
  unsigned char buf[3];
  str.read((char *)buf,3);
  str.seekg(std::ios_base::beg);

  if (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF) return "jpeg";
  if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E) return "png";
  if ((buf[0] == 0x49 && buf[1] == 0x49 && buf[2] == 0x2A) ||
      (buf[0] == 0x4D && buf[1] == 0x4D && buf[2] == 0x00)) return "tiff";
  if (buf[0] == 0x47 && buf[1] == 0x49 && buf[2] == 0x46) return "gif";
  return "";
}

std::string
image_file_fmt(const std::string & fname){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  return image_stream_fmt(str);
}

/**********************************************************/

iPoint
image_size(const std::string & fname, const Opt & opts){
  std::string fmt = image_file_fmt(fname);
  if (fmt == "jpeg") return image_size_jpeg(fname);
  if (fmt == "png")  return image_size_png(fname);
  if (fmt == "gif")  return image_size_gif(fname);
  if (fmt == "tiff") return image_size_tiff(fname);
  throw Err() << "image_size: unknown format: " << fname;
}

// load the whole image 
ImageR
image_load(const std::string & fname, const double scale, const Opt & opts){
  std::string fmt = image_file_fmt(fname);
  if (fmt == "jpeg") return image_load_jpeg(fname, scale);
  if (fmt == "png")  return image_load_png(fname, scale);
  if (fmt == "gif")  return image_load_gif(fname, scale);
  if (fmt == "tiff") return image_load_tiff(fname, scale);
  throw Err() << "image_load: unknown format: " << fname;
}

ImageR
image_load(std::istream & str, const double scale, const Opt & opt){
  std::string fmt = image_stream_fmt(str);
  if (fmt == "jpeg") return image_load_jpeg(str, scale);
  if (fmt == "png")  return image_load_png(str, scale);
  if (fmt == "tiff") return image_load_tiff(str, scale);
  if (fmt == "gif")
    throw Err() << "image_load: loading GIF files from stream is not supported";
  throw Err() << "image_load: unknown image format";
}


// save the whole image
void
image_save(const ImageR & im, const std::string & fname, const Opt & opts){
  std::string fmt = image_ext_fmt(fname);
  if (opts.get("img_out_fmt","") != "") fmt = opts.get("img_out_fmt", "");

  if (fmt == "jpeg") return image_save_jpeg(im, fname, opts);
  if (fmt == "png")  return image_save_png(im, fname, opts);
  if (fmt == "gif")  return image_save_gif(im, fname, opts);
  if (fmt == "tiff") return image_save_tiff(im, fname, opts);

  throw Err() << "image_save: unknown format: " << fname;
}

