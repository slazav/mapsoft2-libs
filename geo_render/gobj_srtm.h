#ifndef GOBJ_SRTM_H
#define GOBJ_SRTM_H

#include "viewer/gobj.h"
#include "srtm/srtm.h"
#include "srtm/srtm_surf.h"

/// SRTM data layer.

/********************************************************************/
#include "getopt/getopt.h"

// add SRTM and DRAWSRTM groups of options
void ms2opt_add_drawsrtm(GetOptSet & opts);

/********************************************************************/

class GObjSRTM : public GObj, SRTMSurf {

  std::shared_ptr<ConvBase> cnv;
  SRTM * srtm;

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

  public:

    GObjSRTM(SRTM *srtm, const Opt & o): SRTMSurf(o), srtm(srtm) { set_opt(o); }

  /************************************************/

    /// Get color for a point (lon-lat coords), according with drawing options.
    uint32_t get_color(const dPoint & p) {
      if (!srtm) return get_bgcolor();
      return SRTMSurf::get_color(srtm->get_val_int4(p), srtm->get_slope_int4(p));}

  /************************************************/

    static Opt get_def_opt();

    void set_opt(const Opt & o) override;

    void set_cnv(const std::shared_ptr<ConvBase> c) override;

    ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

};


#endif
