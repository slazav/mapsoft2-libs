#ifndef POLY_TOOLS_H
#define POLY_TOOLS_H

#include "multiline.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <set>

/* Extra polygon-related functions */

/// Class for checking if a point is inside a polygon.
/// Line is always treated as closed.
template <typename CT, typename PT>
class PolyTester{
  std::vector<Point<CT> > sb,se;    // segment beginning, ending
  std::vector<double> ss;           // segment slope
  std::vector<PolyTester> mtesters; // for MultiLine tester
  bool horiz; // test direction
public:

  // Constructor, build the tester class for a given polygon.
  // Parameters:
  //  - L -- polygon (represented by Line object)
  //  - horiz -- set test direction
  PolyTester(const Line<CT,PT> & L, const bool horiz_ = true): horiz(horiz_){
    // Collect line sides info: start and end points, slopes
    int pts = L.size();
    for (int i = 0; i < pts; i++){
      Point<CT> b(L[i%pts].x, L[i%pts].y),
                e(L[(i+1)%pts].x, L[(i+1)%pts].y);
      if (!horiz){ // swap x and y
        b.y = L[i%pts].x; b.x=L[i%pts].y;
        e.y = L[(i+1)%pts].x;
        e.x = L[(i+1)%pts].y;
      }

      // skip horizontal segments
      if (b.y==e.y) continue;

      // side slope
      double s = double(e.x-b.x)/double(e.y-b.y);
      sb.push_back(b);
      se.push_back(e);
      ss.push_back(s);
    }
  }

  // PolyTester for multiline: keep testers for all segments
  PolyTester(const MultiLine<CT,PT> & L, const bool horiz_ = true): horiz(horiz_){
    for (const auto & seg:L)
      mtesters.push_back(PolyTester(seg, horiz));
  }


  // Get sorted coordinates of the polygon crossings with
  // y=const (or x=const if horiz=false) line.
  // Returns sorted dPoint array where first point coordinates
  // are crossing positions, second are "crossing lengths" (normally
  // zero, non-zero if segment coinsides with y=const line.
  // Length is calculated to the left of the point.
  std::vector<dPoint> get_cr(CT y) const{
    std::vector<dPoint> cr;

    // multiline: merge and sort crossings for all multiline segments:
    if (mtesters.size()>0){
      for (auto const & tst: mtesters){
        auto c = tst.get_cr(y);
        cr.insert(cr.end(), c.begin(), c.end());
      }
      sort(cr.begin(), cr.end());
      return cr;
    }

    // single-line tester
    for (size_t k = 0; k < sb.size(); k++){

      // Skip segments which are fully above or below the line
      if ((sb[k].y > y)&&(se[k].y > y)) continue;
      if ((sb[k].y < y)&&(se[k].y < y)) continue;

      // If segment touches the line with its first or last point
      // it could be 1 or 2 actual crossings. Use only the first
      // point to avoid duplications (adjecent segment also has this crossings).
      if (sb[k].y == y){
        // index if the previous segment
        int kp = k>0? k-1:sb.size()-1;

        // It could be that x coordinates are different (horizontal
        // segment was skipped). Then length of the crossing is not zero.
        double x = std::max(sb[k].x, se[kp].x);
        double l = fabs(sb[k].x - se[kp].x);

        // Segments on both sides of the line - one crossing.
        if ((sb[kp].y < y && se[k].y > y) ||
            (sb[kp].y > y && se[k].y < y)){
          cr.push_back(dPoint(x,l));
        }
        // Segments on one side of the line -- two crossings
        else {
          cr.push_back(dPoint(x,l));
          cr.push_back(dPoint(x-l,0));
        }
        continue;
      }

      // Skip crossing at the end point.
      if (se[k].y == y){
        continue;
      }

      // Crossing in some other point of the segment.
      cr.push_back(dPoint((ss[k] * double(y - sb[k].y)) + sb[k].x, 0));

    }

    // Sort crossings.
    sort(cr.begin(), cr.end());
    return cr;
  }

  // Use the crossing array to check if a point is inside the polygon
  // by calculating number of crossings on the ray (x,y) - (inf,y)
  static bool test_cr(const std::vector<dPoint> & cr, CT x, bool borders = true) {

    // first crossing on the right of the point
    auto i = lower_bound(cr.begin(), cr.end(), dPoint(x,0));

    if (i == cr.end()) return false;

    // point is in the crossing
    if (i->x == x || i->x-i->y <= x) return borders;

    int k=0;
    while (i!=cr.end()) { i++; k++; }
    return k%2==1;
  }

