#ifndef GOBJ_MAPS_H
#define GOBJ_MAPS_H

#include "cairo/cairo_wrapper.h"
#include "conv/conv_multi.h"
#include "conv/conv_base.h"
#include "geo_data/geo_data.h"
#include "geom/poly_tools.h"
#include "cache/cache.h"
#include "image/image_cache.h"
#include "image_tiles/image_t_remote.h"
#include "opt/opt.h"
#include "viewer/gobj.h"

/********************************************************************/
#include "getopt/getopt.h"

// add DRAWMAP group of options
void ms2opt_add_drawmap(GetOptSet & opts);

/********************************************************************/

class GObjMaps : public GObj {
private:

  // Original data. It may be edited through the GObj interface.
  GeoMapList & maps;

  struct MapData{
    ConvMulti cnv;      // conversion from viewer coordinates to the map coordinates
    const GeoMap *src;  // pointer to the map
    dRect src_bbox;     // map original bbox (image coordinates, empty if no bbox)
    dMultiLine src_brd; // map original border (image coordinates)

    dRect bbox;     // map bbox (viewer coordinates, recalculated in set_cnv)
    dMultiLine brd; // map border (viewer coordinates, recalculated in set_cnv)
    double scale;   // map scale (map pixels / viewer pixels)
    double load_sc; // scale for image loading
    int zoom;       // zoom level for tiled maps
    dLine refs;     // map refpoints (in viewer coordinates)
    std::unique_ptr<ImageT> timg; // normal maps use a single img_cache for data storage;
                    // tiled maps have one ImageT object per map.
    dPolyTester test_brd; // test if point is inside map border (viewer coords)

    MapData(const GeoMap & m): load_sc(1.0), zoom(0), src(&m), test_brd(brd) {

      // src_bbox - based on actual image size for non-tiled maps,
      //            based on refpoints for tiled map, cropped to border if it is set
      // src_brd  - take non-empty border from map, or use image bbox
      if (!m.is_tiled){
        src_bbox = dRect(dPoint(), image_size(m.image));
      }
      else {
        src_bbox = m.bbox_ref_img();
        timg = std::unique_ptr<ImageT>(new ImageTRemote(m.image, m.tile_swapy, m.tile_size));
      }
      src_brd  = m.border;
      if (src_brd.size()==0) src_brd.push_back(rect_to_line(src_bbox, false));
      else src_bbox.intersect(src_brd.bbox());
    }

    // Update map scale:
    // Scale is in (image pix)/(viewer pix)
    // There are a few adjustments:
    // - for tiled map we can load different zoom depending on the scale
    // - for normal maps we can load smaller image (and save memory/time)
    void set_scale(const double k, bool sm){
      scale = k;

      // calculate zoom level for tiled images
      if (timg){
        zoom = rint(log(1.0/k)/log(2.0));
        if (zoom < 0) zoom = 0;
        if (zoom < src->tile_minz) zoom = src->tile_minz;
        if (zoom > src->tile_maxz) zoom = src->tile_maxz;
        timg->set_zoom(zoom);
      }

      // scale for normal image loading
      else {
        load_sc = floor(k/load_sc);
        if (sm) load_sc = floor(load_sc/2); // load larger images for smoothing
        if (load_sc <=1) load_sc = 1;           // never load images larger then 1:1
      }
      cnv.set_scale_dst(pow(2,zoom)/load_sc);
    }

  };
  ImageRCache img_cache;
  Cache<iRect, ImageR> tiles;
  std::vector<MapData> data;

  bool smooth;   // smooth map drawing
  bool clip_brd; // clip map to its border
  int  draw_brd; // draw map border (color)
  int  draw_refs;// draw map reference points (color)
  uint32_t fade; // map fade color
  double minsc, maxsc; // scale range (<map pix>/<image pix>) where map should be drawn
  uint32_t def_col; // color to paint the map outside minsc/maxsc

  bool render_tile(const dRect & range_dst);

public:
  // constructor
  GObjMaps(GeoMapList & maps);

  /************************************************/


  static Opt get_def_opt();

  void set_opt(const Opt & o) override;

  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  ret_t draw(const CairoWrapper & cr, const dRect &box) override;

  ret_t check(const dRect &box) const override;

  dRect bbox() const override;
};

#endif
