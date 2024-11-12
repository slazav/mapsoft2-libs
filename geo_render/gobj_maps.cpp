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
  opts.add("map_min_sc", 1,0,g,
    "Min scale (<map pixels>/<image pixels>) (default is 0.1).");
  opts.add("map_max_sc", 1,0,g,
    "Max scale (<map pixels>/<image pixels>) (default is 10).");
  opts.add("map_def_col", 1,0,g,
    "Color to paint the map outside min_sc/max_sc  (default is 0x80FF0000).");
}

Opt
GObjMaps::get_def_opt() {
  Opt o;
  o.put("map_smooth",   false);
  o.put("map_clip_brd", true);
  o.put("map_draw_refs", 0);
  o.put("map_draw_brd",  0);
  o.put("map_fade",      0);
  o.put("map_min_sc",  0.1);
  o.put("map_max_sc",  10.0);
  o.put("map_def_col", "0x80FF0000");
  return o;
}

void
GObjMaps::set_opt(const Opt & opt) {
  smooth    = opt.get("map_smooth",   false);
  clip_brd  = opt.get("map_clip_brd", true);
  draw_refs = opt.get("map_draw_refs", 0);
  draw_brd  = opt.get("map_draw_brd",  0);
  fade      = opt.get("map_fade",      0);
  minsc     = opt.get("map_min_sc",   0.1);
  maxsc     = opt.get("map_max_sc",   10);
  def_col   = opt.get("map_def_col",  0x80FF0000);
  for (auto & d:data){ if (d.timg) d.timg->set_opt(opt); }
  redraw_me();
}

void
GObjMaps::set_cnv(const std::shared_ptr<ConvBase> cnv) {
  if (!cnv) throw Err() << "GObjMaps::set_cnv: cnv is NULL";
  range = dRect();
  for (auto & d:data){

    // We want to calculate some map parameters in viewer projection:
    // bbox, map scale (<map pixels>/<viewer pixels>), map border etc.
    // A few cases should be considered:
    // 1. Viewer projection is set for a tiled map covering the
    //    whole Earth, Map projection is Transverse Mercator
    //    which works only near its central meridian.
    // 2. Opposit case: viewer is in Transverse Mercator, map is
    //    in Mercator covering the while world (except poles).
    // 3. Map and viewer are in Transverse Mercator but with
    //    very different central meridians...

    // conversion viewer->map
    d.cnv.reset();
    if (cnv) d.cnv.push_back(*cnv, true); // viewer -> WGS
    d.cnv.push_back(ConvMap(*d.src), false); // WGS -> map

    // Try to calculate border in viewer coordinates
    // and update border tester.
    // If it is not possible ignore the border.
    try {
      d.brd = d.cnv.bck_acc(close(d.src->border));
    }
    catch (const Err & e) {
      d.brd = dMultiLine();
    }
    d.test_brd = dPolyTester(d.brd);

    // Same with refpoints.
    try {
      d.refs = dLine();
      for (auto const & r:d.src->ref)
        d.refs.push_back(r.first);
      d.cnv.bck(d.refs);
    }
    catch (const Err & e) {
      d.refs = dLine();
    }

    // Same with bbox.
    try {
      d.bbox = d.cnv.bck_acc(d.src_bbox);
      range.expand(d.bbox);
    }
    catch (const Err & e) {
      d.bbox = dRect(-HUGE_VAL, -HUGE_VAL, HUGE_VAL, HUGE_VAL);
      range.expand(d.bbox);
    }

    // Calculate map scale (map pixels per viewer pixel).
    // To have reasonable accuracy in different cases we
    // use map center if it is inside viewer coords (small map in
    // "large" viewer) or use viewer origin (in opposite case)
    // Find this point in viewer coords:
    dPoint p0 = d.refs.bbox().cnt(); // map center in viewer coords
    if (p0.x < 0 && p0.y < 0) p0 = dPoint(0,0); // use viewer origin

    dRect r(p0, p0+dPoint(1,1));
    dPoint sc = d.cnv.scales(r);
    double k = std::max(sc.x, sc.y);

    // update map scale
    d.set_scale(k, smooth);

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
    imageR_src.set_bgcolor(def_col); // default color

    bool draw_map = d.scale*pow(2,d.zoom) >= minsc &&
                    d.scale*pow(2,d.zoom) <= maxsc;

    // tiled map
    if (d.src->is_tiled && draw_map)
      image_src = d.timg.get();

    // non-tiled maps
    if (!d.src->is_tiled && draw_map)
      imageR_src = img_cache.get(d.src->image, d.load_sc);

    if (imageR_src.is_empty()) continue;
    double avr = d.scale/d.load_sc;

    // Simplify map-to-map conversion if possible.
    // Use 5x5 calculation grid and 0.5pt accuracy
    // in source (viewer) coordinates.
    ConvMulti cnv(d.cnv);
    cnv.simplify(draw_range, 5, 0.5);

    // render image
    for (size_t yd=0; yd<image_dst.height(); ++yd){
      if (is_stopped()) return false;
      auto cr = d.test_brd.get_cr(yd + draw_range.y);
      if (d.brd.size() && cr.size()==0) continue;
      for (size_t xd=0; xd<image_dst.width(); ++xd){

        dPoint p(xd + draw_range.x, yd + draw_range.y);
        if (d.brd.size() && !dPolyTester::test_cr(cr, p.x)) continue;

        cnv.frw(p); // convert to source image coordinates
        if (!image_src->check_crd(p.x, p.y)) continue;

        int color;
        if (smooth){
          if (avr<1) color = image_src->get_argb_int4(p);
          else       color = image_src->get_argb_avrg(p, avr);
        }
        else {
          color = image_src->get_argb(p);
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
  if (!tiles.contains(draw_range)){
    if (!render_tile(draw_range)) return GObj::FILL_NONE;
  }

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