  bool test_pt(const PT & P, const bool borders = true){
    return PolyTester<CT,PT>::test_cr(get_cr(P.y), P.x, borders);
  }

};
typedef PolyTester<double,dPoint> dPolyTester;
typedef PolyTester<int,iPoint>    iPolyTester;

/**********************************************************/

/// Check if one-segment polygon L covers point P.
template <typename CT, typename PT>
bool
point_in_polygon(const PT & P, const Line<CT,PT> & L, const bool borders = true){
  PolyTester<CT,PT> lt(L, true);
  return lt.test_pt(P, borders);
}

// Same for multi-segment polygon
template <typename CT, typename PT>
bool
point_in_polygon(const PT & P, const MultiLine<CT,PT> & L, const bool borders = true){
  PolyTester<CT,PT> lt(L, true);
  return lt.test_pt(P, borders);
}


/// Check if one-segment polygon l covers (maybe partially) rectangle r.
/// Return:
///  0 - polygon and rectange are not crossing
///  1 - border of the polygon is crossing/touching rectangle boundary,
///  2 - rectangle is fully inside the polygon,
///  3 - polygon is fully inside the rectangle
template <typename CT, typename PT>
int
rect_in_polygon(const Rect<CT> & R, const Line<CT,PT> & L){

  if (!R) return 0;

  std::vector<dPoint> cr;
  PolyTester<CT,PT> lth(L, true), ltv(L,false);

  // Check is there any crossing at any rectangle side.
  // (segments (R.x,R,x+R.w) and (x, x-l) are crossing)
  cr = lth.get_cr(R.y);
  for (auto const & c:cr){
    auto x1 = c.x,  x2 = x1 - c.y;
    if ((x1 >= R.x || x2 >= R.x) && (x1 <= R.x+R.w || x2 <= R.x+R.w)) return 1;
  }
  cr = lth.get_cr(R.y+R.h);
  for (auto const & c:cr){
    auto x1 = c.x,  x2 = x1 - c.y;
    if ((x1 >= R.x || x2 >= R.x) && (x1 <= R.x+R.w || x2 <= R.x+R.w)) return 1;
  }

  cr = ltv.get_cr(R.x);
  for (auto const & c:cr){
    auto y1 = c.x,  y2 = y1 - c.y;
    if ((y1 >= R.y || y2 >= R.y) && (y1 <= R.y+R.h || y2 <= R.y+R.h)) return 1;
  }
  cr = ltv.get_cr(R.x+R.w);
  for (auto const & c:cr){
    auto y1 = c.x,  y2 = y1 - c.y;
    if ((y1 >= R.y || y2 >= R.y) && (y1 <= R.y+R.h || y2 <= R.y+R.h)) return 1;
  }

  // one rectangle corner is inside polygon
  if (PolyTester<CT,PT>::test_cr(cr, R.y, false)) return 2;

  // one of polygon points is inside the rectangle
  if (L.size() && R.contains_l(L[0])) return 3;
  return 0;
}


/// Same for multi-segment polygons.
template <typename CT, typename PT>
int
rect_in_polygon(const Rect<CT> & R, const MultiLine<CT,PT> & L){

  if (!R) return 0;
  int loops = 0;
  for (auto const & l: L) {
    int r = rect_in_polygon(R, l);
    if (r == 1) return 1; // there is a crossing
    if (r == 2) loops++;  // rectangle is fully in a loop
    if (r == 3 && loops==0) return 3; //
  }
  return (loops%2)? 2:0;
}


// Join a multi-segment polygon into a single-segment one
// using shortest cuts.
template<typename CT, typename PT>
Line<CT,PT> join_polygons(const MultiLine<CT,PT> & L){

  Line<CT,PT> ret;
  if (L.size()==0) return ret;

  typename MultiLine<CT,PT>::const_iterator l = L.begin();
  ret = *l; l++;
  while (l!=L.end()){

    // Find place for the shortest cut between ret vertex
    // and vertex of the next segment

    double dst = INFINITY;

    typename Line<CT,PT>::const_iterator i1,i2; // vertex iterators
    typename Line<CT,PT>::const_iterator q1,q2; // result

    for (i1=ret.begin(); i1!=ret.end(); i1++){
      for (i2=l->begin(); i2!=l->end(); i2++){
        double d = dist(*i1, *i2);
        if (d < dst){ dst = d; q1=i1; q2=i2; }
      }
    }

    // insert new segment
    Line<CT,PT> tmp;
    tmp.push_back(*q1);
    tmp.insert(tmp.end(), q2, l->end());
    tmp.insert(tmp.end(), l->begin(), q2);
    tmp.push_back(*q2);
    ret.insert(q1, tmp.begin(), tmp.end());
    l++;
  }
  return ret;
}

