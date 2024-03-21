#ifndef LINE_H
#define LINE_H

#include <iostream>
#include <algorithm>
#include <ios>
#include <cmath>
#include <list>
#include <vector>
#include "err/err.h"
#include "opt/opt.h"

#include "point.h"
#include "rect.h"

///\addtogroup libmapsoft
///@{

/// 2d line with point type PT and coordinate type CT
template <typename CT, typename PT>
struct Line : std::vector<PT> {

  /// Constructor: make an empty line
  Line() {}

  /// Constructor: make a line using string "[[x1,y1],[x2,y2]]"
  Line(const std::string & s) { *this = str_to_type<Line>(s);}

  /******************************************************************/
  // operators +,-,/,*

  /// Add p to every point (shift the line)
  Line<CT,PT> & operator+= (const PT & p) {
    for (typename Line<CT,PT>::iterator i=this->begin(); i!=this->end(); i++) (*i)+=p;
    return *this;
  }

  /// Subtract p from every point of line
  Line<CT,PT> & operator-= (const PT & p) {
    for (typename Line<CT,PT>::iterator i=this->begin(); i!=this->end(); i++) (*i)-=p;
    return *this;
  }

  /// Divide coordinates by k
  Line<CT,PT> & operator/= (const CT k) {
    for (typename Line<CT,PT>::iterator i=this->begin(); i!=this->end(); i++) (*i)/=k;
    return *this;
  }

  /// Multiply coordinates by k
  Line<CT,PT> & operator*= (const CT k) {
    for (typename Line<CT,PT>::iterator i=this->begin(); i!=this->end(); i++) (*i)*=k;
    return *this;
  }

  /// Divide coordinates by p
  Line<CT,PT> & operator/= (const PT p) {
    for (typename Line<CT,PT>::iterator i=this->begin(); i!=this->end(); i++) (*i)/=p;
    return *this;
  }

  /// Multiply coordinates by p
  Line<CT,PT> & operator*= (const PT p) {
    for (typename Line<CT,PT>::iterator i=this->begin(); i!=this->end(); i++) (*i)*=p;
    return *this;
  }

  /// Add p to every point (shift the line)
  Line<CT,PT> operator+ (const PT & p) const { Line<CT,PT> ret(*this); return ret+=p; }

  /// Subtract p from every point (shift the line)
  Line<CT,PT> operator- (const PT & p) const { Line<CT,PT> ret(*this); return ret-=p; }

  /// Divide coordinates by k
  Line<CT,PT> operator/ (const CT k) const { Line<CT,PT> ret(*this); return ret/=k; }

  /// Multiply coordinates by k
  Line<CT,PT> operator* (const CT k) const { Line<CT,PT> ret(*this); return ret*=k; }

  /// Divide coordinates by p
  Line<CT,PT> operator/ (const PT p) const { Line<CT,PT> ret(*this); return ret/=p; }

  /// Multiply coordinates by p
  Line<CT,PT> operator* (const PT p) const { Line<CT,PT> ret(*this); return ret*=p; }

  /// Invert coordinates
  Line<CT,PT> operator- () const {
    Line<CT,PT> ret;
    for (typename Line<CT,PT>::const_iterator i=this->begin(); i!=this->end(); i++)
      ret.push_back(-(*i));
    return ret;
  }

  /******************************************************************/
  // operators <=>

  /// Less then operator.
  /// L1 is smaller then L2 if first different point in L1 is smaller or does not exist.
  bool operator< (const Line<CT,PT> & p) const {
    typename Line<CT,PT>::const_iterator i1=this->begin(), i2=p.begin();
    do {
      if (i1==this->end()){
        if (i2==p.end()) return false;
        else return true;
      }
      else if (i2==p.end()) return false;

      if ((*i1)!=(*i2)) return (*i1) < (*i2);
      i1++; i2++;
    } while(1);
  }

