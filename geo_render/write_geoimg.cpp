#include "cairo/cairo_wrapper.h"
#include "filename/filename.h"
#include "geo_data/geo_io.h"
#include "geo_data/conv_geo.h"
#include "write_geoimg.h"
#include "image/image_t.h"       // ImageT::make_url
#include "geo_mkref/geo_mkref.h" // for tiled maps
#include "geom/poly_tools.h"     // for rect_in_polygon
#include "geo_tiles/geo_tiles.h"


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
  opts.add("tmap", 0,0,g,
    "Write tiled map. In this case `fname` parameter should contain a template "
    "with {x}, {y}, {z} fields. Output of map file is not supported at "
    "the moment. `mkref` options are ignored.");
  opts.add("zmin", 1,0,g, "Min zoom for tiled maps.");
  opts.add("zmax", 1,0,g, "Max zoom for tiled maps.");
}

#define TMAP_TILE_SIZE 256
void
write_geoimg(const std::string & fname, GObj & obj, const GeoMap & ref, const Opt & opts){

  // process tiled maps
  if (opts.exists("tmap")){
    // calculate zoom range
    int zmin = opts.get("zmin", 0);
    int zmax = opts.get("zmax", 0);

    // For rendering tiles we do not need to use the user-supply reference.
    // The problem is that we may want to have border, which is normally
    // comes with the reference.

    // There is a workaround: to process border options separately, in the
    // same way as it is done in geo_mkref().

    dMultiLine brd;
    // if there is a reference:
    if (ref.ref.size()) {
      ConvMap cnv0(ref); // conversion original map->wgs
      brd = cnv0.frw_acc(ref.border); // -> wgs
    }

    // update border from optons
    geo_mkref_brd(opts, brd);

    GeoTiles tcalc;  // tile calculator

    // Build new options for write_geoimg. We do not need "tmap", "map",
    // "skip_image" options, only "fmt" my be useful.
    Opt o;
    if (opts.exists("fmt"))     o.put("fmt",     opts.get("fmt"));
    if (opts.exists("bgcolor")) o.put("bgcolor", opts.get("bgcolor"));

    // for each zoom level
    for (int z = zmin; z<=zmax; z++){
      iRect tiles = tcalc.range_to_gtiles(obj.bbox(),z); // tile range


      // render tiles
      for (int y = tiles.y; y < tiles.y + tiles.h; y++){
        for (int x = tiles.x; x < tiles.x + tiles.w; x++) {
          iPoint tile(x,y,z);
          std::string f = ImageT::make_url(fname, tile);
          dRect trange_wgs = tcalc.gtile_to_range(tile, z); // Google, not TMS tiles
          if (brd.size() && rect_in_polygon(trange_wgs, brd) == 0) continue;

          // make reference for the tile (similar to code in geo_mkref) 
          GeoMap r;
          r.proj = "WEB";
          r.image_size = iPoint(1,1)*TMAP_TILE_SIZE;
          dLine pts_w = rect_to_line(trange_wgs, false);
          dLine pts_r = rect_to_line(dRect(dPoint(0,0),r.image_size), false);
          pts_r.flip_y(r.image_size.y);
          r.add_ref(pts_r, pts_w);

          // convert border to pixel coordinates of this tile
          ConvMap cnv1(r);
          r.border = cnv1.bck_acc(brd);
          write_geoimg(f, obj, r, o);
        }
      }
    }
    return;
  }


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
  obj.set_opt(opts);
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
  if (ref.border.size()) {
    cr->mkpath_smline(ref.border, true, 0);
    cr->clip();
  }

  // draw tracks and waypoints
  obj.draw(cr, box);

  // write raster formats
  if (fmt == "png" || fmt=="jpeg" || fmt=="tiff" || fmt=="gif"){
    Opt o(opts);
    o.put("img_out_fmt", fmt);
    image_save(img, fname, o);
  }

}

