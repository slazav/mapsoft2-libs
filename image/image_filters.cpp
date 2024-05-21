#include <vector>

void ms2opt_add_image_flts(GetOptSet & opts){
  const char *g = "IMAGE_FLTS";
  opts.add("flt_border", 1,0, g,
    "Border width for filter calculations, pixels, default 50");
  opts.add("autolevel", 1,0, g,
    "Adjust color levels, reduce from 16-bit to 8-bit, invert negative. "
    "This is similar to my olt tool 1628 which I use to process scanned negatives. "
    "Argument is array of three integers from 1 to 254, levels for red, green, blue "
    "colors (first one is also used for gray images). "
    "Color curves are adjusted to have 0 at the darkest color, 256 at the "

  opts.add("autolevel_r", 1,0, g,
    "Red value for --autolevel, 1-254, default 64, also used for grey images.");
  opts.add("autolevel_g", 1,0, g,
    "Green value for --autolevel, 1-254, default 64.");
  opts.add("autolevel_thr", 1,0, g,
    "Green value for --autolevel, 1-254, default 64.");

  opts.add("autolevel_inv", 1,0, g,
    "Invert image, default 1");
}

ImageR image_autolevel(const ImageR & img){

  int brd = opt.get<uint32_t>("flt_border", 50);
  std::vector<int>

str_to_type_ivec

  if (img.type() == IMAGE_32ARGB) return img;
  ImageR ret(img.width(), img.height(), IMAGE_32ARGB);
  for (size_t x=0; x<img.width(); ++x){
    for (size_t y=0; y<img.height(); ++y){
      ret.set32(x,y, img.get_argb(x,y));
    }
  }
  return ret;
}