  /// Equal opertator.
  bool operator== (const Line<CT,PT> & p) const {
    if (this->size()!=p.size()) return false;
    typename Line<CT,PT>::const_iterator i1=this->begin(), i2=p.begin();
    do {
      if (i1==this->end()) return true;
      if ((*i1)!=(*i2)) return false;
      i1++; i2++;
    } while(1);
  }

  /// Same comparison as == but for opposite directions.
  bool is_inv(const Line<CT,PT> & p) const {
    if (this->size()!=p.size()) return false;
    typename Line<CT,PT>::const_iterator i1=this->begin();
    typename Line<CT,PT>::const_reverse_iterator  i2=p.rbegin();
    do {
      if (i1==this->end()) return true;
      if ((*i1)!=(*i2)) return false;
      i1++; i2++;
    } while(1);
  }

  // derived operators:
  bool operator!= (const Line<CT,PT> & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const Line<CT,PT> & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const Line<CT,PT> & other) const { return *this<other || *this==other; } ///< operator<
  bool operator>  (const Line<CT,PT> & other) const { return !(*this<=other); } ///< operator>

  /******************************************************************/

  /// Cast to Line<double, dPoint>
  operator Line<double,dPoint>() const{
    Line<double,dPoint> ret;
    for (auto const & p:*this) ret.push_back(dPoint(p));
    return ret;
  }

  /// Cast to Line<int, iPoint>
  operator Line<int,iPoint>() const{
    Line<int,iPoint> ret;
    for (auto const & p:*this) ret.push_back(iPoint(p));
    return ret;
  }

  /******************************************************************/
  // Some functions. Below same functions are defined outside the class

  /// Number of points
  size_t npts() const {return this->size();}

  /// Calculate line length.
  double length() const {
    double ret=0;
    for (size_t i=1; i<this->size(); i++)
      ret+=dist((*this)[i-1], (*this)[i]);
    return ret;
  }

  /// Calculate line length.
  double length2d() const {
    double ret=0;
    for (size_t i=1; i<this->size(); i++)
      ret+=dist2d((*this)[i-1], (*this)[i]);
    return ret;
  }

  /// Line bounding box in x-y plane
  Rect<CT> bbox() const{
    if (this->size()<1) return Rect<CT>();
    PT min((*this)[0]), max((*this)[0]);

    for (auto const & p:*this){
      if (p.x > max.x) max.x = p.x;
      if (p.y > max.y) max.y = p.y;
      if (p.x < min.x) min.x = p.x;
      if (p.y < min.y) min.y = p.y;
    }
    return Rect<CT>(min,max);
  }

  /// Is line l just shifted version of this. Shift is returned
  bool is_shifted(const Line<CT,PT> & l, PT & shift) const{
    shift = PT();
    if (this->size()!=l.size()) return false;
    if (this->size()==0) return true;
    typename Line<CT,PT>::const_iterator i1=this->begin(), i2=l.begin();
    shift = (*i2) - (*i1);
    do {
      if (i1==this->end()) return true;
      if ((*i2)-(*i1) != shift) return false;
      i1++; i2++;
    } while(1);
  }

  /// Is line empty
  bool is_empty() const {return this->size() == 0;}

  /// Invert line.
  void invert(void) { std::reverse(this->begin(), this->end()); }

  /// rint function: change corner coordinates to nearest integers
  void to_rint() { for (auto & p:*this) p.to_rint(); }

  /// floor function: change coordinates to nearest smaller integers
  void to_floor() { for (auto & p:*this) p.to_floor(); }

  /// ceil function: change coordinates to nearest larger integers
  void to_ceil() { for (auto & p:*this) p.to_ceil(); }

  /// abs function: change coordinates to their absolute values
  void to_abs() { for (auto & p:*this) p.to_abs(); }