// Remove holes in a multi-segment polygon
// using shortest cuts.
template<typename CT, typename PT>
void remove_holes(MultiLine<CT,PT> & L){
  typename MultiLine<CT,PT>::iterator i1,i2;
  for (i1=L.begin(); i1!=L.end(); i1++){
    i2=i1+1;
    while (i2!=L.end()){
      if ( !i2->size() || !point_in_polygon((*i2)[0], *i1, false)){
        i2++; continue; }
      // join i2 -> i1

      // Find place for the shortest cut between ret vertex
      // and vertex of the next segment
      double dst = 1e99;
      typename Line<CT,PT>::iterator p1,q1;
      typename Line<CT,PT>::iterator p2,q2;
        // p1,p2 -- пара вершин
        // q1,q2 -- искомое
      for (p1=i1->begin(); p1!=i1->end(); p1++){
        for (p2=i2->begin(); p2!=i2->end(); p2++){
          double d = dist(*p1, *p2);
          if (d < dst){ dst = d; q1=p1; q2=p2; }
        }
      }
      // insert new segment
      Line<CT,PT> tmp;
      tmp.push_back(*q1);
      tmp.insert(tmp.end(), q2, i2->end());
      tmp.insert(tmp.end(), i2->begin(), q2);
      tmp.push_back(*q2);
      (*i1).insert(q1, tmp.begin(), tmp.end());
      i2=L.erase(i2);
    }
  }
}

/// Read a figure from the string.
/// The figure can be Point, Line/Multiline, Rect.
/// Return the figure as MultiLine.
template <typename CT>
MultiLine<CT,Point<CT> > figure_line(const::std::string &str) {
  MultiLine<CT,Point<CT> > ret;
  if (str=="") return ret;

  // try point
  try {
    Line<CT,Point<CT> > l;
    l.push_back(Point<CT>(str));
    ret.push_back(l);
    return ret;
  }
  catch (Err & e){}

  // try Rect
  try {
    ret.push_back(rect_to_line(Rect<CT>(str), true));
    return ret;
  }
  catch (Err & e){}

  // try Line/Multiline
  try {
    MultiLine<CT,Point<CT> > ml(str);
    return ml;
  }
  catch (Err & e){}
  throw Err() << "can't read figure: " << str;
}

/// Read a figure from the string and get its bounding box.
/// The figure can be Point, Line/Multiline, Rect
template <typename CT>
Rect<CT> figure_bbox(const::std::string &str) {
  return figure_line<CT>(str).bbox();
}

/****************************************************/
/// Find distance to the nearest vertex of a Line.
/// Arguments:
///   l -- line
///   pt -- point
///   ptp -- if the pointer is not NULL then the vertex point will be stored there
///   dist_func -- function for measuring distances (default dist_2d)
/// Return value:
///   distance from point pt to the nearest vertex of line l
///
template <typename CT, typename PT>
double
nearest_vertex(const Line<CT,PT> & l, const PT & pt, PT * ptp=NULL,
               double (*dist_func)(const PT &, const PT &) = NULL){

  double d = INFINITY;
  for (const auto & p:l){
    double d1 = dist_func? dist_func(p,pt) : dist2d(p,pt);
    if (d1>=d) continue;
    d = d1;
    if (ptp) *ptp=p;
  }
  if (std::isinf(d)) throw Err() << "Can't find nearest point: empty line";
  return d;
}

/// Same for MultiLine
template <typename CT, typename PT>
double
nearest_vertex(const MultiLine<CT,PT> & ml, const PT & pt, PT * ptp=NULL,
               double (*dist_func)(const PT &, const PT &) = NULL){
  double d =  INFINITY;
  for (const auto & l:ml){
    for (const auto & p:l){
      double d1 = dist_func? dist_func(p,pt) : dist2d(p,pt);
      if (d1>=d) continue;
      d = d1;
      if (ptp) *ptp=p;
    }
  }
  if (std::isinf(d)) throw Err() << "Can't find nearest point: empty line";
  return d;
}

/****************************************************/

