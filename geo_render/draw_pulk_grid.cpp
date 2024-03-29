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

void
draw_pulk_grid(const CairoWrapper & cr, const iPoint & origin,
               ConvBase & cnv, const Opt & opt){

  /* find wgs coordinate range and 6 degree zones */
  dRect rng = cr.bbox() + origin;
  dRect rng_wgs = cnv.frw_acc(rng); // screen -> wgs
  int lon0a = lon2lon0(rng_wgs.tlc().x);
  int lon0b = lon2lon0(rng_wgs.brc().x);

  cr->set_color_a(opt.get("grid_color", 0x8000000));
  cr->set_line_width(opt.get("grid_thick", 1.0));

  double fs = opt.get("grid_text_size",  0.0);
  if (fs>0){
    cr->set_fc_font(opt.get("grid_text_color", 0xFF000000),
                    opt.get("grid_text_font",  "serif:bold").c_str(), fs);
  }

  /* for all zones */
  for (int lon0=lon0a; lon0<=lon0b; lon0+=6){
    /* build  pulkovo -> wgs conversion or get it from the cache */
    if (!draw_pulk_grid_convs.contains(lon0))
      draw_pulk_grid_convs.add(lon0, ConvGeo(GEO_PROJ_SU(lon0)));

    ConvGeo cnv1(draw_pulk_grid_convs.get(lon0));
    ConvMulti cnv2(cnv, cnv1, 1,0); // map -> pulk

    dRect rng_pulk = cnv1.bck_acc(rng_wgs); // wgs -> pulkovo

    // Grid step. We want to have ~1 line/ 200 px
    double step = opt.get("grid_step",  0.0);
    if (step<=0){
      int nlines = rng.w/200;
      step = 2000;
      while (rng_pulk.w > step*nlines) step*=10.0;
      while (rng_pulk.w < step*nlines) step/=10.0;
    }
    else {step*=1000;}

    dPoint tlc = rng_pulk.tlc(), brc= rng_pulk.brc();
    double xmin = floor(tlc.x/step)*step;
    double xmax = ceil(brc.x/step)*step;
    double ymin = floor(tlc.y/step)*step;
    double ymax = ceil(brc.y/step)*step;

    cr->save();

    if (lon0a!=lon0b){ /* clip the zone */
      dRect rng_clip(lon0-3.0, rng_wgs.y, 6.0, rng_wgs.h);
      rng_clip.y-=rng_clip.h*0.2;
      rng_clip.h+=rng_clip.h*0.4;
      cr->mkpath(cnv.bck_acc(rect_to_line(rng_clip,true)) - origin);
      cr->clip_preserve();
      cr->stroke();
    }

    // draw lines
    for (double x=xmin; x<=xmax; x+=step){
      dLine l;
      l.push_back(dPoint(x, tlc.y));
      l.push_back(dPoint(x, brc.y));
      cr->mkpath(cnv2.bck_acc(l), false);
    }
    for (double y=ymin; y<=ymax; y+=step){
      dLine l;
      l.push_back(dPoint(tlc.x, y));
      l.push_back(dPoint(brc.x, y));
      cr->mkpath(cnv2.bck_acc(l), false);
    }
    cr->stroke();

    // draw labels
    if (fs>0){
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
          cnv2.bck(p1); // pulkovo -> wgs
          double a = cnv2.frw_ang(p1, 0, 1000);
          cr->save();
          cr->translate(p1-origin);
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
}


