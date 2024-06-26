#ifndef RECT_H
#define RECT_H

#include <iostream>
#include <limits>
#include <iomanip>
#include <ios>
#include "point.h"
#include "err/err.h"

///\addtogroup libmapsoft
///@{

/** 2-d rectangle.
 - Coordinates are counted from top-left.
   Top-left corner has lowest coordinates, bottom-right corner
   has highest coordinates.
 - All corners are included in the rectangle.
 - There is a difference between empty rectangle (such as a
   bounding box of a zero-point line) and zero-size rectangle
   (bounding box of a one-point line).
 - many functions throw error if rectangle is empty
*/
template <typename T>
struct Rect {
  T x; ///< x coordinate
  T y; ///< y coordinate
  T w; ///< width
  T h; ///< height
  bool e; ///< empty flag

  /// Constructor: create empty rectangle
  Rect() : x(0), y(0), w(0), h(0), e(true) { }

  /** Constructor: create non-empty rectangle by two opposite corners.
      You can use any pair of points in any order. */
  Rect(Point<T> p1, Point<T> p2)
    : x(std::min(p1.x, p2.x)), y(std::min(p1.y, p2.y)),
      w(std::max(p1.x,p2.x) - std::min(p1.x,p2.x)),
      h(std::max(p1.y,p2.y) - std::min(p1.y,p2.y)),
      e(false) { }

  /** Constructor: create non-empty rectangle from x,y,w,h values.
      If w (or h) is negative, then x (or y) is right (or bottom) coordinate:
      Rect(0,0,-1,-1) = Rect(-1,-1,1,1) */
  Rect(T _x, T _y, T _w=0, T _h=0)
    : x(_w>=0?_x:_x+_w), y(_h>=0?_y:_y+_h),
      w(_w>=0?_w:-_w), h(_h>=0?_h:-_h),
      e(false) { }

  /// Constructor: create a rectangle string "[]" or "[x,y,w,h]"
  Rect(const std::string & s) { *this = str_to_type<Rect>(s);}

  /// Swap rectangle with other one
  void swap (Rect & other){
    std::swap (x, other.x);
    std::swap (y, other.y);
    std::swap (w, other.w);
    std::swap (h, other.h);
    std::swap (e, other.e);
  }

  /// Test if rectangle is empty
  bool is_empty() const { return e; }

  /// Test if rectangle is empty or has zero size in any dimension
  bool is_zsize() const { return w==0 || h==0 || e; }

  /******************************************************************/
  // corners

  /// Top-left corner
  Point<T> tlc() const {
    if (e) throw Err() << "Empty rectangle in tlc call";
    return  Point<T>(x,y);
  }

  /// Top-right corner
  Point<T> trc() const {
    if (e) throw Err() << "Empty rectangle in trc call";
    return  Point<T>(x+w,y);
  }

  /// Bottom-right corner
  Point<T> brc() const {
    if (e) throw Err() << "Empty rectangle in brc call";
    return  Point<T>(x+w,y+h);
  }

  /// Bottom-left corner
  Point<T> blc() const {
    if (e) throw Err() << "Empty rectangle in blc call";
    return  Point<T>(x,y+h);
  }

  /// Center point
  Point<T> cnt() const {
    if (e) throw Err() << "Empty rectangle in cnt call";
    return  Point<T>(x+w/2,y+h/2);
  }

  /******************************************************************/
  // operators +,-,/,*

  /// Divide all coordinates by k
  Rect<T> & operator/= (T k) {
    if (e) throw Err() << "Empty rectangle in operator/";
    x /= k; y /= k; w /= k; h /= k; return *this; }

  /// Multiply all coordinates by k
  Rect<T> & operator*= (T k) {
    if (e) throw Err() << "Empty rectangle in operator*";
    x *= k; y *= k; w *= k; h *= k; return *this; }

  /// Divide coordinates by p.x, p.y
  Rect<T> & operator/= (Point<T> p) {
    if (e) throw Err() << "Empty rectangle in operator/";
    x /= p.x; y /= p.y; w /= p.x; h /= p.y; return *this; }

  /// Multiply coordinates by p.x, p.y
  Rect<T> & operator*= (Point<T> p) {
    if (e) throw Err() << "Empty rectangle in operator*";
    x *= p.x; y *= p.y; w *= p.x; h *= p.y; return *this; }

  /// Subtract point from the rectangle (shift the rectangle)
  Rect<T> & operator-= (Point<T> p) {
    if (e) throw Err() << "Empty rectangle in operator-";
    x -= p.x; y -= p.y; return *this; }

