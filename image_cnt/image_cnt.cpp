#include <set>
#include "err/err.h"
#include "image_cnt.h"

// Coordinates of 4 data cell corners: [0,0] [0,1] [1,1] [1,0]
iPoint crn (int k){ k%=4; return iPoint(k/2, (k%3>0)?1:0); }

// we use int64_t points with some factor for intermediate coordinate calculations
const double pt_acc = 1e-4;

// push segment to multiline
void
push_seg(dMultiLine & ml, const dPoint & p1, const dPoint & p2){

  for (auto & l:ml){
    if (l.size()==0) continue;
    if (dist(*l.rbegin(), p1) < pt_acc){
      l.push_back(p2);
      return;
    }
    if (dist(*l.begin(), p2) < pt_acc){
      l.insert(l.begin(), p1);
      return;
    }
  }

  dLine l;
  l.push_back(p1);
  l.push_back(p2);
  ml.push_back(l);
}

// Get range of image values along p1-p2 segment.
// Use only values on integer segment crossings.
dPoint
img_vrange(const dPoint & p1, const dPoint & p2, const ImageR & img){
  size_t w = img.width(), h = img.height();
  double min=+INFINITY, max=-INFINITY;

  // find crossing of p1-p2 and integer x lines
  if (p1.x!=p2.x){
    int x1 = ceil(std::min(p1.x,p2.x)), x2 = floor(std::max(p1.x,p2.x))+1;

    for (int x = std::max(0,x1); x<std::min((int)w,x2); ++x){
      double y = p1.y + (p2.y-p1.y)/(p2.x-p1.x) * (x-p1.x);

      if (!img.check_crd(x,floor(y)) || !img.check_crd(x,ceil(y))) continue;
      auto v1 = img.get_double(x, floor(y));
      auto v2 = img.get_double(x, ceil(y));
      double v = v1;
      if (ceil(y) != floor(y)) v = v1 + (v2-v1) * (y-floor(y));
      if (min > v) {min = v;}
      if (max < v) {max = v;}
    }
  }
  // same for y
  if (p1.y!=p2.y){
    int y1 = ceil(std::min(p1.y,p2.y)), y2 = floor(std::max(p1.y,p2.y))+1;
    for (int y = std::max(0,y1); y<std::min((int)h,y2); ++y){
      double x = p1.x + (p2.x-p1.x)/(p2.y-p1.y) * (y-p1.y);

      if (!img.check_crd(floor(x),y) || !img.check_crd(ceil(x),y)) continue;
      auto v1 = img.get_double(floor(x),y);
      auto v2 = img.get_double(ceil(x),y);
      double v = v1;
      if (ceil(x) != floor(x)) v = v1 + (v2-v1) * (x-floor(x));
      if (min > v) {min = v;}
      if (max < v) {max = v;}
    }
  }
  return dPoint(min,max);
}

// Get value
double img_int4(const dPoint & p, const ImageR & img){
  size_t w = img.width(), h = img.height();
  iPoint p1 = floor(p), p2 = ceil(p);

  if (p1.x<0) p1.x = 0; if (p1.x>=w) p1.x=w-1;
  if (p2.x<0) p2.x = 0; if (p2.x>=w) p2.x=w-1;
  if (p1.y<0) p1.y = 0; if (p1.y>=h) p1.y=h-1;
  if (p2.y<0) p2.y = 0; if (p2.y>=h) p2.y=h-1;

  double dx = p.x - p1.x;
  double dy = p.y - p1.y;
  double h1 = img.get_double(p1.x, p1.y);
  double h2 = img.get_double(p1.x, p2.y);
  double h3 = img.get_double(p2.x, p1.y);
  double h4 = img.get_double(p2.x, p2.y);
  return h1*(1-dx)*(1-dy) + h2*(1-dx)*dy + h3*dx*(1-dy) + h4*dy*dx;
}

