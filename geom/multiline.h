#ifndef MULTILINE_H
#define MULTILINE_H

#include <iostream>
#include <ios>
#include <cmath>
#include <list>
#include <vector>

#include "point.h"
#include "line.h"

///\addtogroup libmapsoft
///@{

/// Line with multiple segments (std::vector of Line)
template <typename CT, typename PT>
struct MultiLine : std::vector<Line<CT,PT> > {

  /// Constructor: make an empty line
  MultiLine() {}

  /// Constructor: make a line using string "[ [[x1,y1],[x2,y2]] , [[x3,y4],[x5,y5]]]"
  MultiLine(const std::string & s) { *this = str_to_type<MultiLine>(s);}

  /******************************************************************/
  // operators +,-,/,*

  /// Add p to every point (shift the line)
  MultiLine<CT,PT> & operator+= (const PT & p) {
    for (auto & l:*this) l+=p;
    return *this;
  }

  /// Subtract p from every point of line
  MultiLine<CT,PT> & operator-= (const PT & p) {
    for (auto & l:*this) l-=p;
    return *this;
  }

  /// Divide coordinates by k
  MultiLine<CT,PT> & operator/= (const CT k) {
    for (auto & l:*this) l/=k;
    return *this;
  }

  /// Multiply coordinates by k
  MultiLine<CT,PT> & operator*= (const CT k) {
    for (auto & l:*this) l*=k;
    return *this;
  }

  /// Add p to every point (shift the line)
  MultiLine<CT,PT> operator+ (const PT & p) const { MultiLine<CT,PT> ret(*this); return ret+=p; }

  /// Subtract p from every point (shift the line)
  MultiLine<CT,PT> operator- (const PT & p) const { MultiLine<CT,PT> ret(*this); return ret-=p; }

  /// Divide coordinates by k
  MultiLine<CT,PT> operator/ (const CT k) const { MultiLine<CT,PT> ret(*this); return ret/=k; }

  /// Multiply coordinates by k
  MultiLine<CT,PT> operator* (const CT k) const { MultiLine<CT,PT> ret(*this); return ret*=k; }

  /// Invert coordinates
  MultiLine<CT,PT> operator- () const {
    MultiLine<CT,PT> ret;
    for (auto const & i:*this) ret.push_back(-i);
    return ret;
  }