  /// Rotate the line around c at the angle a (rad, clockwise) in x-y plane.
  /// Here we do not use Point::rotate2d to calculate sin/cos only ones
  /// and make things faster.
  void rotate2d(const PT & c, const double a) {
    double C=cos(a), S=sin(a);
    for (auto & p:*this)
      p=PT(C*(double)(p.x-c.x)-S*(double)(p.y-c.y)+c.x,
                 C*(double)(p.y-c.y)+S*(double)(p.x-c.x)+c.y, p.z);
  }

  /// Project the line into x-y plane
  void flatten() { for (auto & p:*this) p.z=0; }

  // "close" the line: add last point equals to the first one
  // (if it is not equal)
  void close(){
    if (this->size()>1 && *(this->rbegin())!=*(this->begin()))
      this->push_back(*(this->begin()));
  }

  // "open" the line: if the last point equals to the first one
  // then remove it.
  void open(){
    if (this->size()>1 && *(this->rbegin())==*(this->begin()))
      this->resize(this->size()-1);
  }

  // flip the line around y=y0 line
  void flip_y(const CT y0=0){
    for (auto & p:*this) p.y = y0 - p.y;
  }

  // flip the line around x=x0 line
  void flip_x(const CT x0=0){
    for (auto & p:*this) p.x = x0 - p.x;
  }

};

/******************************************************************/
// additional operators

/// Multiply coordinates by CT (k*line = line*k)
/// \relates Line
template <typename CT, typename PT>
Line<CT,PT> operator* (const CT k, const Line<CT, PT> & l) { return l*k; }

/// Multiply coordinates by PT (p*line = line*p)
/// \relates Line
template <typename CT, typename PT>
Line<CT,PT> operator* (const PT p, const Line<CT, PT> & l) { return l*p; }

/// Add p to every point (p+line = line+p)
/// \relates Line
template <typename CT, typename PT>
Line<CT,PT> operator+ (const PT & p, const Line<CT, PT> & l) { return l+p; }

/******************************************************************/
// functions, similar to ones in the class

/// Calculate number of points.
/// \relates Line
template <typename CT, typename PT>
size_t npts(const Line<CT,PT> & l){ return l.npts(); }

/// Calculate line length.
/// \relates Line
template <typename CT, typename PT>
double length(const Line<CT,PT> & l){ return l.length(); }

/// Calculate 2D line length.
/// \relates Line
template <typename CT, typename PT>
double length2d(const Line<CT,PT> & l){ return l.length2d(); }

/// Line bounding box
/// \relates Line
template <typename CT, typename PT>
Rect<CT> bbox(const Line<CT,PT> & l) { return l.bbox(); }

/// Is line l just shifted version of this. Shift is returned
/// \relates Line
template <typename CT, typename PT>
bool is_shifted(const Line<CT,PT> & l1, const Line<CT,PT> & l2, PT & shift){
  return l1.is_shifted(l2, shift);
}


/// Invert line.
/// \relates Line
template <typename CT, typename PT>
Line<CT,PT> invert(const Line<CT,PT> & l) {
  Line<CT,PT> ret(l);
  std::reverse(ret.begin(), ret.end());
  return ret;
}

/// rint function: change corner coordenates to nearest integers
/// \relates Line
template <typename CT, typename PT>
Line<CT,PT> rint(const Line<CT,PT> & l) {
  Line<CT,PT> ret;
  for (auto const & p:l) ret.push_back(rint(p));
  return ret;
}

/// floor function: change coordinates to nearest smaller integers
/// \relates Line
template <typename CT, typename PT>
Line<CT,PT> floor(const Line<CT,PT> & l) {
  Line<CT,PT> ret;
  for (auto const & p:l) ret.push_back(floor(p));
  return ret;
}

/// ceil function: change coordinates to nearest larger integers
/// \relates Line
template <typename CT, typename PT>
Line<CT,PT> ceil(const Line<CT,PT> & l) {
  Line<CT,PT> ret;
  for (auto const & p:l) ret.push_back(ceil(p));
  return ret;
}

