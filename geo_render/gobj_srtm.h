#ifndef GOBJ_SRTM_H
#define GOBJ_SRTM_H

#include "viewer/gobj.h"
#include "srtm/srtm.h"


/// SRTM data layer.

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
  int cnt_step1;
  int cnt_step2;
  uint32_t cnt_color;
  double cnt_th;
  double cnt_th2;
  double cnt_crv;

  bool holes; // draw holes?
  uint32_t holes_color;
  double holes_th;

  bool peaks; // draw summits?
  uint32_t peaks_color;
  double peaks_th;
  int peaks_dh;
  int peaks_ps;
  bool peaks_text;
  double peaks_text_size;
  std::string peaks_text_font;



  public:

    Rainbow R; // color converter

    GObjSRTM(const Opt & o) { set_opt(o); }

  /************************************************/

    int draw(const CairoWrapper & cr, const dRect & draw_range) override;

    void set_cnv(const std::shared_ptr<ConvBase> c) override {cnv = c;}

    void set_opt(const Opt & o) override;

    Opt get_opt() const;
};


#endif
