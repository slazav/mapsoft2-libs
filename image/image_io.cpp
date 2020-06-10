#include "filename/filename.h"

// Because of setjmp.h problem, one should include
// io_png.h before image_jpeg.h
#include "io_png.h"
#include "io_jpeg.h"
#include "io_tiff.h"
#include "io_gif.h"

#include "image_io.h"
#include "image_colors.h"

/**********************************************************/
void ms2opt_add_image(GetOptSet & opts){
  const char *g = "IMAGE";
  opts.add("img_in_fmt", 1,0, g,
    "Explicitely set image format: jpeg, png, gif, tiff");
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
image_ext_to_fmt(const std::string & fname){
  if      (file_ext_check(fname, ".jpeg")) return "jpeg";
  else if (file_ext_check(fname, ".jpg"))  return "jpeg";
  else if (file_ext_check(fname, ".png"))  return "png";
  else if (file_ext_check(fname, ".gif"))  return "gif";
  else if (file_ext_check(fname, ".tif"))  return "tiff";
  else if (file_ext_check(fname, ".tiff")) return "tiff";
  return "";
}


iPoint
image_size(const std::string & fname, const Opt & opts){
  std::string fmt = image_ext_to_fmt(fname);
  if (opts.get("img_in_fmt","") != "") fmt = opts.get("img_in_fmt", "");

  if (fmt == "jpeg") return image_size_jpeg(fname);
  if (fmt == "png")  return image_size_png(fname);
  if (fmt == "gif")  return image_size_gif(fname);
  if (fmt == "tiff") return image_size_tiff(fname);

  throw Err() << "image_size: unknown format: " << fname;
  return iPoint(0,0);
}


// load the whole image 
ImageR
image_load(const std::string & fname, const double scale, const Opt & opts){
  std::string fmt = image_ext_to_fmt(fname);
  if (opts.get("img_in_fmt","") != "") fmt = opts.get("img_in_fmt", "");

  if (fmt == "jpeg") return image_load_jpeg(fname, scale);
  if (fmt == "png")  return image_load_png(fname, scale);
  if (fmt == "gif")  return image_load_gif(fname, scale);
  if (fmt == "tiff") return image_load_tiff(fname, scale);

  throw Err() << "image_load: unknown format: " << fname;
  return ImageR();
}


// save the whole image
void
image_save(const ImageR & im, const std::string & fname, const Opt & opts){
  std::string fmt = image_ext_to_fmt(fname);
  if (opts.get("img_out_fmt","") != "") fmt = opts.get("img_out_fmt", "");

  if (fmt == "jpeg") return image_save_jpeg(im, fname, opts);
  if (fmt == "png")  return image_save_png(im, fname, opts);
  if (fmt == "gif")  return image_save_gif(im, fname, opts);
  if (fmt == "tiff") return image_save_tiff(im, fname, opts);

  throw Err() << "image_save: unknown format: " << fname;
}