  /// Add point to the rectangle (shift the rectangle)
  Rect<T> & operator+= (Point<T> p){
    if (e) throw Err() << "Empty rectangle in operator+";
    x += p.x; y += p.y; return *this; }

  /// Divide all coordinates by k
  Rect<T> operator/ (T k) const { Rect<T> ret(*this); return ret/=k; }

  /// Multiply all coordinates by k
  Rect<T> operator* (T k) const { Rect<T> ret(*this); return ret*=k; }

  /// Divide coordinates by p.x, p.y
  Rect<T> operator/ (Point<T> p) const { Rect<T> ret(*this); return ret/=p; }

  /// Multiply coordinates by p.x, p.y
  Rect<T> operator* (Point<T> p) const { Rect<T> ret(*this); return ret*=p; }

  /// Subtract point from the rectangle (shift the rectangle)
  Rect<T> operator- (Point<T> p) const { Rect<T> ret(*this); return ret-=p; }

  /// Add point to the rectangle (shift the rectangle)
  Rect<T> operator+ (Point<T> p) const { Rect<T> ret(*this); return ret+=p; }

  /// operator-: invert
  Rect<T> operator- () const {
    if (e) throw Err() << "Empty rectangle in operator-";
    Rect<T> ret(-this->tlc(), -this->brc()); return ret;
  }

  /******************************************************************/
  // operators <=>

  /// Less-then operator: compare TLC, then BRC.
  /// Empty rectungles are smaller then non-empty ones.
  bool operator< (const Rect<T> & r) const {
      if (r.e) return false;
      if (e) return true;
      if (tlc()!=r.tlc()) return (tlc()<r.tlc());
      return (brc()<r.brc());
  }

  /// Equal operator: Empty rectungles are always equal. 
  bool operator== (const Rect<T> & r) const {
      if (e && r.e) return true;
      if (e || r.e) return false;
      return (x==r.x)&&(y==r.y)&&(w==r.w)&&(h==r.h);
  }

  // derived operators:
  bool operator!= (const Rect<T> & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const Rect<T> & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const Rect<T> & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const Rect<T> & other) const { return !(*this<=other); } ///< operator>

  /******************************************************************/

  /// Cast to Rect<double>
  operator Rect<double>() const{
    if (e) return Rect<double>();
    return Rect<double>(double(this->x), double(this->y),
                        double(this->w), double(this->h));
  }

  /// Cast to Rect<int>
  operator Rect<int>() const{
    if (e) return Rect<int>();
    return Rect<int>(int(this->x), int(this->y),
                     int(this->w), int(this->h));
  }

  /// Cast to bool
  operator bool() const{ return !e; }

  /******************************************************************/
  // Some functions. Below same functions are defined outside the class

  /// rint function: change corner coordenates to nearest integers
  void to_rint() {
    if (e) return;
    Point<T> p1 = rint(tlc());
    Point<T> p3(rint((x-p1.x)+w), rint((y-p1.y)+h));
    *this = p1 + Rect<T>(dPoint(0,0),p3);
  }

  /// floor function: shrink the rectangle to nearest integer coordinate
  /// Rectangle can be shriked to empty one.
  /// We also want to work with large x and y without loosing precision in w and h.
  void to_floor() {
    if (e) return;
    Point<T> p1 = ceil(tlc());
    Point<T> p2 = floor(brc());
    if (p1.x > p2.x || p1.y > p2.y) {*this = Rect(); return;}
    Point<T> p3(floor((x-p1.x)+w), floor((y-p1.y)+h));
    *this = p1 + Rect<T>(dPoint(0,0),p3);
  }

  /// ceil function: expand the rectangle to nearest integer coordinates
  void to_ceil() {
    if (e) return;
    Point<T> p1 = floor(tlc());
    Point<T> p3(ceil((x-p1.x)+w), ceil((y-p1.y)+h));
    *this = p1 + Rect<T>(dPoint(0,0),p3);
  }

  /// Expand rectangle to each side by val value.
  /// If the rectangle is empty throw an error.
  /// If val is negative rectangle can shrink to an empty one.
  void expand (T val) {
    if (e) throw Err() << "Empty rectangle in expand()";
    if (w+2*val<0 || h+2*val<0){
      *this=Rect<T>();
      return;
    }
    *this = Rect<T>(x-val,y-val,w+2*val,h+2*val);
  }

