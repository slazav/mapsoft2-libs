#include "geo_data.h"
#include "geo_utils.h"

dRect
GeoWptList::bbox() const {
  dRect ret;
  for (auto i:*this) ret = expand(ret, i);
  return ret;
}

void
GeoWptList::clear_alt() {
  for (auto & i:*this){ i.z = nan(""); }
}

dRect
GeoTrk::bbox() const {
  dRect ret;
  for (auto i:*this) ret.expand(i);
  return ret;
}

void
GeoTrk::clear_alt() {
  for (auto & i:*this) i.z = nan("");
}

double
GeoTrk::geo_length_2d() const {
  double ret=0;
  for (int i=0; i+1<size(); i++)
    ret += geo_dist_2d((*this)[i], (*this)[i+1]);
  return ret;
}

GeoTrk::GeoTrk(const dMultiLine & ml){
  for (const auto l:ml) {
    bool s = true;
    for (const auto pt:l) {
      GeoTpt tpt(pt);
      tpt.start = s;
      push_back(tpt);
      s = false;
    }
  }
}

GeoTrk::GeoTrk(const dLine & l){
  bool s = true;
  for (const auto pt:l) {
    GeoTpt tpt(pt);
    tpt.start = s;
    push_back(tpt);
    s = false;
  }
}

GeoTrk::operator dLine() const {
  dLine ret;
  for (auto i:*this) ret.push_back(dPoint(i));
  return ret;
}

GeoTrk::operator dMultiLine() const {
  dMultiLine ret;
  ret.resize(1);
  for (auto i:*this){
    if (i.start && ret.rbegin()->size()>0) ret.emplace_back();
    ret.rbegin()->push_back(dPoint(i));
  }
  if (ret.rbegin()->size()==0) ret.resize(ret.size()-1);
  return ret;
}


GeoMap operator* (const double k, const GeoMap & l) { return l*k; }

GeoMap operator+ (const dPoint & p, const GeoMap & l) { return l+p; }


dRect
GeoData::bbox_trks() const {
  dRect ret;
  for (auto i:trks) ret.expand(i.bbox());
  return ret;
}

dRect
GeoData::bbox_wpts() const {
  dRect ret;
  for (auto i:wpts) ret.expand(i.bbox());
  return ret;
}
