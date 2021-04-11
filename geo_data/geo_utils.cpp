#include <cmath>
#include "err/err.h"
#include "opt/opt.h"
#include "geo_utils.h"
#include "geo_io.h"
#include "geom/poly_tools.h"

using namespace std;

// see https://en.wikipedia.org/wiki/Haversine_formula
double geo_dist_2d(const dPoint &p1, const dPoint &p2){
  double R = 6380e3; // metres
  double cy1 = cos(p1.y * M_PI/180.0);
  double cy2 = cos(p2.y * M_PI/180.0);
  double hdx = (1 - cos((p2.x - p1.x) * M_PI/180.0))/2;
  double hdy = (1 - cos((p2.y - p1.y) * M_PI/180.0))/2;
  return 2*R * asin(sqrt(hdy + cy1*cy2*hdx));
}

/**********************/

dMultiLine figure_geo_line(const::std::string &str) {
  try {
    dMultiLine ret;
    GeoData data;
    read_geo(str, data);

    for (const auto & t: data.trks){
      dMultiLine ml = t;
      ret.insert(ret.end(), ml.begin(), ml.end());
    }
    for (const auto & wl: data.wpts){
      for (const auto & w: wl){
        dLine l;
        l.push_back( (dPoint)w);
        ret.push_back(l);
      }
    }
    return ret;
  }
  catch (Err &e) { }
  return figure_line<double>(str);
}

/**********************/

int
lon2lon0(const double lon){
  double lon0 =floor( lon/6.0 ) * 6 + 3;
  while (lon0>180)  lon0-=360;
  while (lon0<-180) lon0+=360;
  return lon0;
}

int
lon2pref(const double lon){
  double lon0 = lon2lon0(lon);
  return (lon0<0 ? 60:0) + (lon0-3)/6 + 1;
}

int
crdx2lon0(const double X){
  int pref= floor(X/1e6);
  if (pref==0) throw Err() << "zero coordinate prefix";
  return (pref-(pref>30 ? 60:0))*6-3;
}

double
crdx2nonpref(const double X){
  return X - floor( X/1e6 ) * 1e6;
}

string
GEO_PROJ_SU(double lon){
  return "SU" + type_to_str((int)lon2lon0(lon));
}
