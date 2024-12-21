#include <sstream>

#include "geo_data/conv_geo.h"
#include "geo_data/geo_utils.h"
#include "geom/line.h"
#include "cache/cache.h"

#include "draw_pulk_grid.h"

using namespace std;

Cache<int, ConvGeo> draw_pulk_grid_convs(10); // lon0 -> Conv

void
ms2opt_add_drawgrd(GetOptSet & opts){
  const char *g = "DRAWGRD";
  opts.add("grid_step",       1,0,g, "grid step (km, default 0, auto)");
  opts.add("grid_color",      1,0,g, "grid color (default 0x80000000)");
  opts.add("grid_thick",      1,0,g, "grid line thickness (default 1)");
  opts.add("grid_text_color", 1,0,g, "grid text color (default 0xff000000)");
  opts.add("grid_text_font",  1,0,g, "grid text font (default \"serif:bold\")");
  opts.add("grid_text_size",  1,0,g, "grid text size, pixels (default 0, no text)");
}

/********************************************************************/

// Helper for drawing km grids.
// Draw grid inside the range and boundaries of the clip region.
void
draw_grid_zone(const CairoWrapper & cr,
          ConvBase & cnv,        // image -> grid
          const iPoint & origin, // cairo surface origin
          const dRect & range_grid, // drawing region in grid coordinates
          const dLine & clip_grid,  // clipping region in grid coordinates (zone boundary)
          double step, bool draw_labels){

  // calculate auto step
  if (step<=0){ // auto mode: we want to have ~1 line/ 200 px
    int nlines = range_grid.w/200;
    step = 2000;
    while (range_grid.w > step*nlines) step*=10.0;
    while (range_grid.w < step*nlines) step/=10.0;
  }
  else {step*=1000;}

  // calculate grid extents
  dPoint tlc = range_grid.tlc(), brc= range_grid.brc();
  double xmin = floor(tlc.x/step)*step;
  double xmax = ceil(brc.x/step)*step;
  double ymin = floor(tlc.y/step)*step;
  double ymax = ceil(brc.y/step)*step;

  // set clipping range and draw its boundary
  cr->save();
  if (clip_grid.size()){
    cr->mkpath(cnv.bck_acc(clip_grid)/* - origin*/);
    cr->clip_preserve();
    cr->stroke();
  }

  // draw grid lines
  for (double x=xmin; x<=xmax; x+=step){
    dLine l;
    l.push_back(dPoint(x, tlc.y));
    l.push_back(dPoint(x, brc.y));
    cr->mkpath(cnv.bck_acc(l) - origin, false);
  }
  for (double y=ymin; y<=ymax; y+=step){
    dLine l;
    l.push_back(dPoint(tlc.x, y));
    l.push_back(dPoint(brc.x, y));
    cr->mkpath(cnv.bck_acc(l) - origin, false);
  }
  cr->stroke();

  // draw labels
  if (draw_labels){
    int text_space = 5;
    step*=2;
    xmin = floor(tlc.x/step)*step;
    xmax = ceil(brc.x/step)*step;
    ymin = floor(tlc.y/step)*step;
    ymax = ceil(brc.y/step)*step;

    for (double x=xmin; x<=xmax; x+=step){
      for (double y=ymin; y<=ymax; y+=step){
        stringstream sx, sy;
        sx << int(x/1000), sy << int(y/1000);
        dPoint p1(x,y);
        cnv.bck(p1); // grid -> image
        double a = cnv.bck_ang(p1, 0, 1000);
        cr->save();
        cr->translate(p1 - origin);
        cr->rotate(a);
        cr->move_to(dPoint(text_space,-text_space));
        cr->show_text(sy.str());

        dRect e = cr->get_text_extents(sx.str());
        cr->move_to(dPoint(text_space+e.h, -2*text_space-e.h));
        cr->rotate(-M_PI/2.0);
        cr->show_text(sx.str());
        cr->restore();
      }
    }
  }
  cr->restore();
}

/************************************************************/
void
draw_pulk_grid(const CairoWrapper & cr, const iPoint & origin,
               ConvBase & cnv, const Opt & opt){

  /* find wgs coordinate range and 6 degree zones */
  dRect rng = cr.bbox() + origin;   // image range
  dRect wgs_rng = cnv.frw_acc(rng); // image range -> wgs range
  int lon0a = lon2lon0(wgs_rng.tlc().x);
  int lon0b = lon2lon0(wgs_rng.brc().x);

  cr->set_color_a(opt.get("grid_color", 0x8000000));
  cr->set_line_width(opt.get("grid_thick", 1.0));

  double fs = opt.get("grid_text_size",  0.0);
  if (fs>0)
    cr->set_fc_font(opt.get("grid_text_color", 0xFF000000),
                    opt.get("grid_text_font",  "serif:bold").c_str(), fs);

  double step = opt.get("grid_step",  0.0);

  /* build  pulkovo ll -> wgs conversion or get it from the cache */
  if (!draw_pulk_grid_convs.contains(INT_MAX))
    draw_pulk_grid_convs.add(INT_MAX, ConvGeo("SU_LL"));
  ConvGeo cnv0(draw_pulk_grid_convs.get(INT_MAX));

  /* for all zones */
  for (int lon0=lon0a; lon0<=lon0b; lon0+=6){

    /* build  pulkovo grid -> wgs conversion or get it from the cache */
    if (!draw_pulk_grid_convs.contains(lon0))
      draw_pulk_grid_convs.add(lon0, ConvGeo(GEO_PROJ_SU(lon0)));
    ConvGeo cnv1(draw_pulk_grid_convs.get(lon0)); // grid -> wgs
    ConvMulti cnv2(cnv, cnv1, 1,0); // screen -> grid

    dRect rng_grid = cnv1.bck_acc(wgs_rng); // wgs -> pulkovo grid

    dLine clip_grid; // clipping range in image coordinates
    if (lon0a!=lon0b){ /* clip zone */
      dRect r(lon0-3.0, wgs_rng.y, 6.0, wgs_rng.h);
      r.expand(0, 0.2); // do not draw horizontal boundaries
      clip_grid = rect_to_line(r, true);
      cnv0.frw(clip_grid); // pulkovo ll -> wgs, point to point conversion
      clip_grid = cnv1.bck_acc(clip_grid); // -> grid units, accurate conversion
    }
    draw_grid_zone(cr, cnv2, origin, rng_grid, clip_grid, step, fs>0);
  }
}


