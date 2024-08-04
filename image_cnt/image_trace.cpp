#include <set>
#include <map>
#include <vector>
#include "image_trace.h"
#include "geom/point_int.h"
#include "geom/line.h"
#include "image/image_r.h"
#include "geom/point_int.h"


/* Functions for river/mountain tracing */

/********************************************************************/
// common object for all tracers
//
class trace_gear{
public:
  const ImageR & img;
  std::set<iPoint> P, B; // processed points, border of processed area
  int n,ns;              // counters: all points, areas without sink
  iPoint p0,p1,p2;       // starting point, last-step point, last processed point
  double h0,h1,h2;       // heights of p0,p1,p2

  trace_gear(const ImageR & img_, const iPoint & p0_): img(img_){

    if (!img.check_crd(p0.x, p0.y)) throw Err() << "trace_gear: point outside the image";
    n = ns = 0;
    p0 = p1 = p2 = p0_;
    h0 = h1 = h2 = img.get_double(p0.x, p0.y);

    P.insert(p0);
    for (int i=0; i<8; i++) B.insert(adjacent(p0,i));
  }

  // Process one more point
  // If it is possible, the point is lower then previous,
  // then ns is set to 0.
  // down parameter -- is the flow goes down (river) of up (mountain)
  // return false if step is not possible
  bool go(bool down){

    // find adjacent point (member of B) with minimum height
    h2 = NAN;
    for (const auto & b: B){
      if (!img.check_crd(b.x, b.y)) {
        p1=p2; h1=h2; // do the last step to the map edge (is it good?)
        h2=NAN; break;
      }
      double hh = img.get_double(b.x, b.y);
      if (std::isnan(h2) ||
         (!down && hh > h2) ||
         ( down && hh < h2)) {
        h2=hh; p2=b;
      }
    }

    // can't move
    if (std::isnan(h2)){
      return false;
    }

    // add found point into P, recalculate B
    P.insert(p2);
    B.erase(p2);
    for (int i=0; i<8; i++){
      iPoint b = adjacent(p2, i);
      if (P.count(b)==0) B.insert(b);
    }

    n++; ns++;
    // If found point is lower than that on a previous step,
    // do the next step, reset ns and as counters
    if ((!down && h2 > h1) || (down && h2 < h1)) {
      h1=h2; p1=p2; ns=0;
      P.clear(); B.clear();
      P.insert(p1);
      for (int i=0; i<8; i++) B.insert(adjacent(p1,i));
    }
    return true;
  }
};


/********************************************************************/
// Trace one river.
// Parameters:
//    p0 -- starting point
//    nmax -- no-sink area when we stop calculation
//    hmin -- threshold height where we want to stop calculation
//    down -- is the flow goes down (river) of up (mountain)
iLine
trace_river(const ImageR & img, const iPoint & p0, int nmax, int hmin, bool down){

  trace_gear G(img, p0);
  std::vector<iPoint> L; // sorted vector of our steps
  L.push_back(p0);

  // do steps down
  while (G.go(down)) {
    L.push_back(G.p2);

    // If we processed > nmax points but can't do step down, it's
    // a no-sink point:
    if (G.ns>nmax) break;
    if (G.h1<hmin) break;
  }

  // Trace back from the last p1 to p0 to build the river line
  // On each step look for adjecent and the earliest point in L.
  iLine ret;
  iPoint p = G.p1;
  ret.push_back(p);
  while (p!=p0){
    for (const auto & b:L){
      if (is_adjacent(b, p)<0) continue;
      p=b;
      ret.push_back(p);
      break;
    }
  }
  return ret;
}

/********************************************************************/
// trace rectangular map

// Helper function for trace_map().
// Go down to a processed point, no-sink point or image border.
//   img -- should support get_double()
//   dirs -- should have IMAGE_8 type and same size as img.
//   p0 -- should be inside img/dirs


