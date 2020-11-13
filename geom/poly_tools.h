#ifndef POLY_TOOLS_H
#define POLY_TOOLS_H

#include "multiline.h"
#include <algorithm>

/* Extra polygon-related functions */

/// Class for checking if a point is inside a polygon.
/// Line is always treated as closed.
template <typename T>
class PolyTester{
  std::vector<Point<T> > sb,se; // segment beginning, ending
  std::vector<double> ss; // segment slope
  bool horiz; // test direction
public:

  // Constructor, build the tester class for a given polygon.
  // Parameters:
  //  - L -- polygon (represented by Line object)
  //  - horiz -- set test direction
  PolyTester(const Line<T> & L, const bool horiz_ = true): horiz(horiz_){

    // Collect line sides info: start and end points, slopes
    int pts = L.size();
    for (int i = 0; i < pts; i++){
      Point<T> b(L[i%pts].x, L[i%pts].y),
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

  // Get sorted coordinates of the polygon crossings with
  // y=const (or x=const if horiz=false) line.
  // Returns sorted dPoint array where first point coordinates
  // are crossing positions, second are "crossing lengths" (normally
  // zero, non-zero if segment coinsides with y=const line.
  // Length is calculated to the left of the point.
  std::vector<dPoint> get_cr(T y){
    std::vector<dPoint> cr;
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
  static bool test_cr(const std::vector<dPoint> & cr, T x, bool borders = true) {

    // first crossing on the right of the point
    auto i = lower_bound(cr.begin(), cr.end(), dPoint(x,0));

    if (i == cr.end()) return false;

    // point is in the crossing
    if (i->x == x || i->x-i->y <= x) return borders;

    int k=0;
    while (i!=cr.end()) { i++; k++; }
    return k%2==1;
  }

};
typedef PolyTester<double> dPolyTester;
typedef PolyTester<int>    iPolyTester;

/**********************************************************/

/// Check if one-segment polygon L covers point P.
template <typename T>
bool
point_in_polygon(const Point<T> & P, const Line<T> & L, const bool borders = true){
  PolyTester<T> lt(L, true);
  auto cr = lt.get_cr(P.y);
  return PolyTester<T>::test_cr(cr, P.x, borders);
}

// Same for multi-segment polygon
template <typename T>
bool
point_in_polygon(const Point<T> & P, const MultiLine<T> & L, const bool borders = true){
  // merge and sort crossings for all multiline segments:
  std::vector<dPoint> cr;
  for (const auto seg:L) {
    PolyTester<T> lt(seg, true);
    auto c = lt.get_cr(P.y);
    cr.insert(cr.end(), c.begin(), c.end());
  }
  sort(cr.begin(), cr.end());
  return PolyTester<T>::test_cr(cr, P.x, borders);
}


/// Check if one-segment polygon l covers (maybe partially) rectangle r.
/// Return:
///  0 - polygon and rectange are not crossing
///  1 - border of the polygon is crossing/touching rectangle boundary,
///  2 - rectangle is fully inside the polygon,
///  3 - polygon is fully inside the rectangle
template <typename T>
int
rect_in_polygon(const Rect<T> & R, const Line<T> & L){

  if (!R) return 0;

  std::vector<dPoint> cr;
  PolyTester<T> lth(L, true), ltv(L,false);

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
  if (PolyTester<T>::test_cr(cr, R.y, false)) return 2;

  // one of polygon points is inside the rectangle
  if (L.size() && R.contains_l(L[0])) return 3;
  return 0;
}


/// Same for multi-segment polygons.
template <typename T>
int
rect_in_polygon(const Rect<T> & R, const MultiLine<T> & L){

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
template<typename T>
Line<T> join_polygons(const MultiLine<T> & L){

  Line<T> ret;
  if (L.size()==0) return ret;

  typename MultiLine<T>::const_iterator l = L.begin();
  ret = *l; l++;
  while (l!=L.end()){

    // Find place for the shortest cut between ret vertex
    // and vertex of the next segment

    double dst = INFINITY;

    typename Line<T>::const_iterator i1,i2; // vertex iterators
    typename Line<T>::const_iterator q1,q2; // result

    for (i1=ret.begin(); i1!=ret.end(); i1++){
      for (i2=l->begin(); i2!=l->end(); i2++){
        double d = dist(*i1, *i2);
        if (d < dst){ dst = d; q1=i1; q2=i2; }
      }
    }

    // insert new segment
    Line<T> tmp;
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
template<typename T>
void remove_holes(MultiLine<T> & L){
  typename MultiLine<T>::iterator i1,i2;
  for (i1=L.begin(); i1!=L.end(); i1++){
    i2=i1+1;
    while (i2!=L.end()){
      if ( !i2->size() || !point_in_polygon((*i2)[0], *i1, false)){
        i2++; continue; }
      // join i2 -> i1

      // Find place for the shortest cut between ret vertex
      // and vertex of the next segment
      double dst = 1e99;
      typename Line<T>::iterator p1,q1;
      typename Line<T>::iterator p2,q2;
        // p1,p2 -- пара вершин
        // q1,q2 -- искомое
      for (p1=i1->begin(); p1!=i1->end(); p1++){
        for (p2=i2->begin(); p2!=i2->end(); p2++){
          double d = dist(*p1, *p2);
          if (d < dst){ dst = d; q1=p1; q2=p2; }
        }
      }
      // insert new segment
      Line<T> tmp;
      tmp.push_back(*q1);
      tmp.insert(tmp.end(), q2, i2->end());
      tmp.insert(tmp.end(), i2->begin(), q2);
      tmp.push_back(*q2);
      (*i1).insert(q1, tmp.begin(), tmp.end());
      i2=L.erase(i2);
    }
  }
}

/// Read a figure from the string and get its bounding box.
/// The figure can be Point, Line/Multiline, Rect
template <typename T>
MultiLine<T> figure_line(const::std::string &str) {
  MultiLine<T> ret;
  if (str=="") return ret;

  // try point
  try {
    Line<T> l;
    l.push_back(Point<T>(str));
    ret.push_back(l);
    return ret;
  }
  catch (Err & e){}

  // try Rect
  try {
    ret.push_back(rect_to_line(Rect<T>(str), true));
    return ret;
  }
  catch (Err & e){}

  // try Line/Multiline
  try {
    MultiLine<T> ml(str);
    return ml;
  }
  catch (Err & e){}
  throw Err() << "can't read figure: " << str;
}

/// Read a figure from the string and get its bounding box.
/// The figure can be Point, Line/Multiline, Rect
template <typename T>
Rect<T> figure_bbox(const::std::string &str) {
  return figure_line<T>(str).bbox();
}


#endif
