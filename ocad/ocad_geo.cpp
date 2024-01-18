#include "ocad_geo.h"

//#include "geom/line_utils.h"
#include "geo_data/conv_geo.h"
#include "geo_data/geo_utils.h"
#include <sstream>

using namespace std;

#define OCAD_SCALE_PAR 1039

GeoMap
ocad_get_ref(const ocad & O){

  dLine pts = rect_to_line(O.range());

  vector<ocad_string>::const_iterator s;
  for (s=O.strings.begin(); s!=O.strings.end(); s++){
    if ((s->type == OCAD_SCALE_PAR) && (s->get_str('r') == "1")){
      double rscale  = s->get<double>('m');
      double grid    = s->get<double>('g');
      double grid_r  = s->get<double>('d');
      dPoint  p0(s->get<double>('x'),
                 s->get<double>('y'));
      double a = s->get<double>('a');
      int zone    = s->get<int>('i'); // grid and zone - "2037"

      auto proj = GEO_PROJ_SU((zone%1000-30)*6-3);
      ConvGeo cnv(proj, "WGS");

      dLine pts0(pts);
      pts.rotate2d(dPoint(), -a * M_PI/180);
      pts*=rscale / 100000; // 1point = 0.01mm
      pts+=p0;
      cnv.frw(pts);

      GeoMap ret;
      for (int i = 0; i < pts.size(); i++)
        ret.ref.emplace(pts[i], pts0[i]);
      ret.proj=proj;
      ret.border.push_back(pts0);
      return ret;
    }
  }
  return GeoMap();
}

void
ocad_set_ref(ocad & O, double rscale, const dPoint & p0){

  // remove old value;
  vector<ocad_string>::iterator si = O.strings.begin();
  while (si!=O.strings.end()){
    if (si->type == OCAD_SCALE_PAR) si = O.strings.erase(si);
    else si++;
  }

  ConvGeo cnv(GEO_PROJ_SU(p0.x), "WGS");
  dPoint pc(p0);
  cnv.bck(pc);

  // add new string
  int grid=1000;
  ostringstream str;
  str << "\tm" << int(rscale)
      << "\tg" << grid   // grid, 0.01mm units
      << "\tr" << "1"    // geo reference is on
      << "\tx" << int(pc.x)
      << "\ty" << int(pc.y)
      << "\ta" << "0"      // angle, deg
      << "\td" << (grid*rscale)/100000 // grid, 1m units
      << "\ti" << lon2pref(p0.x) + 2030;
                  // I don't know that is 2030...

  ocad_string s;
  s.type = OCAD_SCALE_PAR;
  s.data = str.str();
  O.strings.push_back(s);
}
