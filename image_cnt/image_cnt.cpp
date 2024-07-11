#include <set>
#include "err/err.h"
#include "image_cnt.h"

// Coordinates of 4 data cell corners: [0,0] [0,1] [1,1] [1,0]
iPoint crn (int k, int kx=1){ k%=4; return iPoint(kx*(k/2), (k%3>0)?1:0); }

// Directions of 4 data cell sides
iPoint dir (int k, int kx=1){ return crn(k+1, kx)-crn(k, kx); }

// we use int64_t points with some factor for intermediate coordinate calculations
const double pt_acc = 1e-4;
typedef Point<int64_t> lPoint;

// push segment with 1/pt_acc factor; z-coordinate is point type
void
push_seg(std::multimap<lPoint, lPoint> & seg, const dPoint & p1, const dPoint & p2){
  seg.emplace(lPoint(p1.x/pt_acc, p1.y/pt_acc, p1.z), lPoint(p2.x/pt_acc,p2.y/pt_acc, p2.z));
}

// Erase a key-value pair from multimap
void erase_kv (std::multimap<lPoint, lPoint> & mm, const lPoint & k, const lPoint &v){
  auto i = mm.lower_bound(k);
  while (i != mm.upper_bound(k)){
    if (i->second == v) i=mm.erase(i);
    else i++;
  }
}

// Merge oriented segments
dMultiLine
merge_cntr(std::multimap<lPoint, lPoint> & segs) {

  dMultiLine ret;
  while (segs.size()){
    dLine l;
    auto p1 = segs.begin()->first;
    auto p2 = segs.begin()->second;
    l.emplace_back(p1.x*pt_acc, p1.y*pt_acc, p1.z);
    l.emplace_back(p2.x*pt_acc, p2.y*pt_acc, p2.z);
    erase_kv(segs, p1, p2);

    while (segs.count(p2)){
      auto p3 = segs.find(p2)->second;
      // filter stright lines (mostly for borders):
      if (dist(norm(p2-p1),norm(p3-p2)) < pt_acc) l.resize(l.size()-1);
      l.emplace_back(p3.x*pt_acc, p3.y*pt_acc, p3.z);
      erase_kv(segs, p2, p3);
      p1 = p2;
      p2 = p3;
    }
    if (l.size()) ret.push_back(l);
  }
  return ret;
}

// filter a single line (one step), return number of modifications
size_t
filter_line(dLine & line, const ImageR & img, const double v0, const double vtol){
  size_t ret = 0;
  size_t s = line.size();
  for (size_t i=0; i<s; i++){
    dPoint & p0 = line[i];
    if (p0.z) continue;
    const dPoint & p1 = line[i>0? i-1:s-1];
    const dPoint & p2 = line[i<s-1? i+1:0];

    // All points are at integer x or integer y (or both!)
    // Also require that p1.x and p2.x are on different sides of p.x.
    if (fabs(p0.x-rint(p0.x)) < pt_acc) {
      // we would like to move the point towards p1-p2 line (or to (p1.y+p2.y)/2)
      double dy = (p1.y+p2.y)/2 - p0.y;
      if ((p2.x-p0.x)*(p0.x-p1.x)>0)
        dy = p1.y + (p2.y-p1.y)*(p0.x-p1.x)/(p2.x-p1.x) - p0.y;

      double dy1 = 0;
      // let's see how far we can move in terms of img values and vtol
      int y1 = dy>0 ? floor(p0.y)+1 : ceil(p0.y)-1;
      int y2 = dy>0 ? ceil(p0.y+dy)+1 : floor(p0.y+dy)-1;
      int sy = dy>0 ? +1 : -1;
      for (int yy = y1; yy != y2; yy+=sy){
        double v2 = img.get_double(rint(p0.x), yy);
        // point itself should be always within tolerance
        // if next node within tolerance - move there:
        if (fabs(v2-v0)<vtol) {
          dy1 = yy - p0.y;
          continue;
        }
        else { // move to exact tolerance
          double vv = v2 + ((v2-v0>vtol) ? vtol:-vtol);
          double v1 = img.get_double(rint(p0.x), yy-sy);
          double d = double(vv-v1)/double(v2-v1);
          if ((d<0)||(d>=1)) continue;
          dy1 = yy-sy+d - p0.y;
        }
      }
      if (dy1 != 0.0) {
        if (fabs(dy1)>fabs(dy)) dy1=dy;
        p0.y += dy1;
        ret++;
      }
    }

    // same for y:
    if (fabs(p0.y-rint(p0.y)) < pt_acc && (p2.y-p0.y)*(p0.y-p1.y)>0) {
      double dx = (p1.x+p2.x)/2 - p0.x;
      if ((p2.y-p0.y)*(p0.y-p1.y)>0)
        dx = p1.x + (p2.x-p1.x)*(p0.y-p1.y)/(p2.y-p1.y) - p0.x;
      double dx1 = 0;
      int x1 = dx>0 ? floor(p0.x)+1 : ceil(p0.x)-1;
      int x2 = dx>0 ? ceil(p0.x+dx)+1 : floor(p0.x+dx)-1;
      int sx = dx>0 ? +1 : -1;
      for (int xx = x1; xx != x2; xx+=sx){
        double v2 = img.get_double(xx, rint(p0.y));
        if (fabs(v2-v0)<vtol) {
          dx1 = xx - p0.x; // move to yy and go to next cell
          continue;
        }
        else { // move to exact tolerance
          double vv = v2 + ((v2-v0>vtol) ? vtol:-vtol);
          double v1 = img.get_double(xx-sx, rint(p0.x));
          double d = double(vv-v1)/double(v2-v1);
          if ((d<0)||(d>=1)) continue;
          dx1 = xx-sx+d - p0.x;
        }
        // else -- maybe move to exact tolerance
      }
      if (dx1 != 0.0) {
        if (fabs(dx1)>fabs(dx)) dx1=dx;
        p0.x += dx1;
        ret++;
      }
    }
  }
  // remove points on one line
  auto it = line.begin();
  while (it!=line.end()){
    dPoint p0 = *it;
    dPoint p1 = (it==line.begin()) ? *line.rbegin() : *(it-1);
    dPoint p2 = (it+1==line.end()) ? *line.begin() : *(it+1);

    if (dist(p0,p1) < pt_acc ||
        dist(p0,p2) < pt_acc ||
        dist(norm(p1-p0),norm(p2-p0)) < pt_acc)
          it = line.erase(it);
    else it++;
  }

  return ret;
}

