#ifndef GOBJ_MAPS_H
#define GOBJ_MAPS_H

#include "cairo/cairo_wrapper.h"
#include "conv/conv_multi.h"
#include "conv/conv_base.h"
#include "geo_data/geo_data.h"
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
    ConvMulti cnv;  // conversion from viewer coordinates to the map coordinates
    double scale;   // map scale (map pixels / viewer pixels)
    double load_sc; // scale for image loading
    dMultiLine brd; // map border (in viewer coordinates)
    dLine refs;     // map refpoints (in viewer coordinates)
    dRect bbox;     // map bbox (in viewer coordinates)
    dRect src_bbox; // map original bbox (it may take some time to get it)
    const GeoMap * src;   // pointer to the map
  };
  ImageRCache img_cache;
  Cache<iRect, ImageR> tiles;
  std::vector<MapData> data;

  bool smooth;   // smooth map drawing
  bool clip_brd; // clip map to its border
  int  draw_brd; // draw map border (color)
  int  draw_refs;// draw map reference points (color)
  uint32_t fade; // map fade color

public:
  // constructor
  GObjMaps(GeoMapList & maps);

  /************************************************/
  // drawing waypoints on the image
  int draw(const CairoWrapper & cr, const dRect &box) override;


  /************************************************/
  // These functions update drawing templates.
  // They have proper multi-thread locking.

  // update information from options
  void on_set_opt() override;

  // update point coordinates
  void on_set_cnv() override;

  // rescale point coordinates, update range
  void on_rescale(double k) override;

private:
  bool render_tile(const MapData & d, const dRect & range_dst);

};

#endif