void
trace_go_down(const ImageR & img, const iPoint & p0, ImageR & dirs, int nmax, bool down){

  trace_gear G(img, p0);

  std::vector<iPoint> L; // ordeered list of processed points
  L.push_back(p0);

  while (G.go(down)) {
    L.push_back(G.p2);

    // If we processed > nmax points but can't do step down, it's
    // a no-sink point:
    if (nmax && G.ns>nmax) break;

    // if we reach already processed point:
    if (dirs.get8(G.p2.x, G.p2.y)!=255) {
      G.p1 = G.p2;
      break;
    }
  }

  iPoint p = G.p1;

  if (dirs.get8(p.x, p.y)==255)
    dirs.set8(p.x, p.y, 8);

  // do way back to the starting point
  while (p!=p0){
    for (const auto & p1 : L){
      int dir = is_adjacent(p1, p);
      if (dir==-1) continue;
      p=p1;
      dirs.set8(p.x, p.y, dir);
      break;
    }
  }
}

ImageR
trace_map_dirs(const ImageR & img, int nmax, bool down){
  size_t w = img.width(), h=img.height();

  // sink direction 0..7, 8 for no-sink, 255 for unknown
  ImageR dirs(w,h, IMAGE_8); dirs.fill8(255);

  // For each new point go down to a processed point,
  //  no-sink point or image border.
  for (size_t y=0; y<h; y++){
    for (size_t x=0; x<w; x++){
      if (dirs.get8(x,y)!=255) continue;
      trace_go_down(img, iPoint(x,y), dirs, nmax, down);
    }
  }
  return dirs;
}

ImageR
trace_map_areas(const ImageR & dirs){
  if (dirs.type() != IMAGE_8)
    throw Err() << "trace_map_areas: wrong image type";
  size_t w = dirs.width(), h=dirs.height();

  // calculate sink areas
  ImageR areas(w,h, IMAGE_DOUBLE); areas.fillD(0.0);
  for (size_t y=0; y<h; y++){
    for (size_t x=0; x<w; x++){
      iPoint p = iPoint(x, y);

      while (dirs.check_crd(p.x, p.y)) {
        areas.setD(p.x,p.y, areas.getD(p.x,p.y) + 1.0);
        int dir = dirs.get8(p.x,p.y);
        if (dir < 0 || dir > 7) break;
        p = adjacent(p, dir);
      }
    }
  }
  return areas;
}


// RMS difference from a best-fit plane 2r+1 x 2r+1
double plane_dev(const Image & dem, const iPoint & p, const int r){
  if (r<1) return 0.0;

  // Let's calculate coordinates from p and altitude from h(p)
  // RMS difference is D = avr(h(x,y) - A*x - B*y)^2
  // best fit plane: dD/dA = 0, dD/dB = 0,
  //   A* avr(x^2) + B*avr(x*y) = avr(h*x)
  //   A* avr(x*y) + B*avr(y^2) = avr(h*y)
  // A = (sum(h*x)*sum(y^2) - sum(h*y)*sum(x*y)) / (sum(x^2)*sum(y^2) - sum(x*y)^2)
  // B = (sum(h*y)*sum(x^2) - sum(h*x)*sum(x*y)) / (sum(x^2)*sum(y^2) - sum(x*y)^2)
  double sn(0), shh(0), shx(0), shy(0), sxy(0), sxx(0), syy(0);
  double h0 = dem.get_double(p.x, p.y);
  for (int x = -r; x<=r; x++){
    for (int y = -r; y<=r; y++){
      if (!dem.check_crd(p.x+x,p.y+y)) continue;
      double h = dem.get_double(p.x+x, p.y+y) - h0;
      shx += x*h; shy += y*h;
      sxy += x*y; sxx += x*x; syy += y*y;
      shh += h*h;
      sn+=1;
    }
  }
  double det = sxx*syy - sxy*sxy;
  if (det == 0) return 0;
  double A = (shx*syy - shy*sxy)/det;
  double B = (shy*sxx - shx*sxy)/det;
  double D = shh + A*A*sxx + B*B*syy - 2*A*shx - 2*B*shy + 2*A*B*sxy;
  return sqrt(D)/sn;
}

// RMS difference from horizontal line perpendicular to dir
double perp_dev(const Image & dem, const iPoint & p, const int dir, const int r){
  if (r<1) return 0.0;

  if (!dem.check_crd(p.x,p.y)) return 0.0;
  double h0 = dem.get_double(p.x, p.y);
  iPoint p1(p), p2(p);
  double s(0.0);
  double n(0.0);
  for (int x = 1; x<=r; x++){
    p1 = adjacent(p1, dir+2);
    if (dem.check_crd(p1.x,p1.y)){
      double dh = dem.get_double(p1.x, p1.y) - h0;
      s += pow(dh,2);
      n += 1.0;
    }
    p2 = adjacent(p2, dir-2);
    if (dem.check_crd(p2.x,p2.y)){
      double dh = dem.get_double(p2.x, p2.y) - h0;
      s += pow(dh,2);
      n += 1.0;
    }
  }
  return sqrt(s)/n;
}


