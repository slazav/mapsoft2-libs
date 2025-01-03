#ifndef GOBJ_MAPS_H
#define GOBJ_MAPS_H

#include "cairo/cairo_wrapper.h"
#include "conv/conv_multi.h"
#include "conv/conv_base.h"
#include "geo_data/geo_data.h"
#include "image_tiles/image_t.h"
#include "geom/poly_tools.h"
#include "cache/cache.h"
#include "image/image_cache.h"
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
    std::shared_ptr<ImageT> timg; // normal maps use a single img_cache for data storage;
                    // tiled maps have one ImageT object per map.
    dPolyTester test_brd; // test if point is inside map border (viewer coords)

    MapData(const GeoMap & m);

    // Update map scale:
    // Scale is in (image pix)/(viewer pix)
    // There are a few adjustments:
    // - for tiled map we can load different zoom depending on the scale
    // - for normal maps we can load smaller image (and save memory/time)
    void set_scale(const double k, bool sm);
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