///  For point p0 find nearest point on the segment p1,p2
template<typename PT>
dPoint nearest_pt(const PT & p0, const PT & p1, const PT & p2, bool use2d = true){
  if (p1==p2) return p1;
  double ll = use2d ? dist2d(p1,p2) : dist(p1,p2);
  dPoint v = dPoint(p2-p1)/ll;

  double prl = use2d ? pscal2d(dPoint(p0-p1), v): pscal(dPoint(p0-p1), v);
  if ((prl>=0)&&(prl<=ll))  // point is inside a segment
    return (dPoint)p1 + v*prl;

  double d1 = use2d ? dist2d(p0,p1) : dist(p0,p1);
  double d2 = use2d ? dist2d(p0,p2) : dist(p0,p2);
  return d1<d2 ? p1:p2;
}

/*
 Find minimum distance between a line and a point <pt>.

 If the distance is less then <maxdist> then the distance is returned,
 the nearest point in the line is returned in <pt> and direction in <vec>.

 If the distance is larger then <maxdist> then <maxdist> is returned and
 <pt> and <vec> are not modified.
*/
template<typename CT, typename PT>
double nearest_pt(const Line<CT,PT> & line, dPoint & vec, Point<CT> & pt, double maxdist, bool use2d = true){

  Point<CT> pm = pt;

  for (size_t j=1; j<line.size(); j++){
    auto p1 = line[j-1];
    auto p2 = line[j];
    if (p1==p2) continue;

    auto pc = nearest_pt(pt,p1,p2, use2d);
    auto lc = use2d ? dist2d(pt,pc) : dist(pt,pc);
    auto d12 = use2d ? dist2d(p1,p2) : dist(p1,p2);
    if (lc<maxdist) { maxdist=lc; pm=pc; vec = dPoint(p2-p1)/d12; }
  }
  pt=pm;
  return maxdist;
}

// same for MultiLine
template<typename CT, typename PT>
double nearest_pt(const MultiLine<CT,PT> & lines, dPoint & vec, Point<CT> & pt, double maxdist, bool use2d = true){
  dPoint pm=pt;
  for (const auto & line:lines){
    auto p = pt;
    maxdist = nearest_pt(line, vec, p, maxdist, use2d);
    if ( p != pt) { pm = p;}
  }
  pt=pm;
  return maxdist;
}

/****************************************************/
// Check if two segments are crossing (2D),
// return crossing point in cr (even if segments are not crossing).
// Both ends included into segments: this may produce
// duplication of crossing points (if line just touches another line),
// but should always produce even number of crossings for closed lines.
// Segments on any parallel lines (even same) are not crossing.
// No crossings for zero-length segments.

template<typename PT>
bool segment_cross_2d(const PT & p1, const PT & p2,
                      const PT & q1, const PT & q2,
                      dPoint & cr){

  // line through points (x1,y1) and (x2,y2):
  // y*(x2-x1) = (y2-y1)*x + (y1*x2 - y2*x1)

  // Find crossing of two lines (x0,y0): AX*x0 = BX, AY*y0 = BY
  double AX = (p2.y-p1.y)*(q2.x-q1.x) - (q2.y-q1.y)*(p2.x-p1.x);
  double AY = (q2.y-q1.y)*(p2.x-p1.x) - (p2.y-p1.y)*(q2.x-q1.x);
  double BX = (q1.y*q2.x - q2.y*q1.x)*(p2.x-p1.x) - (p1.y*p2.x - p2.y*p1.x)*(q2.x-q1.x);
  double BY = (p1.y*p2.x - p2.y*p1.x)*(q2.y-q1.y) - (q1.y*q2.x - q2.y*q1.x)*(p2.y-p1.y);

  cr = dPoint(BX/AX, BY/AY);
  // note that coordinates can be inf or nan!
  if (cr.x != cr.x || cr.y != cr.y) return false;

  double dp = dist2d(p1,p2);
  double dq = dist2d(q1,q2);

  if (dist2d(cr,p1) > dp || dist2d(cr,p2) > dp ||
      dist2d(cr,q1) > dq || dist2d(cr,q2) > dq) return false;

  return true;
}

/****************************************************/
// Find all crossings of two closed lines
struct cross_t {
  dPoint pt; // crossing point
  size_t i1, i2; // indices of line1 and line2 segments

  cross_t(const dPoint & pt, const size_t i1, const size_t & i2):
      pt(pt), i1(i1), i2(i2) {}
};


