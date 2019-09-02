#ifndef DRAW_PULK_GRID_H
#define DRAW_PULK_GRID_H

#include "opt/opt.h"
#include "conv/conv_base.h"
#include "geom/point.h"
#include "cairo/cairo_wrapper.h"

/*
Options:
  --grid_draw_color 0xFF000000
  --grid_draw_thick 2

*/

void
draw_pulk_grid(CairoWrapper & cr, const iPoint & origin,
               const ConvBase & cnv, const Opt & opt);

#endif