#include "cairo/cairo_wrapper.h"
#include "filename/filename.h"
#include "geo_data/geo_io.h"
#include "geo_data/conv_geo.h"
#include "write_geoimg.h"


void
ms2opt_add_geoimg(GetOptSet & opts){
  ms2opt_add_out(opts);
  ms2opt_add_image(opts);
  ms2opt_add_ozimap_o(opts);

  const char *g = "IMAGE";
  opts.remove("img_in_fmt");
  opts.remove("img_out_fmt");
  opts.add("out_fmt", 1,0,g,
    "Explicitely set image output format: pdf, ps, svg, png, jpeg, tiff, gif. "
    "By default format is determined from the output file extension.");
  opts.add("bgcolor", 1,0,g,
    "Image background color (default 0xFFFFFFFF).");
  opts.add("map", 1,'m',g,
    "Write map file in OziExprorer format (by default it is not written).");
  opts.add("skip_image", 0,0,g,
    "Do not write image file (can be used if only the map file is needed). "
    "Option -o <file> should be provided anyway, the filename "
    "will be written to the map-file.");
}

void
write_geoimg(const std::string & fname, GObj & obj, const GeoMap & ref, const Opt & opts){

  std::string fmt;
  if      (file_ext_check(fname, ".pdf"))  fmt="pdf";
  else if (file_ext_check(fname, ".ps"))   fmt="ps";
  else if (file_ext_check(fname, ".svg"))  fmt="svg";
  else if (file_ext_check(fname, ".png"))  fmt="png";
  else if (file_ext_check(fname, ".jpg"))  fmt="jpeg";
  else if (file_ext_check(fname, ".jpeg")) fmt="jpeg";
  else if (file_ext_check(fname, ".tif"))  fmt="tiff";
  else if (file_ext_check(fname, ".tiff")) fmt="tiff";
  else if (file_ext_check(fname, ".gif"))  fmt="gif";

  if (opts.get("out_fmt","") != "") fmt = opts.get("out_fmt", "");


  // write map file
  if (opts.exists("map")){
    GeoMap r(ref);
    r.image = fname;
    write_ozi_map(opts.get("map","").c_str(), r, opts);
  }

  // exit if --skip_image option exists
  if (opts.exists("skip_image")){ return; }

  std::shared_ptr<ConvMap> cnv(new ConvMap(ref));
  obj.set_opt(std::shared_ptr<Opt>(new Opt(opts)));
  obj.set_cnv(cnv);

  // find image dimensions
  dRect box = dRect(dPoint(), (dPoint)ref.image_size);
  if (box.is_zsize()) box = ref.border.bbox();
  if (box.is_zsize()) throw Err() << "write_img: can't get map dimensions";

  // setup cairo context
  CairoWrapper cr;
  ImageR img;
  int w=box.brc().x, h=box.brc().y;
  if      (fmt == "pdf") cr.set_surface_pdf(fname.c_str(), w,h);
  else if (fmt == "ps")  cr.set_surface_ps(fname.c_str(), w,h);
  else if (fmt == "svg") cr.set_surface_svg(fname.c_str(), w,h);
  else if (fmt == "png" || fmt=="jpeg" || fmt=="tiff" || fmt=="gif"){
    img = ImageR(w,h,IMAGE_32ARGB);
    img.fill32(0);
    cr.set_surface_img(img);
  }
  else
    throw Err(-2) << "Can't determine output format for file: " << fname;

  // fill the image with bgcolor
  cr->set_color_a(opts.get<int>("bgcolor", 0xFFFFFFFF));
  cr->paint();

  // draw tracks and waypoints
  obj.draw(cr, box);

  // write raster formats
  if (fmt == "png" || fmt=="jpeg" || fmt=="tiff" || fmt=="gif"){
    Opt o(opts);
    o.put("img_out_fmt", fmt);
    image_save(img, fname, o);
  }

}