template<typename CT, typename PT>
std::list<cross_t>
poly_cross_2d(const Line<CT,PT> & line1, const Line<CT,PT> & line2){
  std::list<cross_t> ret;

  size_t di1 = 1;
  size_t N1 = line1.size();
  size_t N2 = line2.size();
  // exact crossings (lineA exact index -> lineB index)
  std::map<size_t, size_t> ecr1, ecr2;

  for (size_t i1b = 0; i1b < N1; i1b+=di1){

    // TODO: it should be a separate normalization function:
    // remove self crossings and empty segments

    size_t i1e = i1b+1<N1 ? i1b+1 : 0;
    // skip zero-length segments
    while (i1e!=i1b && line1[i1b] == line1[i1e])
      i1e = i1e+1<N1 ? i1e+1 : 0;
    // skip zero-length segment at the end
    if (i1e+1 == N1 && line1[0] == line1[i1e])
      i1e=0;
    di1=(i1e+N1-i1b)%N1;

    size_t di2 = 1;
    for (size_t i2b = 0; i2b < N2; i2b+=di2){

      size_t i2e = i2b+1<N2 ? i2b+1 : 0;
      // skip zero-length segments
      while (i2e!=i2b && line2[i2b] == line2[i2e])
        i2e = i2e+1<N2 ? i2e+1 : 0;
      // skip zero-length segment at the end
      if (i2e+1 == N2 && line2[0] == line2[i2e])
        i2e=0;
      di2=(i2e+N2-i2b)%N2;

      dPoint cr;
      if (!segment_cross_2d(line1[i1b], line1[i1e],
                            line2[i2b], line2[i2e], cr)) continue;

      auto i1r = (cr==line1[i1e]) ? i1e:i1b;
      auto i2r = (cr==line2[i2e]) ? i2e:i2b;
      // skip duplicated exact crossings
      if (cr == line1[i1b]){
        if (ecr1.count(i1b) && i2r == ecr1[i1b]) continue;
        ecr1.emplace(i1b,i2r);
      }
      if (cr == line1[i1e]){
        if (ecr1.count(i1e) && i2r == ecr1[i1e]) continue;
        ecr1.emplace(i1e,i2r);
      }
      if (cr == line2[i2b]){
        if (ecr2.count(i2b) && i1r == ecr2[i2b]) continue;
        ecr2.emplace(i2b,i1r);
      }
      if (cr == line2[i2e]){
        if (ecr2.count(i2e) && i1r == ecr2[i2e]) continue;
        ecr2.emplace(i2e,i1r);
      }
      ret.emplace_back(cr, i1b, i2b);
    }
  }
  return ret;
}

/****************************************************/


/// Filter out some points (up to number np, or distance from original line e)
template<typename CT, typename PT>
void line_filter_v1(Line<CT,PT> & line, double e, int np,
                    double (*dist_func)(const PT &, const PT &) = NULL){
  while (1) {
    // Calculate distance from each non-end point to line between its neighbours.
    // Find minimum of this value.
    double min=-1;
    size_t mini = 0; // index of point with minimal deviation, or 0
    for (size_t i=1; i+1<line.size(); i++){
      dPoint p1 = line[i-1];
      dPoint p2 = line[i];
      dPoint p3 = line[i+1];
      auto pc = nearest_pt(p2,p1,p3);
      double dp = dist_func? dist_func(p2,pc) : dist(p2,pc);
      if ((min<0) || (min>dp)) {min = dp; mini=i;}
    }
    if (mini == 0) break;
    // skip point if needed
    if ( ((e>0) && (min<e)) ||
         ((np>0) && (line.size()>np))) line.erase(line.begin()+mini);
    else break;
  }
}

// Same for MultiLine. Remove also segments shorter then e.
template<typename CT, typename PT>
void line_filter_v1(MultiLine<CT,PT> & lines, double e, int np,
                    double (*dist_func)(const PT &, const PT &) = NULL){
  for (auto l = lines.begin(); l!=lines.end(); l++){
    line_filter_v1(*l, e, np, dist_func);
    // remove 2-point lines shorter then e
    if (l->size() != 2 || e<=0) continue;
    auto p1 = (*l)[0];
    auto p2 = (*l)[1];
    double d = dist_func? dist_func(p1,p2) : dist(p1,p2);
    if (d<e) lines.erase(l--);
  }
}