  /******************************************************************/
  // operators <=>
  /// Less then operator.
  /// L1 is smaller then L2 if first different line in L1 is smaller or does not exist.
  bool operator< (const MultiLine<CT,PT> & p) const {
    typename MultiLine<CT,PT>::const_iterator i1=this->begin(), i2=p.begin();
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
  bool operator== (const MultiLine<CT,PT> & p) const {
    if (this->size()!=p.size()) return false;
    typename MultiLine<CT,PT>::const_iterator i1=this->begin(), i2=p.begin();
    do {
      if (i1==this->end()) return true;
      if ((*i1)!=(*i2)) return false;
      i1++; i2++;
    } while(1);
  }

  // derived operators:
  bool operator!= (const MultiLine<CT,PT> & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const MultiLine<CT,PT> & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const MultiLine<CT,PT> & other) const { return *this<other || *this==other; } ///< ope
  bool operator>  (const MultiLine<CT,PT> & other) const { return !(*this<=other); } ///< operator>

  /******************************************************************/

  /// Cast to MultiLine<dPoint>.
  operator MultiLine<double,dPoint>() const{
    MultiLine<double,dPoint> ret;
    for (auto const & l:*this) ret.push_back(dLine(l));
    return ret;
  }

  /// Cast to MultiLine<iPoint>.
  operator MultiLine<int,iPoint>() const{
    MultiLine<int,iPoint> ret;
    for (auto const & l:*this) ret.push_back(iLine(l));
    return ret;
  }

  /******************************************************************/
  // Some functions. Below same functions are defined outside the class

 /// Number of points
  size_t npts() const {
    size_t ret = 0;
    for(auto const & l:*this) ret+=l.size();
    return ret;
  }

  /// MultiLine length (sum of segment lengths).
  double length() const {
    double ret=0;
    for(auto const & l:*this) ret+=l.length();
    return ret;
  }

  /// MultiLine 2D length (sum of segment lengths, z is ignored).
  double length2d() const {
    double ret=0;
    for(auto const & l:*this) ret+=l.length2d();
    return ret;
  }

  /// MultiLine bounding box.
  Rect<CT> bbox() const{
    Rect<CT> ret;
    for(auto const & l:*this) ret.expand(l.bbox());
    return ret;
  }

  /// Is MultiLine empty (no segents or all segments are empty)
  bool is_empty() const{
    for (auto const & l:*this) if (l.size()) return false;
    return true;
  }

  /// rint function: change corner coordinates to nearest integers
  void to_rint() { for (auto & l:*this) l.to_rint(); }

  /// floor function: change coordinates to nearest smaller integers
  void to_floor() { for (auto & l:*this) l.to_floor(); }

  /// ceil function: change coordinates to nearest larger integers
  void to_ceil() { for (auto & l:*this) l.to_ceil(); }

  /// abs function: change coordinates to their absolute values
  void to_abs() { for (auto & l:*this) l.to_abs(); }


  /// rotate the MultiLine around c at the angle a (rad, clockwise).
  /// Here we do not use Point::rotate2d to calculate sin/cos only ones
  /// and make things faster.
  void rotate2d(const PT & c, const double a) {
    double C=cos(a), S=sin(a);
    for (auto & l:*this)
      for (auto & p:l)
        p=PT(C*(double)(p.x-c.x)-S*(double)(p.y-c.y)+c.x,
                   C*(double)(p.y-c.y)+S*(double)(p.x-c.x)+c.y, p.z);
  }

  /// Project the multiline into x-y plane
  void flatten() { for (auto & l:*this) l.flatten(); }

  // "close" each segment.
  void close(){ for (auto &l:*this) l.close(); }

  // "open" each segment.
  void open(){ for (auto &l:*this) l.open(); }

  // flip the line around y=y0 line
  void flip_y(const CT y0=0){
    for (auto & l:*this) l.flip_y(y0);
  }

  // flip the line around x=x0 line
  void flip_x(const CT x0=0){
    for (auto & l:*this) l.flip_x(x0);
  }

};

/******************************************************************/
// additional operators

/// Multiply coordinates by k (k*multiline = multiline*k)
/// \relates MultiLine
template <typename CT, typename PT>
MultiLine<CT,PT> operator* (const CT k, const MultiLine<CT,PT> & l) { return l*k; }

/// Add p to every point (shift the line) (p+multiline = multiline+p)
/// \relates MultiLine
template <typename CT, typename PT>
MultiLine<CT,PT> operator+ (const PT & p, const MultiLine<CT,PT> & l) { return l+p; }

/******************************************************************/
// functions, similar to ones inside the class

/// Calculate MultiLine number of points.
/// \relates MultiLine
template <typename CT, typename PT>
size_t npts(const MultiLine<CT,PT> & l){ return l.npts(); }

/// Calculate MultiLine length.
/// \relates MultiLine
template <typename CT, typename PT>
double length(const MultiLine<CT,PT> & l){ return l.length(); }

/// Calculate 2D MultiLine length.
/// \relates MultiLine
template <typename CT, typename PT>
double length2d(const MultiLine<CT,PT> & l){ return l.length2d(); }

/// MultiLine bounding box
/// \relates MultiLine
template <typename CT, typename PT>
Rect<CT> bbox(const MultiLine<CT,PT> & l) { return l.bbox(); }



/// rint function: change corner coordenates to nearest integers
/// \relates Line
template <typename CT, typename PT>
MultiLine<CT,PT> rint(const MultiLine<CT,PT> & ml) {
  MultiLine<CT,PT> ret;
  for (auto & l:ml) ret.push_back(rint(l));
  return ret;
}

/// floor function: change coordinates to nearest smaller integers
/// \relates Line
template <typename CT, typename PT>
MultiLine<CT,PT> floor(const MultiLine<CT,PT> & ml) {
  MultiLine<CT,PT> ret;
  for (auto & l:ml) ret.push_back(floor(l));
  return ret;
}

/// ceil function: change coordinates to nearest larger integers
/// \relates Line
template <typename CT, typename PT>
MultiLine<CT,PT> ceil(const MultiLine<CT,PT> & ml) {
  MultiLine<CT,PT> ret;
  for (auto const & l:ml) ret.push_back(ceil(l));
  return ret;
}

/// abs function: change coordinates to their absolute values
template <typename CT, typename PT>
MultiLine<CT,PT> abs(const MultiLine<CT,PT> & ml) {
  MultiLine<CT,PT> ret(ml);
  for (auto & l:ret) l.to_abs();
  return ret;
}

/// rotate a line around c at the angle a (rad)
template <typename CT, typename PT>
MultiLine<CT,PT> rotate2d(const MultiLine<CT,PT> & ml, const PT & c, const double a) {
  MultiLine<CT,PT> ret(ml);
  ret.rotate2d(c,a);
  return ret;
}

/// Project the line to x-y plane.
template <typename CT, typename PT>
MultiLine<CT,PT> flatten(const MultiLine<CT,PT> & ml) {
  MultiLine<CT,PT> ret;
  for (auto & l:ml) ret.push_back(flatten(l));
  return ret;
}

// "close" the line: add last point equals to the first one
// (if it is not equal)
template <typename CT, typename PT>
MultiLine<CT,PT> close(const MultiLine<CT,PT> & l){
  MultiLine<CT,PT> ret(l); ret.close(); return ret; }

// "open" the line: if the last point equals to the first one
// then remove it.
template <typename CT, typename PT>
MultiLine<CT,PT> open(const MultiLine<CT,PT> & l){
  MultiLine<CT,PT> ret(l); ret.open(); return ret; }

// flip the line around y=y0 line
template <typename CT, typename PT>
MultiLine<CT,PT> flip_y(const MultiLine<CT,PT> & l, const CT y0=0){
  MultiLine<CT,PT> ret(l); ret.flip_y(y0); return ret; }

// flip the line around x=x0 line
template <typename CT, typename PT>
MultiLine<CT,PT> flip_x(const MultiLine<CT,PT> & l, const CT x0=0){
  MultiLine<CT,PT> ret(l); ret.flip_x(x0); return ret; }

/******************************************************************/
// additional functions

/// Distance between two lines A and B: sqrt(sum(dist(A[i],B[i])^2)).
/// Returns +inf for lines with different number of segments
/// or different number of points in a segment.
template <typename CT, typename PT>
double dist(const MultiLine<CT,PT> & A, const MultiLine<CT,PT> & B){
  double ret = 0;
  if (A.size() != B.size()) return INFINITY;
  for (size_t i=0; i<A.size(); i++) ret+=pow(dist(A[i],B[i]),2);
  return sqrt(ret);
}

/******************************************************************/
// type definitions

/// MultiLine with double coordinates
/// \relates MultiLine
typedef MultiLine<double,dPoint> dMultiLine;

/// MultiLine with int coordinates
/// \relates MultiLine
typedef MultiLine<int,iPoint> iMultiLine;

/******************************************************************/
// input/output

/// \relates MultiLine
/// \brief Output operator: print MultiLine as a JSON array of lines
template <typename CT, typename PT>
std::ostream & operator<< (std::ostream & s, const MultiLine<CT,PT> & l){
  s << "[";
  for (typename MultiLine<CT,PT>::const_iterator i=l.begin(); i!=l.end(); i++)
    s << ((i==l.begin())? "":",") << *i;
  s << "]";
  return s;
}


// see json_pt.cpp
dMultiLine str_to_mline(const std::string & str);


/// \brief Input operator: read MultiLine from a JSON array of lines.
/// \note Single line is also allowed.
/// \note This >> operator is different from that in
/// Point or Rect. It always reads the whole stream and
/// returns error if there are extra characters.
/// No possibility to read two objects from one stream.
/// \relates MultiLine
template <typename CT, typename PT>
std::istream & operator>> (std::istream & s, MultiLine<CT,PT> & ml){
  // read the whole stream into a string
  std::ostringstream os;
  s >> std::noskipws >> os.rdbuf();
  ml=str_to_mline(os.str());
  return s;
}

///@}
#endif
