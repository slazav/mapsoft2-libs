#ifndef POLY_OP_H
#define POLY_OP_H

#include "poly_tools.h"
#include "multiline.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <set>

/* Polygon operations (not finished) */

/****************************************************/
// Check if two segments are crossing (2D),
// return crossing point in cr (even if segments are not crossing).
// Both ends included into segments: this may produce
// duplication of crossing points (if line just touches another line),
// but should always produce even number of crossings for closed lines.
// Segments on any parallel lines (even same) are not crossing.
// No crossings for zero-length segments.

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
    cross_t *c1n, *c2n; // next crossing along line1 and line2, NULL if unknown
    cross_t *c1p, *c2p; // previous crossing along line1 and line2, NULL if unknown

    cross_t(const dPoint & pt, const size_t i1b, const size_t & i1e,
            const size_t i2b, const size_t & i2e,
            const double dist1, const double dist2):
      pt(pt), i1b(i1b), i1e(i1e), i2b(i2b), i2e(i2e),
      dist1(dist1), dist2(dist2),
      c1n(NULL), c2n(NULL), c1p(NULL), c2p(NULL) {}
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

  // Fill c1n, c2n, c1p, c2p fields
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
      cross_s[i]->c1n = cross_s[i+1<N? i+1:0];
      cross_s[i]->c1p = cross_s[i>0? i-1:N-1];
    }

    // Same for line2
    struct {
      bool operator()(const cross_t * a, const cross_t * b) const {
        return a->i2b < b->i2b || (a->i2b == b->i2b && a->dist2 < b->dist2); }
    } cross_cmp2;
    std::sort(cross_s.begin(), cross_s.end(), cross_cmp2);

    for (size_t i = 0; i<N; i++){
      cross_s[i]->c2n = cross_s[i+1<N? i+1:0];
      cross_s[i]->c2p = cross_s[i>0? i-1:N-1];
    }
  }

std::cerr << "sort crossings\n";

for (size_t i = 0; i<N; i++){
  std::cerr << "CR: " << i << " " << cross[i].i1b << " <1> " << cross[i].i1e <<  " " << cross[i].i2b << " <2> " << cross[i].i2e << "\n";
}
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
      if (l1) { // switching line1 -> line2
        cross_t *c2n = c1->c2n;
        cross_t *c2p = c1->c2p;
        if (!c2n || !c2p) throw Err() <<
          "NULL crossing address"; // should not happen
        // We should check if segment goes outside or inside previous line.
        // For this we choose reference point: start of next crossing segment
        // if crossings are on different segments, center between crossings otherwise.
        auto pref = line2[c2n->i2b];
        if (c1->i2b == c2n->i2b)
          pref += norm(line2[c2n->i2e] - line2[c2n->i2b]) * c2n->dist2/2.0;
        c2 = (op == POLY_ADD) == pt1.test_pt(pref) ? c2n : c2p;
        // Go from c1 to c2 along line2:
std::cerr << "  l2: " << c1->i2e << " " << c2->i2e <<  "\n";
        if (c1->i2e < c2->i2e)
          for (size_t i = c1->i2e; i!=c2->i2e; i = i<line2.size()-1 ? i+1:0) loop.push_back(line2[i]);
        else
          for (size_t i = c1->i2e; i!=c2->i2e; i = i>0? i-1:line2.size()-1) loop.push_back(line2[i]);
      }
      else { // same for switching line2 -> line1
        cross_t *c2n = c1->c1n;
        cross_t *c2p = c1->c1p;
        if (!c2n || !c2p) throw Err() <<
          "NULL crossing address"; // should not happen
        // We should check if segment goes outside or inside previous line.
        // For this we choose reference point: start of next crossing segment
        // if crossings are on different segments, center between crossings otherwise.
        auto pref = line1[c2n->i1b];
        if (c1->i1b == c2n->i1b)
          pref += (line1[c2n->i1e] - line1[c2n->i1b]) * c2n->dist1/2.0;
        c2 = (op == POLY_ADD) == pt2.test_pt(pref) ? c2n : c2p;
        // Go from c1 to c2 along line1:
std::cerr << "  l1: " << c1->i1e << " " << c2->i1e <<  "\n";
        if (c1->i1e < c2->i1e)
          for (size_t i = c1->i1e; i!=c2->i1e; i = i<line1.size()-1 ? i+1:0) loop.push_back(line1[i]);
        else
          for (size_t i = c1->i1e; i!=c2->i1e; i = i>0 ? i-1:0) loop.push_back(line1[i]);
      }
std::cerr << "  sw: " << c2->i1b << " " << c0->i1b <<  "\n";
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

#endif