/// Found bounding convex polygon for points.
/// Resulting polygon start from point with minimal x,
/// it is always clockwise-oriented, its last point =
/// first point
template <typename CT, typename PT>
Line<CT,PT> convex_border(const Line<CT,PT> & points){
  Line<CT,PT> ret;
  if (points.size()==0) return ret;
  typename Line<CT,PT>::const_iterator p, p0, p1;

  // find point with minimal x
  p0=points.begin();
  for (p=points.begin(); p!=points.end(); p++){
    if (p0->x > p->x) p0=p;
  }

  dPoint v0(0,1);

  do {
    ret.push_back(*p0);
    p1=p0;
    p1++; if (p1==points.end()) p1=points.begin();
    if (p0==p1) break;

    // find point with maximal v*norm(p-p0)
    double cmax=-1;
    for (p=points.begin(); p!=points.end(); p++){
      if (*p == *p0) continue;
      dPoint v = norm2d(*p - *p0);
      double c = pscal2d(v0, v);
      if (cmax < c) { cmax=c; p1=p;}
    }
    v0=norm(*p1 - *p0);
    p0=p1;
    assert (ret.size() <= points.size());
  } while (*p1!=*ret.begin());
  return ret;
}

// test_pairs helper function for margin_classifier().
// For each pair of points (p1,p2) from oriented contour l1
// find the shortest distance from the line p1-p2 to points in l2.
// Then find the pair with maximum value (but less then maxdist).
// Distance is negative inside l1 and positive outside it.
// The points and distance are returned in maxdist, p1,p2.
template <typename CT, typename PT>
void
test_pairs(const Line<CT,PT> & l1, const Line<CT,PT> & l2,
           double & maxdist, PT & p1, PT & p2){
  typename Line<CT,PT>::const_iterator i,j;
  for (i=l1.begin(); i!=l1.end(); i++){
    j=i+1; if (j==l1.end()) j=l1.begin();
    if (*i==*j) continue;
    double mindist=INFINITY;
    typename Line<CT,PT>::const_iterator k;
    for (k=l2.begin(); k!=l2.end(); k++){
      dPoint v1(*j-*i);
      dPoint v2(*k-*i);
      double K = pscal2d(v1,v2)/pscal2d(v1,v1);
      double d = len(K*v1-v2);
      if ((v1.x*v2.y-v2.x*v1.y)<0) d*=-1;
      if (d<mindist) mindist=d;
    }
    if (maxdist < mindist){
      maxdist = mindist;
      p1=*i; p2=*j;
    }
  }
}


/*
 Margin Classifier --
 Find line which separates two point sets L1 and L2 with maximal margin.
 Returns margin value (can be < 0).
 p0 is set to line origin, and t is set to line direction.
*/
template <typename CT, typename PT>
double
margin_classifier(const Line<CT,PT> & L1, const Line<CT,PT> & L2,
  dPoint & p0, dPoint & t){

  // find borders
  Line<CT,PT> l1=convex_border(L1);
  Line<CT,PT> l2=convex_border(L2);

  // Support vectors (http://en.wikipedia.org/wiki/Support_vector_machine)
  // are located on a convex boundary of a point set. At least in one set
  // there are two of them and they are neighbours in the boundary.

  // For each pair we apply test_pairs() function.
  // Function convex_border() gives clockwise-oriented boundaries,
  // distances are positive outside them.

  double maxdist=-INFINITY;
  PT p1,p2;
  test_pairs(l1,l2,maxdist,p1,p2);
  test_pairs(l2,l1,maxdist,p1,p2);

  // We have found two support vectors: p1 and p2 in one of the boundaries and
  // the distance between sets (positive or negative).

  // Now move the line p1-p2 by maxdist/2.

  t = norm(p2-p1);
  dPoint n(-t.y, t.x);
  p0 = dPoint(p1) + n*maxdist/2.0;

  return maxdist;
}

// Find area of a polygon (sign shows orientation).
template <typename CT, typename PT>
double line_area(const Line<CT,PT> & line){
  double ret = 0.0;
  for (size_t i = 0; i < line.size(); i++){
    auto p1 = line[i];
    auto p2 = i>0? line[i-1]: line[line.size()-1];
    ret += (p1.x - p2.x)*(p1.y + p2.y)/2.0;
  }
  return ret;
}


typedef enum {
  POLY_ADD,
  POLY_SUB
} poly_op_t;


