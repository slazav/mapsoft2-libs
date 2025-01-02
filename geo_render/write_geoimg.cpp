#include "cairo/cairo_wrapper.h"
#include "filename/filename.h"
#include "geo_data/geo_io.h"
#include "geo_data/conv_geo.h"
#include "write_geoimg.h"
#include "image_tiles/image_t_local.h"
#include "image/image_colors.h"
#include "geo_mkref/geo_mkref.h" // for tiled maps
#include "geom/poly_tools.h"     // for rect_in_polygon
#include "geo_tiles/geo_tiles.h"
#include <fstream>


void
ms2opt_add_geoimg(GetOptSet & opts){
  ms2opt_add_std(opts, {"OUT"});
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
  opts.add("map", 1,'m',g, "Write OziExplorer map file for the image.");
  opts.add("skip_image", 0,0,g,
    "Do not write image file (can be used if only the map file is needed). "
    "Option -o <file> should be provided anyway, the filename "
    "will be written to the map-file.");
  opts.add("title", 1,0,g, "write text in the top-left corner.");
  opts.add("title_size", 1,0,g, "Font size for the title.");
  opts.add("tmap", 0,0,g,
    "Write tiled map. In this case `fname` parameter should contain a template "
    "with {x}, {y}, {z} fields. Output of map file is not supported at "
    "the moment. `mkref` options are ignored. Sub-directories are created if needed.");
  opts.add("zmin", 1,0,g, "Min zoom for tiled maps.");
  opts.add("zmax", 1,0,g, "Max zoom for tiled maps.");
  opts.add("tmap_scale", 1,0,g,
    "When creating tiles with multiple zoom levels scale larger tiles to "
    "create smaller ones (instead of rendering all tiles separately). "
    "Tile is created only if at least one source tile is newer then "
    "the destination tile (or destination does not exist). Default: 0.");
  opts.add("skip_empty", 0,0,g,
    "Do not save image if nothing was drawn. Default: 0");
}

#define TMAP_TILE_SIZE 256

void
write_tiles(const std::string & fname, GObj & obj, const dMultiLine & brd, const Opt & opts){

  int zmin = opts.get("zmin", 0);
  int zmax = opts.get("zmax", 0);
  uint32_t bg = opts.get<int>("bgcolor", 0xFFFFFFFF);
  bool verb = opts.get<int>("verbose", false);
  obj.set_opt(opts);

  GeoTiles tcalc;  // tile calculator
  ImageTLocal timg(fname); // interface to tiled image
  timg.set_opt(opts);

  // bbox: intersection of border bbox and object bbox
  dRect bbox = intersect_nonempty(brd.bbox(), obj.bbox());

  if (bbox.is_empty()) throw Err() <<
    "Error calculating tile range. Try to set non-empty boundary";

  // set of existing directories
  std::set<std::string> dirs;

  // for each zoom level
  for (int z = zmax; z>=zmin; --z){
    iRect tiles = tcalc.range_to_gtiles(bbox,z); // tile range
    tiles.intersect(iRect(0,0, pow(2,z), pow(2,z)));

    // render tiles
    for (int y = tiles.y; y < tiles.y + tiles.h; y++){
      for (int x = tiles.x; x < tiles.x + tiles.w; x++) {
        iPoint tile(x,y,z);
        dRect trange_wgs = tcalc.gtile_to_range(tile, z); // Google, not TMS tiles
        dRect trange_img = dRect(0,0,TMAP_TILE_SIZE,TMAP_TILE_SIZE);
        // skip tiles outside global border
        if (brd.size() && rect_in_polygon(trange_wgs, brd) == 0) continue;

        // make filename, create subdirectories if needed
        std::string f = ImageT::make_url(fname, tile);

        // render tile in a normal way
        if (z == zmax || !opts.get("tmap_scale", false)){
          // make reference for the tile (similar to code in geo_mkref) 
          GeoMap r;
          r.proj = "WEB";
          r.image_size = iPoint(1,1)*TMAP_TILE_SIZE;
          dLine pts_w = rect_to_line(trange_wgs, false);
          dLine pts_r = rect_to_line(trange_img, false);
          pts_r.flip_y(r.image_size.y);
          r.add_ref(pts_r, pts_w);

          std::shared_ptr<ConvMap> cnv(new ConvMap(r));
          obj.set_cnv(cnv);

          // skip tiles outside object range
          if (obj.check(trange_img) == GObj::FILL_NONE) continue;

          if (verb) std::cout << "create tile: " << tile << "\n";
          // create background image
          ImageR img;
          if (opts.exists("add") && timg.tile_exists(tile)){
            try {
              img = image_to_argb(timg.tile_read(tile));
              if (img.height()!=TMAP_TILE_SIZE || img.width()!=TMAP_TILE_SIZE)
                img = ImageR();
            } catch (const Err & e) { }
          }
          if (img.is_empty()){
            img = ImageR(TMAP_TILE_SIZE, TMAP_TILE_SIZE, IMAGE_32ARGB);
            img.fill32(bg);
          }

          // setup cairo context
          CairoWrapper cr;
          cr.set_surface_img(img);

          // clip to border
          // convert border to pixel coordinates of this tile
          r.border = cnv->bck_acc(brd);
          if (r.border.size()) {
            cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
            cr->mkpath_smline(r.border, true, 0);
            cr->clip();
          }

          // Draw data
          // Save context (objects may want to have their own clip regions)
          cr->save();
          int res = obj.draw(cr, trange_img);
          cr->restore();
          timg.tile_write(tile, img);
        }

        // collect the tile from four larger tiles
        else {
          if (!timg.tile_rescale_check(tile)) continue;
          if (verb) std::cout << "rescale tile: " << tile << "\n";
          timg.tile_rescale(tile);
        }
      }
    }
  }
}