  /// Expand rectangle by vx and vy values.
  /// If the rectangle is empty throw an error.
  /// If val is negative rectangle can shrink to an empty one.
  void expand (T vx, T vy) {
    if (e) throw Err() << "Empty rectangle in expand()";
    if (w+2*vx<0 || h+2*vy<0){
      *this = Rect<T>();
      return;
    }
    *this = Rect<T>(x-vx,y-vy,w+2*vx,h+2*vy);
  }

  /// Expand rectangle to cover point p. Can be used with empty rectangle.
  void expand (const Point<T> & p) {
    if (e) {
      *this = Rect<T>(p,p);
      return;
    }
    T x1 =  std::min (x, p.x);
    T y1 =  std::min (y, p.y);
    T x2 =  std::max (x+w, p.x);
    T y2 =  std::max (y+h, p.y);
    *this = Rect<T>(x1,y1,x2-x1,y2-y1);
  }

  /// Expand rectangle to cover rectangle r. Can be used with empty rectangle.
  void expand (const Rect<T> & r) {
    if (e){ *this = r; return; }
    if (r.e) return;
    T x1 =  std::min (x, r.x);
    T y1 =  std::min (y, r.y);
    T x2 =  std::max (x+w, r.x+r.w);
    T y2 =  std::max (y+h, r.y+r.h);
    *this = Rect<T>(x1,y1,x2-x1,y2-y1);
  }

  /// Calculate intersection with rectangle r. Intersection with
  /// and empty rectangle gives empty result.
  void intersect (const Rect<T> & r) {
    if (e || r.e) { *this = Rect<T>(); return; }
    T x1 =  std::max (x, r.x);
    T y1 =  std::max (y, r.y);
    T x2 =  std::min (x+w, r.x+r.w);
    T y2 =  std::min (y+h, r.y+r.h);
    T w = x2-x1;
    T h = y2-y1;
    if (w<0 || h<0) { *this = Rect<T>(); return; }
    *this = Rect<T>(x1,y1,w,h);
  }

  /// Calculate intersection with rectangle r.
  /// Empty rectangles are ignored.
  void intersect_nonempty (const Rect<T> & r) {
    if (r.e) { return; }
    if (e) { *this = r; return; }
    intersect(r);
  }

  /// If rectangle contains a point (only lower bounds are included).
  bool contains_l (const Point<T> & p) const {
    if (e) return false;
    return (p.x >= x) && (p.x < x+w) &&
           (p.y >= y) && (p.y < y+h);
  }

  /// If rectangle contains a point (only upper bounds are included).
  bool contains_u (const Point<T> & p) const {
    if (e) return false;
    return (p.x > x) && (p.x <= x+w) &&
           (p.y > y) && (p.y <= y+h);
  }

  /// If rectangle contains a point (bounds are not included).
  bool contains_n (const Point<T> & p) const {
    if (e) return false;
    return (p.x > x) && (p.x < x+w) &&
           (p.y > y) && (p.y < y+h);
  }

  /// If rectangle contains a point (all bounds are included).
  bool contains (const Point<T> & p) const {
    if (e) return false;
    return (p.x >= x) && (p.x <= x+w) &&
           (p.y >= y) && (p.y <= y+h);
  }

  /// If rectangle contains another rectangle.
  /// If any of two rectangles is empty false is returned.
  bool contains (const Rect<T> & r) const {
    if (e || r.e) return false;
    return (r.x >= x) && (r.x+r.w <= x+w) &&
           (r.y >= y) && (r.y+r.h <= y+h);
  }

};

/******************************************************************/
// additional operators

/// Multiply coordinates by k (k*rect = rect*k)
/// \relates Rect
template <typename T>
Rect<T> operator* (const T k, const Rect<T> & r) { return r*k; }

/// Multiply coordinates by p (p*rect = rect*p)
/// \relates Rect
template <typename T>
Rect<T> operator* (const Point<T> p, const Rect<T> & r) { return r*p; }

/// Add p to every point (p+rect = rect+p)
/// \relates Rect
template <typename T>
Rect<T> operator+ (const Point<T> & p, const Rect<T> & r) { return r+p; }

/******************************************************************/
// same functions as in the class

/// rint function: change corner coordenates to nearest integers
/// we can not use rr.to_rint() because of rounding problems (?)
template <typename T>
Rect<T> rint(const Rect<T> & r) { Rect<T> ret(r); ret.to_rint(); return ret;}

/// floor function: shrink the rectangle to nearest integer coordinate
/// Rectangle can be shriked to empty one.
template <typename T>
Rect<T> floor(const Rect<T> & r) { Rect<T> ret(r); ret.to_floor(); return ret; }