dMultiLine
trace_map(const ImageR & dem, const int nmax, const bool down, const double mina,
          const start_detect_t start_detect, const double start_par,
          const size_t smooth_passes){

  ImageR dirs = trace_map_dirs(dem, nmax, down);

  size_t w = dirs.width(), h=dirs.height();
  if (dirs.type() != IMAGE_8)
    throw Err() << "trace_map: wrong image type";
  if (dem.width() != w || dem.height() != h)
    throw Err() << "trace_map: wrong image dimensions";

  // calculate sink areas and sum of altitudes in these areas
  ImageR areas(w,h, IMAGE_DOUBLE); areas.fillD(0.0);
  ImageR hdiff(w,h, IMAGE_DOUBLE); hdiff.fillD(0.0);
  for (size_t y=0; y<h; y++){
    for (size_t x=0; x<w; x++){
      iPoint p = iPoint(x, y);
      double H = dem.get_double(p.x,p.y);

      while (dirs.check_crd(p.x, p.y)) {
        double A = areas.getD(p.x,p.y);
        areas.setD(p.x,p.y, A + 1);
        if (start_detect == TRACE_START_MINDH){
          double S = hdiff.getD(p.x,p.y);
          hdiff.setD(p.x,p.y, S + H);
        }
        int dir = dirs.get8(p.x,p.y);
        if (dir < 0 || dir > 7) break;
        p = adjacent(p, dir);
      }
    }
  }

  // Make set of all points with large enough area,
  // update hdiff image to have h - s/a
  std::map<iPoint, double> pts;
  for (size_t y=0; y<h; y++){
    for (size_t x=0; x<w; x++){
      double A = areas.getD(x,y);
      if (start_detect == TRACE_START_MINDH){
        double H = dem.get_double(x,y);
        double S = hdiff.getD(x,y);
        double dh = fabs(H - S/A);
        hdiff.setD(x,y, dh);
      }
      if (A > mina) pts.emplace(iPoint(x,y), A);
    }
  }

  // trace rivers/ridges
  dMultiLine ret;
  while (pts.size()){

    // alwaws start with a point with smallest area
    double A0 = pts.begin()->second;
    iPoint p = pts.begin()->first;
    for (const auto & i: pts){
      if (A0 > i.second) {A0 = i.second; p = i.first;}
    }

    // Visibility flag. It could be different conditions
    // for setting and clearing it.
    bool visible = false;

    // Start from this point and go along the river/ridge
    // Note that area always increase on this way, hdiff - not.
    dLine l;
    // std::cerr << "start trace: " << p << " " << A0 << "\n";
    while (dirs.check_crd(p.x, p.y)) {

      double A = areas.getD(p.x,p.y);
      int dir = dirs.get8(p.x, p.y);

      // select where to make the trace visible (only once)
      if (!visible) {
        switch (start_detect) {
        case TRACE_START_MINDH: {
          double dH = hdiff.getD(p.x,p.y);
          if (dH > start_par) visible = true;
          break;
        }
        case TRACE_START_SIDEH2: {
          // double step in perpendicular direction
          auto p1 = adjacent(p, dir+2); p1 = adjacent(p1, dir+2);
          auto p2 = adjacent(p, dir-2); p2 = adjacent(p2, dir-2);
          if (dem.check_crd(p1.x, p1.y) && dem.check_crd(p2.x, p2.y)){
            double H0 = dem.get_double(p.x, p.y);
            double dH1 = H0 - dem.get_double(p1.x, p1.y);
            double dH2 = H0 - dem.get_double(p2.x, p2.y);
            if (down) {dH1 = -dH1; dH2 = -dH2;}
            if(dH1 > start_par && dH2 > start_par) visible = true;
          }
          break;
        }
        case TRACE_START_PERP3: {
          double dev = perp_dev(dem, p, dir, 3);
          if (dev > start_par) visible = true;
          break;
        }
        case TRACE_START_PERP4: {
          double dev = perp_dev(dem, p, dir, 4);
          if (dev > start_par) visible = true;
          break;
        }
        case TRACE_START_PL3:{
          double dev = plane_dev(dem, p, 3);
          if (dev > start_par) visible = true;
          break;
        }
        case TRACE_START_PL4:{
          double dev = plane_dev(dem, p, 4);
          if (dev > start_par) visible = true;
          break;
        }
        case TRACE_START_NONE:
          visible = true;
          break;
        }
      }

      // is it a node?
      bool node = A > A0+mina;
      A0 = A;

      // std::cerr << "  " << p << " A=" << A << " dir=" << dir
      //   << (visible ? " vis":"") << (node ? " *":"") << "\n";

      if (visible) l.push_back(dPoint(p.x, p.y, node? 1:0));
      else
      if (l.size()>1) {
        ret.push_back(l);
        l.clear();
      }
      if (pts.count(p)==0) break; // stop at processed point
      if (!visible && node) break; // stop if a non-visible river meet another river (it could be visible)

      pts.erase(p);
      if (dir < 0 || dir > 7) break; // stop at the end of trace

      p = adjacent(p, dir);
    }
    // std::cerr << "  stop: " << (l.size() ? l[l.size()-1] : dPoint()) << " " << l.size() << "pts \n";
    if (l.size()>1) ret.push_back(l);
  }

  // smooth lines within pixel precision
  for (size_t i =0; i<smooth_passes; i++){
    for (auto & l:ret){
      for (size_t i = 0; i+2<l.size(); i++){
        if (l[i+1].z != 0.0) continue;
        l[i+1] = (l[i] + l[i+1])/2;
        l[i+1].z = 0.0;
      }
    }
  }

  return ret;
}

