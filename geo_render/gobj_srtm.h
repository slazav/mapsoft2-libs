#ifndef GOBJ_SRTM_H
#define GOBJ_SRTM_H

#include "viewer/gobj.h"
#include "srtm/srtm.h"


/// SRTM data layer.

/********************************************************************/
#include "getopt/getopt.h"

// add SRTM and DRAWSRTM groups of options
void ms2opt_add_drawsrtm(GetOptSet & opts);

/********************************************************************/

class GObjSRTM : public SRTM, public GObj {

  std::shared_ptr<ConvBase> cnv;

  /// how to draw data
  enum draw_mode_t {
    SRTM_DRAW_NONE,
    SRTM_DRAW_SHADES, // heights shaded with slope value
    SRTM_DRAW_HEIGHTS,
    SRTM_DRAW_SLOPES,
  } draw_mode;

  double hmin,hmax;  // limits for heights and shades modes
  double smin,smax;  // limits for slopes mode

  bool interp_holes; // interpolate holes in data
  uint32_t bgcolor;  // how to draw holes
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

    Rainbow R; // color converter

    GObjSRTM(const Opt & o) { set_opt(o); }

  /************************************************/

    static Opt get_def_opt();

    void set_opt(const Opt & o) override;

    void set_cnv(const std::shared_ptr<ConvBase> c) override;

    int draw(const CairoWrapper & cr, const dRect & draw_range) override;



};


#endif
