#include "cairo/cairo_wrapper.h"
#include "filename/filename.h"
#include "geo_data/geo_io.h"
#include "geo_data/conv_geo.h"
#include "write_geoimg.h"
#include "image/image_t.h"       // ImageT::make_url
#include "image/image_colors.h"
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
  opts.add("add", 0,0,g,
    "If image file exists, read it and use as the background. "
    "The image should have same dimensions.");
  opts.add("bgcolor", 1,0,g,
    "Image background color (default 0xFFFFFFFF).");
  opts.add("map", 1,'m',g,
    "Write map file in OziExprorer format (by default it is not written).");
  opts.add("skip_image", 0,0,g,
    "Do not write image file (can be used if only the map file is needed). "
    "Option -o <file> should be provided anyway, the filename "
    "will be written to the map-file.");
  opts.add("title", 1,0,g, "write text in the top-left corner.");
  opts.add("tmap", 0,0,g,
    "Write tiled map. In this case `fname` parameter should contain a template "
    "with {x}, {y}, {z} fields. Output of map file is not supported at "
    "the moment. `mkref` options are ignored.");
  opts.add("zmin", 1,0,g, "Min zoom for tiled maps.");
  opts.add("zmax", 1,0,g, "Max zoom for tiled maps.");
  opts.add("tmap_scale", 1,0,g,
    "When creating tile map with multiple zoom levels scale larger tiles to "
    "create smaller ones (instead of rendering all tiles separately). Default: 0");
}

#define TMAP_TILE_SIZE 256

void
write_tiles(const std::string & fname, GObj & obj, const GeoMap & ref, const Opt & opts){
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
  GeoTiles tcalc;  // tile calculator

  // Build new options for write_geoimg. We do not need "tmap", "map",
  // "skip_image", "border_*" options.
  Opt o(opts);
  o.erase("tmap");
  o.erase("map");
  o.erase("skip_image");
  o.erase("tmap_scale");
  o.erase("border_wgs");
  o.erase("border_file");

  // for each zoom level
  for (int z = zmax; z>=zmin; --z){
    iRect tiles = tcalc.range_to_gtiles(obj.bbox(),z); // tile range

    // render tiles
    for (int y = tiles.y; y < tiles.y + tiles.h; y++){
      for (int x = tiles.x; x < tiles.x + tiles.w; x++) {
        iPoint tile(x,y,z);
        dRect trange_wgs = tcalc.gtile_to_range(tile, z); // Google, not TMS tiles
        if (brd.size() && rect_in_polygon(trange_wgs, brd) == 0) continue;

        // render tile in a normal way
        if (z == zmax || !opts.get("tmap_scale", false)){
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

          // write to file
          std::string f = ImageT::make_url(fname, tile);
          write_geoimg(f, obj, r, o);
        }

        // collect the tile from four larger tiles
        else {
          size_t w = tcalc.get_tsize();
          ImageR img(w, w, IMAGE_32ARGB);
          img.fill32(0);
          for (int t=0; t<4; t++){
            iPoint src_tile(2*x + t%2, 2*y + t/2, z+1);
            std::string src_f = ImageT::make_url(fname, src_tile);
            if (!file_exists(src_f)) continue;

            ImageR src_img = image_load(src_f);
            if (src_img.width() != w || src_img.height()!=w)
              throw Err() << "wrong tile size: " << src_f << ": " << src_img; 
            for (size_t y1 = 0; y1<w/2; ++y1){
              for (size_t x1 = 0; x1<w/2; ++x1){
                // calculate 4-point average for all 4 color components
                int cc[4] = {0,0,0,0};
                for (int t1 = 0; t1<4; t1++){
                  uint32_t c =  src_img.get_argb(2*x1 + t1%2, 2*y1 + t1/2);
                  for (int i = 0; i<4; ++i) cc[i] += ((c>>(8*i)) & 0xff);
                }
                uint32_t c = 0;
                for (int i = 0; i<4; ++i)
                  c += ((cc[i]/4) & 0xff) << (8*i);

                img.set32(x1 + (t%2)*w/2, y1 + (t/2)*w/2, c);
              }
            }
          }
          std::string f = ImageT::make_url(fname, tile);
          image_save(img, f, o);
        }
      }
    }
  }
}

void
write_geoimg(const std::string & fname, GObj & obj, const GeoMap & ref, const Opt & opts){

  // process tiled maps
  if (opts.exists("tmap")){
    write_tiles(fname, obj, ref, opts);
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

  // create background image
  uint32_t bg = opts.get<int>("bgcolor", 0xFFFFFFFF);
  bool raster = (fmt == "png" || fmt=="jpeg" || fmt=="tiff" || fmt=="gif");
  if (raster) {
    if (opts.exists("add") && file_exists(fname)){
      Opt o;
      o.put("img_in_fmt", fmt);
      img = image_to_argb(image_load(fname, 1, o));
      if (img.height()!=h || img.width()!=w)
        throw Err() << "Wrong image dimensions: " << fname << ": " << img;
    }
    else {
      img = ImageR(w,h,IMAGE_32ARGB);
      img.fill32(bg);
    }
    cr.set_surface_img(img);
  }
  else if (fmt == "pdf") cr.set_surface_pdf(fname.c_str(), w,h);
  else if (fmt == "ps")  cr.set_surface_ps(fname.c_str(), w,h);
  else if (fmt == "svg") cr.set_surface_svg(fname.c_str(), w,h);
  else throw Err(-2) << "Can't determine output format for file: " << fname;

  if (!raster){
    cr->set_color_a(bg);
    cr->paint();
  }

  // clip to border
  if (ref.border.size()) {
    cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
    cr->mkpath_smline(ref.border, true, 0);
    cr->clip();
  }

  // Draw data
  // Save context (objects may want to have their own clip regions)
  cr->save();
  obj.draw(cr, box);
  cr->restore();

  // Draw title
  cr->reset_clip();
  if (opts.exists("title")){
    double fs = 12.0;
    cr->set_fc_font(0xFF000000, "sans:bold", fs);
    cr->text(opts.get("title").c_str(), dPoint(5.0, 5.0+fs), 0);
  }

  // write raster formats
  if (raster) {
    Opt o(opts);
    o.put("img_out_fmt", fmt);
    image_save(img, fname, o);
  }

}