dMultiLine
trace_map2(const ImageR & dem, const int nmax, const bool down, const double mina){

  ImageR dirs = trace_map_dirs(dem, nmax, down);

  size_t w = dirs.width(), h=dirs.height();
  if (dirs.type() != IMAGE_8)
    throw Err() << "trace_map: wrong image type";
  if (dem.width() != w || dem.height() != h)
    throw Err() << "trace_map: wrong image dimensions";

  // calculate sink areas
  ImageR areas(w,h, IMAGE_DOUBLE); areas.fillD(0.0);
  for (size_t y=0; y<h; y++){
    for (size_t x=0; x<w; x++){
      iPoint p = iPoint(x, y);
      while (dirs.check_crd(p.x, p.y)) {
        double A = areas.getD(p.x,p.y);
        areas.setD(p.x,p.y, A + 1);
        int dir = dirs.get8(p.x,p.y);
        if (dir < 0 || dir > 7) break;
        p = adjacent(p, dir);
      }
    }
  }

  // Make set of all points with large enough area:
  std::map<iPoint, double> pts;
  for (size_t y=0; y<h; y++){
    for (size_t x=0; x<w; x++){
      double A = areas.getD(x,y);
      if ( A > mina) pts.emplace(iPoint(x,y), A);
    }
  }

  // trace rivers/ridges
  dMultiLine ret;
  while (pts.size()){

    // Always start with a point with smallest area
    double A0 = pts.begin()->second;
    iPoint p = pts.begin()->first;
    for (const auto & i: pts){
      if (A0 > i.second) {A0 = i.second; p = i.first;}
    }

    // Start from this point and go along the river/ridge
    // Note that area always increase on this way
    dLine l;
    // std::cerr << "start trace: " << p << " " << A0 << "\n";
    while (dirs.check_crd(p.x, p.y)) {

      double A = areas.getD(p.x,p.y);
      int dir = dirs.get8(p.x, p.y);
      // std::cerr << "  " << p << " A=" << A << " dir=" << dir << "\n";

      //detect nodes and break line
      bool node = false;
      if (l.size()){
        for (int i = 0; i < 8; i++){
          if (dir == i) continue; // forward direction
          iPoint p1 = adjacent(p, i);
          if (p1 == *l.rbegin()) continue; // backward dir
          if ((i+4)%8 != dirs.get8(p1.x, p1.y)) continue; // wrong dir
          if (areas.getD(p1.x,p1.y) <=mina ) continue;
          node = true;
          // std::cerr << "node\n";
          break;
        }
      }

      l.push_back(p);
      if (node) break;

      if (pts.count(p)==0) break; // stop at processed point
      pts.erase(p);

      if (dir < 0 || dir > 7) break; // stop at the end of trace
      p = adjacent(p, dir);
    }
    // std::cerr << "  stop: " << (l.size() ? l[l.size()-1] : dPoint()) << " " << l.size() << "pts \n";
    if (l.size()) ret.push_back(l);
  }
  return ret;
}



