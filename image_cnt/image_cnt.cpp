#include <set>
#include "err/err.h"
#include "geom/point_int.h"
#include "image_cnt.h"

// Coordinates of 4 data cell corners: [0,0] [0,1] [1,1] [1,0]
iPoint crn (int k){ k%=4; return iPoint(k/2, (k%3>0)?1:0); }

// We work with integer grid. Accuracy should be much smaller then 1.
const double pt_acc = 1e-4;

// push segment to multiline
void
push_seg(dMultiLine & ml, const dPoint & p1, const dPoint & p2){

  // Try to attach segment to an existing line.
  // Try both sides, assuming same segment orientation.
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

  // create new line
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

// Get value with bilinear interpolation
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
          const bool closed){
  size_t w = img.width(), h = img.height();

  // Step 1: find oriented segments
  std::map<double, dMultiLine> ret;
  for (int y=0; y<h-1; y++){
    for (int x=0; x<w-1; x++){

      iPoint p(x,y);
      // container for previous points inside each loop
      std::map<double, std::set<dPoint>> pts;

      // Crossing of all 4 data cell sides with contours
      // (coordinate v along the 4-segment line).
      for (int k=0; k<4; k++){
        iPoint p1 = p+crn(k);
        iPoint p2 = p+crn(k+1);

        // Set z coordinate to 1 on the boundary
        p1.z = (p1.x==0 || p1.x==w-1 || p1.y==0 || p1.y==h-1) ? 1:0;
        p2.z = (p2.x==0 || p2.x==w-1 || p2.y==0 || p2.y==h-1) ? 1:0;
        bool brd = p1.z && p2.z;

        auto v1 = img.get_double(p1.x, p1.y);
        auto v2 = img.get_double(p2.x, p2.y);
        if (std::isinf(v1) || std::isinf(v2)) throw Err() << "image_cnt: Inf values";
        if (std::isnan(v1) || std::isnan(v2)) throw Err() << "image_cnt: NaN values";

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

          // contour along the border if closed=true
          if (brd && closed && v1a>=vv && v2a>=vv){
            push_seg(ret[vv], p1, p2);
            continue;
          }

          if (v1a==v2a) continue;

          // find crossing of the cell side with the contour:
          double d = (vv-v1a)/(v2a-v1a);
          if ((d<0)||(d>=1)) continue;

          // One more hack with shift:
          // If crossing is too close to the corner then merge can fail.
          // We want points on different segments to be more then
          // pt_acc from each other. Shift by pt_acc on each side will give
          // sqrt(2)*pt_acc min distance.
          if (d<pt_acc)   d = pt_acc;
          if (d>1-pt_acc) d = 1-pt_acc;

          dPoint cr = (dPoint)p1 + (dPoint)(p2-p1)*d;

          // find pairs of crossings, add to the Multiline:
          if (!pts[vv].empty()){
            dPoint crp = *pts[vv].begin();
            pts[vv].clear();
            if (v1a>vv) push_seg(ret[vv], cr,crp);
            if (v2a>vv) push_seg(ret[vv], crp,cr);
          }
          else {
            pts[vv].insert(cr);
          }

          // segment along the border:
          if (brd && closed) {
            if (v1a>vv) push_seg(ret[vv], p1,cr);
            if (v2a>vv) push_seg(ret[vv], cr,p2);
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
  }

  return ret;
}

/********************************************************************/
void
image_cnt_vtol_filter(const ImageR & img,
  std::map<double, dMultiLine> & ret, const double vtol, const double R){

  size_t w = img.width(), h = img.height();

  for (auto & s:ret){
    auto v0 = s.first;
    auto & ml = s.second;

    // Iteratively minimize line length.
    if (vtol<=0 || R<=0) continue;

    for (auto & l:ml){
      if (l.size()<3) continue;
      double maxsh = 2*pt_acc;
      bool closed = dist(*l.begin(), *l.rbegin()) < pt_acc;
      if (closed) l.resize(l.size()-1);

      int maxiter=100;
      while (maxsh > pt_acc/2 && maxiter>0){
        maxiter--;
        maxsh = 0;
        for (auto i1 = l.begin(); i1!=l.end(); i1++){
          auto i2=i1+1;
          auto i3=(i2!=l.end()) ? i2+1: i2;

          if (closed){
            if (i2 == l.end()) {i2 =l.begin(); i3 = i2+1;}
            else if (i3 == l.end()) i3 = l.begin();
          }
          else if (i3 == l.end()) continue;

          if (dist(*i1,*i2)>1.5 || dist(*i2,*i3)>1.5) continue;

          // We have a triangle p1 p2 p3. with angle
          // We can move p2 to p2n, towards p4 = (p1+p3)/2.
          //   p2n = p2 + t*(p4-p2), with a parameter t.
          // We assume that height v(p2n) changes linearly
          //   v(t) = v(p2) + |p2n-p2|/|p4-p2| * (v(p4)-v(p2)).
          //        = v(p2) + t*(v(p4)-v(p2))

          // We minimize energy: potential energy + "tension".
          // Potential energy is
          //   Ep = ((v(t)-v0)/vtol)^2, it is 0 at v0 and 1 at v0+vtol
          // There are a few ways how we can introduce "tension":
          //   Et = -(p1-p2n)/|p1-p2n| * (p2n-p3)/|p2n-p3| -- minimize angle
          //   Et = (|p2n-p1|+|p2n-p3|)/|p3-p1| -- minimize normilised length
          //   Et = (|p2n-p4|/|p3-p1| * R/|p3-p1|)^2 -- minimize normalised distance between p2n and p4
          // The third one is the most convenient.
          // Total energy:
          //   E(t) = (A + B*t)^2 + C*(1-t)^2
          //   with A = (v(p2)-v0)/vtol, B = (v(p4)-v(p2))/vtol
          //        C = R^2*|p2-p4|^2/|p3-p1|^4

          dPoint p4 = (*i1+*i3)/2;
          double A = (img_int4(*i2, img) - v0)/vtol;
          double B = (img_int4(p4, img) - v0)/vtol;
          double C = pow(2,R) * pscal(*i2-p4, *i2-p4) / (pow,pscal(*i3-*i1, *i3-*i1),2);

          // Minimization gives:
          double tmin = (fabs(C+B*B)>1e-4)? (C-A*B)/(C+B*B) : 0;
          dPoint p2n = *i2 + tmin*(p4-*i2);

          // hard limit: we can't go beyond vtol
          if (fabs(img_int4(p2n, img) - v0) > vtol) continue;

          double sh = dist2d(p2n, *i2);
          if (sh>maxsh) maxsh=sh;

          *i2 += tmin*(p4-*i2);
          i2->z = 0;

        }
      }
      if (closed) l.push_back(*l.begin());
    }

    // remove empty
    // (vtol filter can keep lines with two same points)
    auto i1 = ml.begin();
    while (i1!=ml.end()){
      if (i1->size()<2 ||
         (i1->size()==2 && dist(*i1->begin(),*i1->rbegin())<pt_acc) ||
          i1->bbox().w * i1->bbox().h < 1.0) // collapsed contours
        i1=ml.erase(i1);
      else ++i1;
    }
  }
}

/********************************************************************/
ImageR
image_smooth_lim(const ImageR & img, const double dh, const double dr){
  if (dr<=0 || dh<=0) return img;
  size_t w = img.width();
  size_t h = img.height();
  ImageR ret(w,h, IMAGE_DOUBLE);

  for (size_t y=0; y<h; y++){
    for (size_t x=0; x<w; x++){
      double v = img.get_double(x,y);

      double s = v;
      double n = 1.0;
      // Average with Gaussian weight
      int ri = ceil(2*dr);
      for (int y1=y-ri; y1<=y+ri; y1++){
        if (y1<0 || y1>=h) continue;
        for (int x1=x-ri; x1<=x+ri; x1++){


          if (x1<0 || x1>=w) continue;
          if (x1==x && y1==y) continue;
          double dd = pow((double)x1-(double)x,2) + pow((double)y1-(double)y,2);
          double w = exp(-dd/2.0/pow(dr,2));
          s+=w*img.get_double(x1,y1);
          n+=w;
        }
      }
      double dv = s/n - v;

/**************/

// Following limits go to dh when difference -> inf

      // sharp limit
//      if (fabs(dv)>dh) dv*=dh/fabs(dv);

      // exponential limit (like ballistic viscosity in He3-B:)
//      if (dh>0 && dv!=0) dv = dh * dv/fabs(dv) * (1.0 - exp(-fabs(dv/dh)));
//      else dv=0;

      // 1/x limit
//      if (dh>0) dv *= 1.0/(fabs(dv)/dh+1.0);
//      else dv=0;

//      // one more exponential limit
//      if (dh>0) dv = dh*tanh(dv/dh);
//      else dv=0;

// Following limits go to 0 when difference -> inf
// (no changes at sharp ridges)

//    // sharp limit
      if (fabs(dv)>dh) dv*=dh*dh/dv/dv;

      // exponential limit
//      if (dh>0) dv *= exp(-0.5*fabs(dv)/dh);
//      else dv=0;

      ret.setD(x,y, v + dv);
    }
  }
  return ret;
}

/********************************************************************/
dLine
image_peaks(const ImageR & img, double DH, size_t PS, double minh){
  if (PS == 0) PS = img.width() * img.height();

  dLine ret;
  std::set<iPoint> done;
  for (int y=0; y<img.height(); y++){
    for (int x=0; x<img.width(); x++){

      iPoint p(x,y);
      if (done.count(p)>0) continue;
      double h0 = img.get_double(x,y);
      if (!std::isnan(minh) && h0<minh) continue;

      std::set<iPoint> pts, brd;
      add_set_and_border(p, pts, brd);
      do{
        // find maximum of the border
        double max = -INFINITY;
        iPoint maxpt;
        for (auto const & b:brd){
          if (img.check_crd(b.x, b.y)){
            double h1 = img.get_double(b.x, b.y);
            if (h1>max) {max = h1; maxpt=b;}
          }
          else {
            // is original point is too close to data edge
            if (dist(b,p)<1.5) {max = -INFINITY; break;}
          }
        }
        if (std::isinf(max)) break;

        // if max is higher then original point:
        if (max > h0) { break; }

        // if we descended more then DH or covered area more then PS:
        if ((h0 - max > DH ) || (pts.size() > PS)) {
          ret.emplace_back(x, y, h0);
          break;
        }
        add_set_and_border(maxpt, pts, brd);
        done.insert(maxpt);
      } while (true);
    }
  }
  return ret;
}