/// ceil function: expand the rectangle to nearest integer coordinates
template <typename T>
Rect<T> ceil(const Rect<T> & r) { Rect<T> ret(r); ret.to_ceil(); return ret; }

/// Expand rectangle to each side by val value.
/// \relates Rect
template <typename T>
Rect<T> expand(const Rect<T> & r, T val) { Rect<T> rr(r); rr.expand(val); return rr;}

/// Expand rectangle by vx and vy values.
/// If the rectangle is empty throw an error.
/// If val is negative rectangle can shrink to an empty one.
/// \relates Rect
template <typename T>
Rect<T> expand(const Rect<T> & r, T vx, T vy) { Rect<T> rr(r);  rr.expand(vx,vy); return rr;}

/// Get rectungle which covers rectangle r and point p. Can be used with empty rectangle.
/// \relates Rect
template <typename T>
Rect<T> expand (const Rect<T> & r, const Point<T> & p) { Rect<T> rr(r); rr.expand(p); return rr;}

/// Get rectungle which covers rectangles r1 and r2. Can be used with empty rectangle.
/// \relates Rect
template <typename T>
Rect<T> expand (const Rect<T> & r1, const Rect<T> & r2) { Rect<T> rr(r1); rr.expand(r2); return rr;}

/// Calculate intersection of rectangles r1 and r2. Intersection with
/// and empty rectangle gives empty result.
/// \relates Rect
template <typename T>
Rect<T> intersect (const Rect<T> & r1, const Rect<T> & r2) {Rect<T> rr(r1); rr.intersect(r2); return rr;}

/// Calculate intersection of rectangles r1 and r2.
/// Empty rectangles are ignored.
/// \relates Rect
template <typename T>
Rect<T> intersect_nonempty (const Rect<T> & r1, const Rect<T> & r2) {Rect<T> rr(r1); rr.intersect_nonempty(r2); return rr;}

/// Is rectangle contains a point (only lower bounds are included).
/// \relates Rect
template <typename T>
bool contains_l (const Rect<T> & r, const Point<T> & p) { return r.contains_l(p); }

/// Is rectangle contains a point (only upper bounds are included).
/// \relates Rect
template <typename T>
bool contains_u (const Rect<T> & r, const Point<T> & p) { return r.contains_u(p); }

/// Is rectangle contains a point (bounds are not included).
/// \relates Rect
template <typename T>
bool contains_n (const Rect<T> & r, const Point<T> & p) { return r.contains_n(p); }

/// Is rectangle contains a point (all bounds are included).
/// \relates Rect
template <typename T>
bool contains (const Rect<T> & r, const Point<T> & p) { return r.contains(p); }

/// Is rectangle contains another rectangle.
/// If any of two rectangles is empty false is returned.
/// \relates Rect
template <typename T>
bool contains (const Rect<T> & r1, const Rect<T> & r2) { return r1.contains(r2); }

/// Distance between two rectangles: sqrt(dist(tlc1,tlc1)^2, dist(brc1,brc1)^2).
/// returns 0 for two empty rectangles and +inf for empty and non-empty one.

template <typename T>
double dist(const Rect<T> & r1, const Rect<T> & r2) {
  if (r1.e && r2.e) return 0;
  if (r1.e || r2.e) return INFINITY;
  return hypot(dist(r1.tlc(),r2.tlc()), dist(r1.brc(),r2.brc()));}

/******************************************************************/
// type definitions

/// Rect with double coordinates
/// \relates Rect
typedef Rect<double> dRect;

/// Rect with int coordinates
/// \relates Rect
typedef Rect<int> iRect;

/******************************************************************/
// input/output

/// \relates Rect
/// \brief Output operator: print Rect as a json array: [] or [x,y,w,h]
template <typename T>
std::ostream & operator<< (std::ostream & s, const Rect<T> & r){
  if (r.e) {s << "[]"; return s;}
  s << std::setprecision(9)
    << "[" << r.x << "," << r.y << "," << r.w << "," << r.h << "]";
  return s;
}

// see json_pt.cpp
dRect str_to_rect(const std::string & str);

/// \relates Rect
/// \brief Input operator: read Rect from a json array: [] or [x,y,w,h]
template <typename T>
std::istream & operator>> (std::istream & s, Rect<T> & r){
  // read the whole stream into a string
  std::ostringstream os;
  s >> std::noskipws >> os.rdbuf();
  r=str_to_rect(os.str());
  return s;
}

///@}
#endif