// Crop single-segment polygons. Line1 is cropped by line2.
template <typename CT, typename PT>
MultiLine<CT,PT> poly_op(const Line<CT,PT> & line1, const Line<CT,PT> & line2, const poly_op_t & op){

  // line crossing
  struct cross_t {
    dPoint pt; // crossing point
    size_t i1b,i1e; // indices of line1 segment
    size_t i2b,i2e; // indices of line2 segment
    double dist1; // distance from beginning of line1 segment
    double dist2; // distance from beginning of line2 segment
    cross_t *i1n, *i2n; // next crossing along line1 and line2, NULL if unknown
    cross_t *i1p, *i2p; // previous crossing along line1 and line2, NULL if unknown

    cross_t(const dPoint & pt, const size_t i1b, const size_t & i1e,
            const size_t i2b, const size_t & i2e,
            const double dist1, const double dist2):
      pt(pt), i1b(i1b), i1e(i1e), i2b(i2b), i2e(i2e),
      dist1(dist1), dist2(dist2),
      i1n(NULL), i2n(NULL), i1p(NULL), i2p(NULL) {}
  };

  // Find all crossings
  std::vector<cross_t> cross;
  for (size_t i1b = 0; i1b < line1.size(); i1b++){
    size_t i1e = i1b+1<line1.size() ? i1b+1 : 0;

    for (size_t i2b = 0; i2b < line2.size(); i2b++){
      size_t i2e = i2b+1<line2.size() ? i2b+1 : 0;
      dPoint cr;
      bool res = segment_cross_2d(
        line1[i1b], line1[i1e], line2[i2b], line2[i2e], cr);

      if (res) {
        cross.emplace_back(cr, i1b, i1e, i2b, i2e,
          dist2d(line1[i1b],cr), dist2d(line2[i2b],cr));
std::cerr << "  cr: " << i1b << " " << i1e << " " << i2b << " " << i2e << " " << cr << "\n";
      }
    }
  }
std::cerr << "line1: " << line1 << "\n";
std::cerr << "line2: " << line2 << "\n";
std::cerr << "crossings: " << cross.size() << "\n";

  // Fill i*n, i*p fields
  size_t N = cross.size();
  if (N>0){
    std::vector<cross_t *> cross_s(N);
    for (size_t i = 0; i < N; i++) cross_s[i] = &cross[i];

    // Sorted crossings along line1
    struct {
      bool operator()(const cross_t * a, const cross_t * b) const {
        return a->i1b < b->i1b || (a->i1b == b->i1b && a->dist1 < b->dist1); }
    } cross_cmp1;
    std::sort(cross_s.begin(), cross_s.end(), cross_cmp1);

    for (size_t i = 0; i<N; i++){
      cross_s[i]->i1n = cross_s[i+1<N? i+1:0];
      cross_s[i]->i1p = cross_s[i>0? i-1:N-1];
    }

    // Same for line2
    struct {
      bool operator()(const cross_t * a, const cross_t * b) const {
        return a->i2b < b->i2b || (a->i2b == b->i2b && a->dist2 < b->dist2); }
    } cross_cmp2;
    std::sort(cross_s.begin(), cross_s.end(), cross_cmp2);

    for (size_t i = 0; i<N; i++){
      cross_s[i]->i2n = cross_s[i+1<N? i+1:0];
      cross_s[i]->i2p = cross_s[i>0? i-1:N-1];
    }
  }

std::cerr << "sort crossings\n";

  // we need to test are points inside lines or not
  PolyTester<CT,PT> pt1(line1), pt2(line2);

  // Now we can calculate result line!
  MultiLine<CT,PT> ret;

  // no crossing case
  if (N==0) {
    if (op == POLY_ADD){
      if (line1.size() == 0) {ret.push_back(line2); return ret;}
      if (line2.size() == 0) {ret.push_back(line1); return ret;}
      // one can imaging all points except a few are coinside
      // if at least one is inside, return another line
      for (size_t i = 0; i < line1.size(); i++)
        if (pt2.test_pt(line1[i])) {ret.push_back(line2); return ret;}
      for (size_t i = 0; i < line2.size(); i++)
        if (pt1.test_pt(line2[i])) {ret.push_back(line1); return ret;}
      ret.push_back(line1); return ret;
    }
    else if (op == POLY_SUB){
      if (line1.size() == 0) return ret;
      if (line2.size() == 0) {ret.push_back(line1); return ret;}
      // non-empty difference only if line2 is inside line1
      for (size_t i = 0; i < line2.size(); i++)
        if (pt1.test_pt(line2[i])) {ret.push_back(line1); ret.push_back(line2); return ret;}
      return ret;
    }
    else throw Err() << "Unknown operation";
  }

//  double a1 = line_area(line1);
//  double a2 = line_area(line2);
//  std::cerr << "areas: " << a1 << "  " << a2 << "\n";

  // we have at least one crossing. Let's start from this point
  std::set<cross_t *> todo;
  for (size_t i = 0; i < N; i++) todo.insert(&cross[i]);
  while (todo.size()>0){
    auto c0 = *todo.begin();
    todo.erase(todo.begin());
std::cerr << "start loop\n";

    // We want to do a full loop starting from the crossing.
    // On each crossing we switch to another line, choosing direction
    // which goes inside or ouside of previous line, depending on
    // op parameter.

    bool l1 = false; // previous line is line1?
    Line<CT,PT> loop; // constructed loop
    cross_t *c1 = c0;
    cross_t *c2;
    while (1){
      loop.push_back(c1->pt);
std::cerr << "  pt: " << c1->pt << "\n";
      if (l1) { // switching line1 -> line2
        cross_t *c2n = c1->i2n;
        cross_t *c2p = c1->i2p;
        if (!c2n || !c2p) throw Err() <<
          "NULL crossing address"; // should not happen
        // We should check if segment goes outside or inside previous line.
        // For this we choose reference point: start of next crossing segment
        // if crossings are on different segments, center between crossings otherwise.
        auto pref = line2[c2n->i2b];
        if (c1->i2b == c2n->i2b)
          pref += norm(line2[c2n->i2e] - line2[c2n->i2b]) * (c1->dist2 + c2n->dist2)/2.0;
        c2 = (op == POLY_ADD) == pt1.test_pt(pref) ? c2n : c2p;
        // Go from c1 to c2 along line2:
std::cerr << "  l2: " << c1->i2e << " " << c2->i2e <<  "\n";
        for (size_t i = c1->i2e; i!=c2->i2e; i = i<line2.size()-1 ? i+1:0) loop.push_back(line2[i]);
      }
      else { // same for switching line2 -> line1
        cross_t *c2n = c1->i1n;
        cross_t *c2p = c1->i1p;
        if (!c2n || !c2p) throw Err() <<
          "NULL crossing address"; // should not happen
        // We should check if segment goes outside or inside previous line.
        // For this we choose reference point: start of next crossing segment
        // if crossings are on different segments, center between crossings otherwise.
        auto pref = line1[c2n->i1b];
        if (c1->i1b == c2n->i1b)
          pref += (line1[c2n->i1e] - line1[c2n->i1b]) * (c1->dist1 + c2n->dist1)/2.0;
        c2 = (op == POLY_ADD) == pt2.test_pt(pref) ? c2n : c2p;
        // Go from c1 to c2 along line1:
std::cerr << "  l1: " << c1->i1e << " " << c2->i1e <<  "\n";
        for (size_t i = c1->i1e; i!=c2->i1e; i = i<line1.size()-1 ? i+1:0) loop.push_back(line1[i]);
      }
      if (c2 == c0) break; // end of loop
      auto c2i = todo.find(c2);
      if (c2i!=todo.end()) todo.erase(c2i);
      c1 = c2;
      l1 = !l1;
    }
std::cerr << "loop done\n";
    ret.push_back(loop);
  }

  return ret;
}

