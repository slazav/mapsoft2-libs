#ifndef GEOM_POINT_INT_H
#define GEOM_POINT_INT_H

#include <set>
#include "point.h"
#include "multiline.h"


/// Find one of 8 adjecent points.
/// Direction dir=0..7 corresponds to NW,N,NE,E,SE,S,SW,W.
/// For dir>7 and dir<0 positive modulus is used:
///  adjacent(p, d+8*n) = adjacent(p, d)
iPoint adjacent(const iPoint &p, int dir);

/// Check if points are adjecent. If yes, return
/// direction from p1 to p2, -1 overwise.
int is_adjacent(const iPoint & p1, const iPoint & p2);

/// Make border of a set of points.
std::set<iPoint> border_set(const std::set<iPoint>& pset);

/// Consistently add a point into a set and its border.
/// Return true is the point was added (was not member of the set before).
/// This is much faster then making a set and then using border().
bool add_set_and_border(const iPoint & p,
  std::set<iPoint>& pset, std::set<iPoint>& bord);

/// Convert set of points into its border line.
/// Note: for point [0,0] border is [[0,0],[0,1],[1,1],[1,0]]
iMultiLine border_line(const std::set<iPoint>& pset);

/// Bresenham algorythm: draw a line from p1 to p2,
/// return set of points.
/// see details in https://github.com/slazav/bresenham/blob/master/br.c
/// Line thickness is 2w+1 points. sh - shift to the right (or left if it is negative)
std::set<iPoint> bres_line(iPoint p1, iPoint p2,
                           const int w=0, const int sh=0);

#endif
