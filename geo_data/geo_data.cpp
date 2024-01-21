#include "geo_data.h"

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
  for (const auto & seg:*this)
    for (const auto & pt:seg)
      ret.expand(pt);
  return ret;
}

void
GeoTrk::clear_alt() {
  for (auto & seg:*this)
    for (auto & pt:seg)
      pt.z = nan("");
}

GeoTrk::GeoTrk(const dMultiLine & ml){
  clear();
  for (const auto & l:ml) {
    GeoTrkSeg seg;
    for (const auto & pt:l) seg.push_back((GeoTpt)pt);
    push_back(seg);
  }
}

GeoTrk::GeoTrk(const dLine & l){
  clear();
  GeoTrkSeg seg;
  for (const auto & pt:l) seg.push_back((GeoTpt)pt);
  push_back(seg);
}

GeoTrk::operator dLine() const {
  dLine ret;
  for (const auto l:*this) {
    for (const auto pt:l) ret.push_back(pt);
  }
  return ret;
}

GeoTrk::operator dMultiLine() const {
  dMultiLine ret;
  for (const auto seg:*this) {
    dLine l;
    for (const auto pt:seg) l.push_back(pt);
    ret.push_back(l);
  }
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