/********************************************************************/
// Trace area of one river/mountain
//
/*
class trace_area{
  public:
  std::set<iPoint> done;          // processed points
  std::map<iPoint,double> areas;  // processed areas
  std::map<iPoint,char> dirs;     // directions of flow

  bool down;   // is the flow goes down (river) of up (mountain)
  int dh;      // max height difference of a "wrong sink"
  int maxp;    // max size of a "wrong sink"
  double mina; // collect data for rivers larger then mina area, km^2
  SRTM3 & S;   // SRTM data

  std::set<iPoint> stop; // "stop segment", calculation never cross it.

  double maxa; // max area (-1 for no limit)
  double suma;

  // constructor: initialize some parameters
  trace_area(SRTM3 & S_, int dh_, int maxp_, double mina_, bool down_):
          S(S_), dh(dh_), maxp(maxp_), mina(mina_), maxa(-1), suma(0), down(down_){ }

  // set stop segment, return starting point for it (minimum on the right side)
  iPoint set_stop_segment(const iPoint & p1, const iPoint & p2){
    // stop segment - thick line between points:
    stop = brez(p1,p2,1,0);
    // shifted right:
    std::set<iPoint> init = brez(p1,p2,0,1);
    iPoint pm = p1;
    short  hm = S.geth(pm,true);
    std::set<iPoint>::const_iterator i;
    for (i=init.begin(); i!=init.end(); i++){
      short h = S.geth(*i,true);
      if ((down && h < hm) || (!down && h > hm)) {hm=h, pm=*i;}
    }
    return pm;
  }

  // is there a sink from p1 to p2?
  bool is_flow(const iPoint &p1, const iPoint &p2){

    trace_gear G(S, p1);
    short h_thr = S.geth(p2,true) + (down? -dh: dh);

    while(1) {
      iPoint p=G.go(down);
      if (p==p2) return true;
      // already processed point
      if (done.count(p)) return false;
      // stop segment
      if (stop.count(p)) return false;
      // maxp limit
      if (G.n > maxp) return false;
      // dh limit
      int h=S.geth(p,true);
      if ((down && h < h_thr) || (!down && h > h_thr)) return false;
    }
    return false;
  }

  // recursively get area
  double get_a(const iPoint &p0){
    double a=S.area(p0) * 1e-6; // dot area in km^2

    if (maxa > 0 && (suma+=a) > maxa) return 0;

    done.insert(p0);
    for (int i=0; i<8; i++){
      iPoint p = adjacent(p0,i);
      if (done.count(p) || stop.count(p) || !is_flow(p, p0)) continue;
      double a1 = get_a(p);
      if (a1>mina) dirs[p]=i;
      a+=a1;
    }
    if (a>mina) areas[p0]=a;
    return a;
  }

  // sort rivers
  std::list<std::list<iPoint> > sort_areas(){

    std::list<std::list<iPoint> > ret;
    std::set<iPoint> done;

    while(1){
      std::list<iPoint> riv;

      // find max area
      double ma = -1;
      iPoint mp;
      std::map<iPoint,double>::const_iterator ai;
      std::map<iPoint,char>::const_iterator di;
      for (ai=areas.begin(); ai!=areas.end(); ai++){
        if (done.count(ai->first) || ma >= ai->second ) continue;
        ma=ai->second; mp=ai->first;
      }

      // all points processed
      if (ma<0) break;

      // go up from found point
      while(1){
        done.insert(mp);
        riv.push_back(mp);
        ma = -1;
        iPoint mp1;
        // found adjacent point which flows to mp with maximal area
        for (int i=0; i<8; i++){
          iPoint p=adjacent(mp,i);
          ai = areas.find(p);
          di = dirs.find(p);
          if (di == dirs.end() || ai == areas.end() ||
              di->second != i || ma >= ai->second) continue;

          ma=ai->second; mp1=p;
        }
        if (ma<0) break;
        mp=mp1;
      }
      ret.push_back(riv);
    }
    return ret;
  }

};
*/