/// abs function: change coordinates to their absolute values
template <typename CT, typename PT>
Line<CT,PT> abs(const Line<CT,PT> & l) {
  Line<CT,PT> ret(l);
  for (auto & p:ret) p.to_abs();
  return ret;
}

/// rotate a line around c at the angle a (rad)
template <typename CT, typename PT>
Line<CT,PT> rotate2d(const Line<CT,PT> & l, const PT & c, const double a) {
  Line<CT,PT> ret(l);
  ret.rotate2d(c,a);
  return ret;
}

/// Project the line to x-y plane.
template <typename CT, typename PT>
Line<CT,PT> flatten(const Line<CT,PT> & l) {
  Line<CT,PT> ret(l);
  for (auto & p:ret) p.z=0;
  return ret;
}

// "close" the line: add last point equals to the first one
// (if it is not equal)
template <typename CT, typename PT>
Line<CT,PT> close(const Line<CT,PT> & l){
  Line<CT,PT> ret(l); ret.close(); return ret; }

// "open" the line: if the last point equals to the first one
// then remove it.
template <typename CT, typename PT>
Line<CT,PT> open(const Line<CT,PT> & l){
  Line<CT,PT> ret(l); ret.open(); return ret; }

// flip the line around y=y0 line
template <typename CT, typename PT>
Line<CT,PT> flip_y(const Line<CT,PT> & l, const CT y0=0){
  Line<CT,PT> ret(l); ret.flip_y(y0); return ret; }

// flip the line around x=x0 line
template <typename CT, typename PT>
Line<CT,PT> flip_x(const Line<CT,PT> & l, const CT x0=0){
  Line<CT,PT> ret(l); ret.flip_x(x0); return ret; }

/******************************************************************/
// additional functions

/// Convert rectangle to a line. Line goes clockwise,
/// starting from top-left corner (<tlc>, <trc>, <brc>, <blc>, and
/// if <closed> parameter is true back to <tlc>).
/// \relates Line
template <typename CT>
Line<CT,Point<CT> > rect_to_line(const Rect<CT> & r, bool closed=true) {
  Line<CT,Point<CT> > ret;
  ret.push_back(r.tlc());
  ret.push_back(r.trc());
  ret.push_back(r.brc());
  ret.push_back(r.blc());
  if (closed) ret.push_back(r.tlc());
  return ret;
}

/// Distance between two lines A and B: sqrt(sum(dist(A[i],B[i])^2)).
/// Returns +inf for lines with different number of points.
template <typename CT, typename PT>
double dist(const Line<CT,PT> & A, const Line<CT,PT> & B){
  double ret = 0;
  if (A.size() != B.size()) return INFINITY;
  for (size_t i=0; i<A.size(); i++) ret+=pow(dist(A[i],B[i]),2);
  return sqrt(ret);
}

/******************************************************************/
// type definitions

/// Line with double coordinates
/// \relates Line
typedef Line<double, dPoint> dLine;

/// Line with int coordinates
/// \relates Line
typedef Line<int, iPoint> iLine;

/******************************************************************/
// input/output

/// \relates Line
/// \brief Output operator: print Line as a JSON array of points
template <typename CT, typename PT>
std::ostream & operator<< (std::ostream & s, const Line<CT,PT> & l){
  s << "[";
  for (typename Line<CT,PT>::const_iterator i=l.begin(); i!=l.end(); i++)
    s << ((i==l.begin())? "":",") << *i;
  s << "]";
  return s;
}

// see json_pt.cpp
dLine str_to_line(const std::string & str);

/// Input operator: read Line from a JSON array of points
/// This >> operator is different from that in
/// Point or Rect. It always reads the whole stream and
/// returns error if there are extra characters.
/// No possibility to read two objects from one stream.
/// \relates Line
template <typename CT, typename PT>
std::istream & operator>> (std::istream & s, Line<CT,PT> & l){
  // read the whole stream into a string
  std::ostringstream os;
  s >> std::noskipws >> os.rdbuf();
  l=str_to_line(os.str());
  return s;
}

///@}
#endif