void
write_geoimg(const std::string & fname, GObj & obj, const GeoMap & ref, const Opt & opts){

  // process tiled maps
  if (opts.exists("tmap")){
    // calculate WGS border
    dMultiLine brd = ref.border;
    if (!ref.empty()) {
      ConvMap cnv(ref); // conversion original map->wgs
      brd = cnv.frw_acc(brd); // -> wgs
    }
    write_tiles(fname, obj, brd, opts);
    return;
  }

  if (ref.empty()) throw Err() <<
    "Can't build map reference: use --mkref option";

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
    auto mapfile = opts.get("map","");
    r.image = file_rel_path(fname, mapfile);
    write_ozi_map(mapfile, r, opts);
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

  // check if we can skip drawing
  auto chk = obj.check(box);
  if (chk == GObj::FILL_NONE){
    if (opts.get<bool>("skip_empty")) return;
    if (opts.exists("add") && file_exists(fname)) return;
  }

  // setup cairo context
  CairoWrapper cr;
  ImageR img;
  size_t w=box.brc().x, h=box.brc().y;

  // create background image
  uint32_t bg = opts.get<int>("bgcolor", 0xFFFFFFFF);
  bool raster = (fmt == "png" || fmt=="jpeg" || fmt=="tiff" || fmt=="gif");
  if (raster) {
    if (opts.exists("add") && file_exists(fname)){
      Opt o;
      o.put("img_in_fmt", fmt);
      try {
        img = image_to_argb(image_load(fname, 1, o));
        if (img.height()!=h || img.width()!=w) throw Err();
      } catch (const Err & e) {
        img = ImageR(w,h,IMAGE_32ARGB);
        img.fill32(bg);
      }
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
  else{
    if (fmt!="")
      throw Err(-2) << "Unknown output format for image file: " << fname << ": " << fmt;
    else
      throw Err(-2) << "Can't determine output format for image file: " << fname;
  }

  if (!raster){
    cr->set_color_a(bg);
    cr->paint();
  }

  if (chk != GObj::FILL_NONE){
    // clip to border
    if (ref.border.size()) {
      cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
      cr->mkpath_smline(ref.border, true, 0);
      cr->clip();
    }

    // Draw data
    // Save context (objects may want to have their own clip regions)
    cr->save();
    int res = obj.draw(cr, box);
    cr->restore();

    if (opts.get<bool>("skip_empty") && res == GObj::FILL_NONE) return;
  }

  // Draw title
  cr->reset_clip();
  if (opts.exists("title")){
    double fs = opts.get("title_size", 12.0);
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


void
write_html_map(const std::string & htmfile, const std::string & imgfile,
   const GeoMap & ref, const std::list<GeoMapList> & maps){

  // find image dimensions (same as in write_img)
  dRect box = dRect(dPoint(), (dPoint)ref.image_size);
  if (box.is_zsize()) box = ref.border.bbox();
  if (box.is_zsize()) throw Err() << "write_html_map: can't get map dimensions";

  if (htmfile == "") return;
  auto ff = file_rel_path(imgfile, htmfile);
  std::ofstream f(htmfile);
  f << "<html><body>\n"
    << "<img border=\"0\" "
    <<      "src=\""    << ff << "\" "
    <<      "width=\""  << box.brc().x << "\" "
    <<      "height=\"" << box.brc().y << "\" "
    <<      "usemap=\"#map:" << ff << "\">\n"
    << "<map name=\"map:" << ff << "\">\n";
    for (const auto & ml: maps) {
      for (const auto & m: ml) {
        ConvMap c1(m), c2(ref);

        // for each border segment
        for (const auto & b: m.border) {
          dLine bc = c2.bck_acc(c1.frw_acc(b));
          f << "<area shape=\"poly\" "
            <<       "href=\""   << m.image << "\" "
            <<       "alt=\""    << m.comm << "\" "
            <<       "title=\""  << m.comm << "\" "
            <<       "coords=\"";
          for (size_t i=0; i<bc.size(); i++)
            f << (i>0?",":"") << bc[i].x << "," << bc[i].y;
          f << "\">\n";
        }
      }
    }
    f << "</map>\n"
      << "</body></html>";
    f.close();
  }

