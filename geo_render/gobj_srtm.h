#ifndef GOBJ_SRTM_H
#define GOBJ_SRTM_H

#include "viewer/gobj.h"
#include "srtm/srtm_surf.h"

/// SRTM data layer.

/********************************************************************/
#include "getopt/getopt.h"

// add SRTM and DRAWSRTM groups of options
void ms2opt_add_drawsrtm(GetOptSet & opts);

/********************************************************************/

class GObjSRTM : public GObj {

  std::shared_ptr<ConvBase> cnv;
  SRTMSurf * srtm;

  bool surf;         // draw color surface
  double maxsc;      // max scale (srtm pixels / viewer pixels)
  double maxscv;     // max scale for vector data

  bool cnt; // draw contours?
  int cnt_step;
  int cnt_smult;
  uint32_t cnt_color;
  double cnt_w;
  double cnt_wmult;
  double cnt_crv;

  bool holes; // draw holes?
  uint32_t holes_color;
  double holes_w;

  bool peaks; // draw summits?
  uint32_t peaks_color;
  double peaks_w;
  int peaks_dh;
  int peaks_ps;
  bool peaks_text;
  double peaks_text_size;
  std::string peaks_text_font;

#define SURF_TILE_CACHE_SIZE 128
  Cache<iRect, ImageR> surf_tiles;

  public:

    GObjSRTM(SRTMSurf *srtm, const Opt & o):
      srtm(srtm), surf_tiles(SURF_TILE_CACHE_SIZE){ set_opt(o); }

    static Opt get_def_opt();

    void set_opt(const Opt & o) override;

    void set_cnv(const std::shared_ptr<ConvBase> c) override;

    bool render_surf_tile(const dRect & range_dst);
    ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

};


#endif
