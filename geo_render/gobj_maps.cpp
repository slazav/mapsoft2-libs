#include <vector>
#include <cmath>

#include "geohash/storage.h"
#include "geom/line.h"
#include "geo_data/conv_geo.h"

#include "gobj_maps.h"

using namespace std;

#define IMAGE_CACHE_SIZE 10

#define TILE_CACHE_SIZE 128

void
ms2opt_add_drawmap(GetOptSet & opts){
  const char *g = "DRAWMAP";
  opts.add("map_smooth", 1,0,g,
    "Smooth map drawing, interpolation for small scales, "
    "averaging for large ones, (0|1, default - 0).");
  opts.add("map_clip_brd", 1,0,g,
    "Clip map to its border (default 1).");
  opts.add("map_draw_refs", 1,0,g,
    "Draw map reference points (ARGB color, default 0).");
  opts.add("map_draw_brd", 1,0,g,
    "Draw map border (ARGB color, default 0).");
  opts.add("map_fade", 1,0,g,
    "Color to fade the map (default is 0, no fading).");
}

Opt
GObjMaps::get_def_opt() {
  Opt o;
  o.put("map_smooth",   false);
  o.put("map_clip_brd", true);
  o.put("map_draw_refs", 0);
  o.put("map_draw_brd",  0);
  o.put("map_fade",      0);
  return o;
}

void
GObjMaps::set_opt(const Opt & opt) {
  smooth    = opt.get("map_smooth",   false);
  clip_brd  = opt.get("map_clip_brd", true);
  draw_refs = opt.get("map_draw_refs", 0);
  draw_brd  = opt.get("map_draw_brd",  0);
  fade      = opt.get("map_fade",     0);
  redraw_me();
}

void
GObjMaps::set_cnv(const std::shared_ptr<ConvBase> cnv) {
  range = dRect();
  for (auto & d:data){

    // conversion viewer->map
    d.cnv.reset();
    if (cnv) d.cnv.push_back(*cnv, true); // viewer -> WGS
    d.cnv.push_back(ConvMap(*d.src), false); // WGS -> map

    // border in viewer coordinates
    d.brd = d.cnv.bck_acc(close(d.src->border));

    // update border tester
    d.test_brd = dPolyTester(d.brd);

    // reference points in viewer coordinates
    d.refs = dLine();
    for (auto const & r:d.src->ref)
      d.refs.push_back(r.first);
    d.cnv.bck(d.refs);

    // map bbox in viewer coordinates
    // Not good for large map in wrong projection,
    // Distorsions can be really large!
    d.bbox = d.cnv.bck_acc(d.src_bbox);
    range.expand(d.bbox);

    // calculate map scale (map pixels per viewer pixel)
    // at the map's origin
    dRect r(0,0,1,1);
    r = d.cnv.bck_acc(r); // map->viewer
    dPoint sc = d.cnv.scales(r);
    double k = std::max(sc.x, sc.y);

    // update map scale
    d.set_scale(k, smooth);

    // Simplify the conversion if possible.
    // Use 0.5pt accuracy in source coordinates (viewer).
    d.cnv.simplify(d.bbox, 5, 0.5);

    // for tiled maps clear downloader's queue
    if (d.timg) d.timg->clear_queue();

  }
  tiles.clear();
  redraw_me();
}

/**********************************************************/

GObjMaps::GObjMaps(GeoMapList & maps):
    maps(maps), img_cache(IMAGE_CACHE_SIZE), tiles(TILE_CACHE_SIZE),
    smooth(false), clip_brd(true), draw_brd(0), draw_refs(0), fade(0) {

  for (auto & m:maps){
    m.update_size();
    data.emplace_back(m);
  }
}

bool
GObjMaps::render_tile(const dRect & draw_range) {

  ImageR image_dst = ImageR(draw_range.w, draw_range.h, IMAGE_32ARGB);
  image_dst.fill32(0);

  for (auto const & d:data){

    if (intersect(draw_range, d.bbox).is_zsize()) continue;

    // prepare Image source.
    // For normal maps it's a ImageR, from ImageCache
    // For tiled maps it is ImageT object from MapData.

    ImageR imageR_src; // keep ImageR data (to be moved to ImageCache?)
    Image * image_src = &imageR_src;
    imageR_src.set_bgcolor(d.src->def_color); // default color

    bool draw_map = d.scale*pow(2,d.zoom) >= d.src->min_scale &&
                    d.scale*pow(2,d.zoom) <= d.src->max_scale;

    // tiled map
    if (d.src->is_tiled && draw_map)
      image_src = d.timg.get();

    // non-tiled maps
    if (!d.src->is_tiled && draw_map)
      imageR_src = img_cache.get(d.src->image, d.load_sc);

    double avr = d.scale/d.load_sc;

    // render image
    for (size_t yd=0; yd<image_dst.height(); ++yd){
      if (is_stopped()) return false;
      auto cr = d.test_brd.get_cr(yd + draw_range.y);
      if (d.brd.size() && cr.size()==0) continue;
      for (size_t xd=0; xd<image_dst.width(); ++xd){

        dPoint p(xd + draw_range.x, yd + draw_range.y);
        if (d.brd.size() && !dPolyTester::test_cr(cr, p.x)) continue;

        d.cnv.frw(p); // convert to source image coordinates
        image_src->check_crd(p.x, p.y);

        int color;
        if (smooth){
          if (avr<1) color = image_src->get_color_int4(p);
          else       color = image_src->get_color_avrg(p, avr);
        }
        else {
          color = image_src->get_color(p);
        }
        image_dst.set32(xd, yd, color);
      }
    }
  }
  tiles.add(draw_range, image_dst);
  return true;
}

void
GObjMaps::prepare_range(const dRect & range) {
  // For tiled maps start parallel downloading of all maps in the range.
  for (auto const & d:data){
    if (!d.src->is_tiled) continue;
    dRect r = d.cnv.frw_acc(range);
    d.timg->prepare_range(r);
  }
}


GObj::ret_t
GObjMaps::draw(const CairoWrapper & cr, const dRect & draw_range) {

  if (is_stopped()) return GObj::FILL_NONE;
  if (intersect(draw_range, range).is_zsize()) return GObj::FILL_NONE;

  // render tile, put to tile cache if needed
  if (!tiles.contains(draw_range)) render_tile(draw_range);

  // render image
  cr->set_source(image_to_surface(tiles.get(draw_range)),
    draw_range.x, draw_range.y);
  cr->paint();

  // draw map borders
  if (draw_brd){
    for (auto const & d:data){
      cr->set_color_a(draw_brd);
      cr->mkpath(d.brd);
      cr->stroke();
    }
  }

  // draw map ref points
  if (draw_refs){
    for (auto const & d:data){
      cr->set_color_a(draw_refs);
      for (auto const & p:d.refs)
        cr->circle(p, 2);
      cr->stroke();
    }
  }

  // fade the layer
  if (fade){
    cr->set_color_a(fade);
    cr->paint();
  }

  return GObj::FILL_PART;
}