/********************************************************************/
std::map<double, dMultiLine>
image_cnt(const ImageR & img,
          const double vmin, const double vmax, const double vstep,
          const bool closed, const double vtol){
  size_t w = img.width(), h = img.height();

  // Step 1: find oriented segments
  std::map<double, dMultiLine> ret;
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
        }
        if (std::isnan(vmax)) {
          max = (v1<v2)? v2:v1;
          max = ceil(max/vstep) * vstep;
        }
        if (vstep<=0) throw Err() << "image_cnt: positive step expected";
        if (min > max) throw Err() << "image_cnt: min > max";

        for (double vv=min; vv<=max; vv+=vstep){

          // Method have lots of problems if vv can be
          // equal to v1 or v2. Shift bad values:
          double sh = 1e-3*vstep;
          double v1a = (fabs(vv-v1)>sh/2)? v1 : v1 - sh;
          double v2a = (fabs(vv-v2)>sh/2)? v2 : v2 - sh;

          if (brd && closed && v1a>=vv && v2a>=vv){
            push_seg(ret[vv], p1, p2);
            continue;
          }

          if (v1a==v2a) continue;

          double d = (vv-v1a)/(v2a-v1a);
          if ((d<0)||(d>=1)) continue;

          dPoint cr = (dPoint)p1 + (dPoint)(p2-p1)*d;
          if (!pts[vv].empty()){
            dPoint crp = *pts[vv].begin();
            pts[vv].clear();
            if (v1>vv) push_seg(ret[vv], cr,crp);
            if (v2>vv) push_seg(ret[vv], crp,cr);
          }
          else {
            pts[vv].insert(cr);
          }

          if (brd && closed) {
            if (v1>vv) push_seg(ret[vv], p1,cr);
            if (v2>vv) push_seg(ret[vv], cr,p2);
          }
        }
      }
    }
  }

  // Step 2: merge segments, apply filters
  for (auto & s:ret){
    auto v0 = s.first;
    auto & ml = s.second;

    for (auto i1 = ml.begin(); i1!=ml.end(); ++i1){
      for (auto i2 = ml.begin(); i2!=ml.end(); ++i2){
        if (i1==i2) continue;
        if (i2->size()<2 || i1->size()<2) continue;
        if (dist(*i1->rbegin(), *i2->begin()) < pt_acc){
          i1->insert(i1->end(), i2->begin()+1, i2->end());
          i2->clear();
          continue;
        }
        if (dist(*i1->begin(), *i2->rbegin()) < pt_acc){
          i2->insert(i2->end(), i1->begin()+1, i1->end());
          i1->clear();
          continue;
        }
      }
    }

    // remove empty
    auto i1 = ml.begin();
    while (i1!=ml.end()){
      if (i1->size()==0) i1=ml.erase(i1);
      else ++i1;
    }

    // filter stright border lines
    for (auto & l:ml){
      auto i1 = l.begin();
      while (i1+2!=l.end()){
        auto i2=i1+1, i3=i1+2;
        if (i1->z && i2->z && i3->z &&
           (dist(*i1,*i2)<pt_acc || dist(*i2,*i3)<pt_acc ||
            dist(norm(*i2-*i1), norm(*i3-*i2)) < pt_acc)) l.erase(i2);
        else ++i1;
      }
    }

/*
    // V1 filter with vtol.
    // Remove points while altitude along the line is within the tolerance.
    if (vtol>0){
      for (auto & l:ml){
        auto i1 = l.begin();
        while (i1+2!=l.end()){
          auto i2=i1+1, i3=i1+2;
          dPoint rng = img_vrange(*i1, *i3, img);
          if (fabs(rng.x-v0) < vtol && fabs(rng.y-v0) < vtol ||
              std::isinf(rng.x) || std::isinf(rng.y))
            l.erase(i2);
          else ++i1;
        }
      }
    }
*/
    // V2 filter with vtol.
    // Iteratively minimize line length.
    if (vtol>0){
      for (auto & l:ml){
        double maxsh = 2*pt_acc;
        while (maxsh > pt_acc){
          maxsh = 0;
          for (auto i1 = l.begin(); i1+2!=l.end(); i1++){
            auto i2=i1+1, i3=i1+2;
            if (dist(*i1,*i2)>1.5 || dist(*i2,*i3)>1.5) continue;

            // can we move i2?
            dPoint p2 = (*i1+*i3)/2;
            double v2 = img_int4(p2, img);
            if (fabs(v2-v0) < vtol){
              double sh = dist2d(p2, *i2);
              if (sh>maxsh) maxsh=sh;
              *i2 = p2;
              i2->z = 0;
            }

          }
        }
        // filter stright lines
        auto i1 = l.begin();
        while (i1+2!=l.end()){
          auto i2=i1+1, i3=i1+2;
          if (dist(*i1,*i2)<pt_acc || dist(*i2,*i3)<pt_acc ||
              dist(norm(*i2-*i1), norm(*i3-*i2)) < pt_acc)
            l.erase(i2);
          else ++i1;
        }

      }
    }


    // remove empty
    // (vtol filter can keep lines with two same points)
    i1 = ml.begin();
    while (i1!=ml.end()){
      if (i1->size()<2 ||
         (i1->size()==2 && dist(*i1->begin(),*i1->rbegin())<pt_acc))
        i1=ml.erase(i1);
      else ++i1;
    }

  }

  return ret;
}

