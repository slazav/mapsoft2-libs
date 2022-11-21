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

double
geo_nearest_dist(const dLine & l, const dPoint & pt){
  double d =  INFINITY;
  for (const auto & p:l){
    double d1 = geo_dist_2d(p, pt);
    if (d1>=d) continue;
    d = d1;
  }
  if (std::isinf(d)) throw Err() << "Can't find nearest point: empty line";
  return d;
}

double
geo_nearest_dist(const dMultiLine & ml, const dPoint & pt){
  double d =  INFINITY;
  for (const auto & l:ml){
    for (const auto & p:l){
      double d1 = geo_dist_2d(p, pt);
      if (d1>=d) continue;
      d = d1;
    }
  }
  if (std::isinf(d)) throw Err() << "Can't find nearest point: empty line";
  return d;
}

dPoint
geo_nearest_pt(const dLine & l, const dPoint & pt){
  dPoint ret;
  double d =  INFINITY;
  for (const auto & p:l){
    double d1 = geo_dist_2d(p, pt);
    if (d1>=d) continue;
    d = d1; ret=p;
  }
  if (std::isinf(d)) throw Err() << "Can't find nearest point: empty line";
  return ret;
}

dPoint
geo_nearest_pt(const dMultiLine & ml, const dPoint & pt){
  dPoint ret;
  double d =  INFINITY;
  for (const auto & l:ml){
    for (const auto & p:l){
      double d1 = geo_dist_2d(p, pt);
      if (d1>=d) continue;
      d = d1; ret=p;
    }
  }
  if (std::isinf(d)) throw Err() << "Can't find nearest point: empty line";
  return ret;
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

std::map<std::string, std::pair<int, int> > os_codes = {
{"SV", {0, 0}}, {"SW", {1, 0}}, {"SX", {2, 0}}, {"SY", {3, 0}}, {"SZ", {4, 0}}, {"TV", {5, 0}},
{"SR", {1, 1}}, {"SS", {2, 1}}, {"ST", {3, 1}}, {"SU", {4, 1}}, {"TQ", {5, 1}}, {"TR", {6, 1}},
{"SM", {1, 2}}, {"SN", {2, 2}}, {"SO", {3, 2}}, {"SP", {4, 2}}, {"TL", {5, 2}}, {"TM", {6, 2}},
{"SH", {2, 3}}, {"SJ", {3, 3}}, {"SK", {4, 3}}, {"TF", {5, 3}}, {"TG", {6, 3}}, {"SC", {2, 4}},
{"SD", {3, 4}}, {"SE", {4, 4}}, {"TA", {5, 4}}, {"NW", {1, 5}}, {"NX", {2, 5}}, {"NY", {3, 5}},
{"NZ", {4, 5}}, {"OV", {5, 5}}, {"NR", {1, 6}}, {"NS", {2, 6}}, {"NT", {3, 6}}, {"NU", {4, 6}},
{"NL", {0, 7}}, {"NM", {1, 7}}, {"NN", {2, 7}}, {"NO", {3, 7}},
{"NF", {0, 8}}, {"NG", {1, 8}}, {"NH", {2, 8}}, {"NJ", {3, 8}}, {"NK", {4, 8}},
{"NA", {0, 9}}, {"NB", {1, 9}}, {"NC", {2, 9}}, {"ND", {3, 9}},
{"HW", {1,10}}, {"HX", {2,10}}, {"HY", {3,10}}, {"HZ", {4,10}},
{"HT", {3,11}}, {"HU", {4,11}}, {"HP", {4,12}} };

dPoint
os_to_pt(const std::string & str){
  // check coordinate length
  int len = str.size() - 2;
  if (len<2 || len%2!=0) throw Err() << "bad OS reference length: " << str;
  len/=2;

  // read two-letter code
  auto c = os_codes.find(str.substr(0,2));
  if (c == os_codes.end()) throw Err() << "unknown OS letter code: " << str;

  double x = str_to_type<int>(str.substr(2, len));
  double y = str_to_type<int>(str.substr(2+len, len));
  x = x*pow(10,5-len) + c->second.first  * 1e5;
  y = y*pow(10,5-len) + c->second.second * 1e5;

  return dPoint(x,y);
}

#include "conv_geo.h"
#include "geo_nom/geo_nom.h"
dRect
nom_to_wgs(const std::string & name){
  nom_scale_t sc;
  auto r = nom_to_range(name, sc, true);
  ConvGeo cnv("SU_LL");
  // Default accuracy in frw_acc is 0.5, convert only corners:
  return cnv.frw_acc(r);
}