// Same for multilines.
template <typename CT, typename PT>
MultiLine<CT,PT> poly_op(const MultiLine<CT,PT> & ml1, const MultiLine<CT,PT> & ml2, const poly_op_t & op){
  // subtract
  MultiLine<CT,PT> ret;
  if (op == POLY_SUB) {
    for (const auto & l1:ml1){
      for (const auto & l2:ml2){
        auto l = poly_op(l1,l2, POLY_SUB);
        ret.insert(ret.end(), l.begin(), l.end());
      }
    }
  }
  else {
/*    // join both multilines
    ret.insert(ret.end(), ml1.begin(), ml1.end());
    ret.insert(ret.end(), ml2.begin(), ml2.end());
    size_t ns = 0;
    while (ns!=ret.size()){
      // join 
      auto i1 = ret.begin(), i2 = ret.begin();

      for (const i)
      for (const auto & l1:ml1){
        for (const auto & l2:ml2){
     
        }
      }
    }

    for (const auto & l1:ml1){
      ret = poly_op(ret,l2, POLY_ADD);
    }
*/
  }
}

// Check if pts2 is a hole inside pts1:
// all points of pts2 are inside pts1.
// all points of pts1 are outside pts2,
// This is not a very good definition of a hole,
// but should be OK for real maps.
template <typename CT, typename PT>
bool
check_hole(const Line<CT,PT> & pts1, const Line<CT,PT> & pts2){
  PolyTester<CT,PT> pt1(pts1);
  for (const auto & p2:pts2){
    if (!pt1.test_pt(p2, true)) return false;
  }
  PolyTester<CT,PT> pt2(pts2);
  for (const auto & p1:pts1){
    if (pt2.test_pt(p1, false)) return false;
  }
  return true;
}


#endif
