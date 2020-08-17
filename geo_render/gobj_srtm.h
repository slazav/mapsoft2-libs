#ifndef GOBJ_SRTM_H
#define GOBJ_SRTM_H

#include "viewer/gobj.h"
#include "srtm/srtm.h"


/// SRTM data layer.

class GObjSRTM : public SRTM, public GObj {

  std::shared_ptr<ConvBase> cnv;

  /// how to draw the surface
  enum draw_mode_t {
    SRTM_DRAW_NONE,
    SRTM_DRAW_SHADES, // heights shaded with slope value
    SRTM_DRAW_HEIGHTS,
    SRTM_DRAW_SLOPES,
  } draw_mode;

  bool interp_holes; // interpolate holes in data
  uint32_t bgcolor; // how to draw holes

  public:

    Rainbow R; // color converter

    GObjSRTM(const Opt & o) { set_opt(o); }

  /************************************************/

    int draw(const CairoWrapper & cr, const dRect & draw_range) override;

    void set_cnv(const std::shared_ptr<ConvBase> c) override {cnv = c;}

    void set_opt(const Opt & o) override;
};


#endif