/********************************************************************/
std::map<double, dMultiLine>
image_cnt(const ImageR & img,
          const double vmin, const double vmax, const double vstep,
          const bool closed, const double vtol){
  size_t w = img.width(), h = img.height();

  // Step 1: find oriented segments:
  std::map<double, std::multimap<lPoint, lPoint> > segs;
  for (int y=0; y<h-1; y++){
    for (int x=0; x<w-1; x++){

      iPoint p(x,y);
      std::map<double, std::set<dPoint>> pts;

      // Crossing of all 4 data cell sides with contours
      // (coordinate v along the 4-segment line).
      for (int k=0; k<4; k++){
        iPoint p1 = p+crn(k);
        iPoint p2 = p+crn(k+1);
        p1.z = (p1.x==0 || p1.x==w-1 || p1.y==0 || p1.y==h-1) ? 1:0;
        p2.z = (p2.x==0 || p2.x==w-1 || p2.y==0 || p2.y==h-1) ? 1:0;

        bool brd = p1.z && p2.z;
        auto v1 = img.get_double(p1.x, p1.y);
        auto v2 = img.get_double(p2.x, p2.y);

        // which countours we want to find
        double min(vmin), max(vmax);
        if (std::isnan(vmin)) {
          min = (v1<v2)? v1:v2;
          min = floor(min/vstep) * vstep;
          max = (v1<v2)? v2:v1;
          max = ceil(max/vstep) * vstep;
        }
        if (std::isnan(vmax)) {
          max = (v1<v2)? v2:v1;
          max = ceil(max/vstep) * vstep;
        }
        if (vstep<=0) throw Err() << "image_cnt: positive step expected";
        if (min > max) throw Err() << "image_cnt: min > max";

        for (double vv=min; vv<=max; vv+=vstep){
          if (brd && closed && v1>=vv && v2>=vv)
            push_seg(segs[vv], p1, p2);

          if (v1==v2) continue;

          double d = double(vv-v1)/double(v2-v1);
          if ((d<0)||(d>=1)) continue;

          dPoint cr = (dPoint)p1 + (dPoint)(p2-p1)*d;
          if (!pts[vv].empty()){
            dPoint crp = *pts[vv].begin();
            pts[vv].clear();
            if (v1>vv) push_seg(segs[vv], cr,crp);
            if (v2>vv) push_seg(segs[vv], crp,cr);
          }
          else {
            pts[vv].insert(cr);
          }

          if (brd && closed) {
            if (v1>vv) push_seg(segs[vv], p1,cr);
            if (v2>vv) push_seg(segs[vv], cr,p2);
          }
        }
      }
    }
  }

  // Step 2: merge segments into Multilines
  std::map<double, dMultiLine> ret;
  for (auto & s:segs){
    ret[s.first] = merge_cntr(s.second);
  }

  // Step 3: filter
  // filter lines
  if (vtol>0) {
    for (auto & ll:ret){
      auto v0 = ll.first;
      for (auto & line:ll.second){
        if (line.size() < 1) continue;
        filter_line(line, img, v0, vtol);
      }
    }
  }

  return ret;
}

