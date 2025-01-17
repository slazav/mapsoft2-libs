-----------------
## Point class

`Point<T>` is a 3D point with coordinates of arbitrary numerical type T.

- Constructors:
  - `Point()` -- point with zero coordinates.
  - `Point(x,y,z=0)` -- point with coordinates x,y and z.
  - `Point(string)` -- read from std::string (see below).

- Typedefs:
  - `dPoint` is a `Point<double>`.
  - `iPoint` is a `Point<int>`.

- Arithmetic operations (`p` are points, `k` are numbers
  of same type as point coordinates):
  - `p+=p`, `p-=p`, `p+p`, `p-p`, `-p`.
  - `p*=k`, `p/=k`, `p*k`, `k*p`, `p/k`.
  - `p*=p1`, `p/=p1`, `p*p1`, `p1*p`, `p/p1`.
  - `p==p`, `p!=p`.
  - `p<p`, `p<=p`, `p>=p`, `p>p`.

- Other operations:
  - `p1.swap(p2)` -- swap two points.
  - `dPoint(p)`, `iPoint(p)` -- cast to double- or integer-coordinate point.
  - `p.mlen()`, `mlen(p)` -- manhattan length: abs(x) + abs(y) + abs(z).
  - `p.len()`, `len(p)` -- length: sqrt(x^2 + y^2 + z^2).
  - `p.mlen2d()`, `mlen2d(p)` -- 2D manhattan length: abs(x) + abs(y).
  - `p.len2d()`, `len2d(p)` --  2D length: sqrt(x^2 + y^2).

  - `p.to_rint()` -- set coordinates to nearest integer values.
  - `p.to_floor()` -- set coordinates to nearest smaller integers.
  - `p.to_ceil()`  -- set coordinates to nearest larger integers.
  - `p.to_abs()` -- set coordinates to their absolute values.
  - `p.rotate2d(pc,a)` -- rotate around central point pc by angle a (rad, anticlockwise) in x-y plane.
  - `p.flatten()` -- project the point to x-y plane.
  - `rint(p)`, `floor(p)`, `ceil(p)`, `abs(p)`, `rotate2d(p,pc,a)`, `flatten(p)` --
    do same operations, returned modified point, keep original point unchanged.

  - `pscal(p1,p2)` -- scalar product: p1.x*p2.x + p1.y*p2.y + p1.z*p2.z.
  - `pscal2d(p1,p2)` -- 2D scalar product: p1.x*p2.x + p1.y*p2.y.
  - `dist(p1,p2)` -- distance between points: (p1-p2).len().
  - `dist2d(p1,p2)` -- 2D distance between points: (p1-p2).len2d().
  - `norm(p)` -- normalize: p/len(p), throw error if len(p)==0.
  - `norm2d(p)` -- 2D normalize, z is set to 0, throw error if len2d(p)==0.

- Point can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with two or three numerical fields, `[x,y,z]` or
  `[x,y]` for `z==0`. Coordinates can be numericals (integer of floating point)
   or strings containing integer values in decimal or hex form
   (e.g. 0.1, 0, "0", "0xA").

-----------------
## Rect class

Rect<T> is a 2D rectangle with coordinates of arbitrary numerical type T.
Rectangle is defined by top-left corner coordinates `x,y`, width `w`,
height `h` and empty flag `e`.

- Coordinates are counted from top-left.
  Top-left corner has lowest coordinates, bottom-right corner
  has highest coordinates. Width and height are always non-negative.

- There is a difference between empty rectangle (such as a
  bounding box of a zero-point line) and zero-size rectangle
  (bounding box of a one-point line). Many functions throw
  error if rectangle is empty.

- If 3D points are used in operations with rectangles, only `x` and `y`
coordinates are involved.

- Constructors:
  - `Rect()` -- empty rectangle.
  - `Rect(x,y,w,h)` -- non-empty rectangle with x,y,w,h values.
     It is possible to use negative w and h in the constructor.
     Resulting rectungle will have positive width and height, but will
     be located on top of y or on the left of x.
  - `Rect(p1,p2)` -- non-empty rectangle with opposite corners
                     defined by points p1,p2.
  - `Rect(string)` -- read from std::string (see below).

- Typedefs:
  - `dRect` is a `Rect<double>`,
  - `iRect` is a `Rect<int>`.

- Arithmetic operations (`r` are rectangles, `p` points, `k` numbers):
  - `r+=p, r-=p, r+p, p+r, r-p, -r`
  - `r*=k, r/=k, r*k, k*r, r/k`
  - `r==r, r!=r`
  - `r<r, r<=r, r>=r, r>r`

- Other operations:
  - `r1.swap(r2)` -- swap rectangles.
  - `r.is_empty()` -- check if rectangle is empty. same as `!(bool)r`.
  - `r.is_zsize()` -- check if rectangle has zero size (w==0 or h==0) or empty.
  - `r.tcl(), r.trc(), r.brc(), r.blc(), r.cnt()` -- top-left, top-right,
    bottom-left, bottom-left corners and central point.
  - `dRect(r)` -- cast to double- or integer-coordinate rectangle.
  - `r.to_rint()` -- set coordinates to nearest integer values.
  - `r.to_floor()` -- shrink the rectangle to nearest integers (may become empty!).
  - `r.to_ceil()` -- expand the rectangle to nearest integers.
  - `r.expand(k)` -- expand rectangle to each side by k value.
  - `r.expand(kx,ky)` -- expand by kx and ky in x and y directions.
  - `r.expand(p)` -- expand rectangle to cover point p.
  - `r1.expand(r2)` -- expand rectangle to cover rectangle r2.
  - `r1.intersect(r2)` -- intersect with rectangle r2 (intersection with empty rectangle is empty).
  - `r1.intersect_nonempty(r2)` -- intersect with rectangle r2 (intersection with empty rectangle is original rectangle).

  - `rint(r)`, `floor(r)`, `ceil(r)`, `expand(r,k)`, `expand(r,kx,ky)`, 
    `expand(r,p)`, `expand(r1,r2)`, `intersect(r1,r2)`, `intersect_nonempty(r1,r2)` --
    do same operations, returned modified rectagles but keep original rectangle
    unchanged.

  - `r.contains_l(p)`, `contains_l(r,p)` -- check if rectangle contains a point, only lower bound is included.
  - `r.contains_u(p)`, `contains_u(r,p)` -- check if rectangle contains a point, only upper bound is included.
  - `r.contains_n(p)`, `contains_n(r,p)` -- check if rectangle contains a point, bounds are not included.
  - `r.contains(p)`, `contains(r,p)` -- check if rectangle contains a point, all bounds are included.
  - `r1.contains(r2)`, `contains(r1,r2)` -- check if rectangle contains another rectangle.
  - `dist(r1,r2)` -- "distance" between rectangles, hypot(dist(tlc1,tlc2),dist(brc1,brc2)).
    Returns 0 for two empty rectangles and +inf for empty and non-empty one.

- Rectangle can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with four fields `[x,y,w,h]` or empty array.
  Coordinates can be numericals (integer of floating point) or strings
  containing integer values in decimal or hex form (e.g. 0.1, 0, "0", "0xA").

-----------------
## Line class

Line is a std::vector of Point.

- Constructors:
  - `Line()` -- empty line.
  - `Line(string)` -- read from std::string (see below).

- Typedefs:
  - `dLine` is a `Line<double>`.
  - `iLine` is a `Line<int>`.

- Arithmetic operations (`l` are lines, `p` points, `k` numbers):
  - `l+=p, l-=p, l+p, p+l, l-p, -l`
  - `l*=k, l/=k, l*k, k*l, l/k`
  - `l*=p, l/=p, l*p, p*l, l/p`
  - `l==l, l!=l`
  - `l<l, l<=l, l>=l, l>l`

- Other operations:
  - `dLine(l)`, `iLine(l)` -- cast to double- or integer-coordinate line.
  - `l.length()`, `length(l)` -- line length.
  - `l.npts()`, `npts(l)` -- number of points.
  - `l.length2d()`, `length2d(l)` -- 2D line length.
  - `l.area()`, `area(l)` -- 2D area of closed polygon, sign shows orientation.
  - `l.bbox()`, `bbox(l)` -- return a bounding box (Rect object) in x-y plane.
  - `l1.is_shifted(l2, sh)`, `is_shifted(l1, l2, sh)` -- check if line l2 is a
    shifted version of l1, return the shift.
  - `l.is_empty()` -- check if line is empty (has no points), same as l.size()==0.

  - `l.invert()` -- invert the line.
  - `l.to_rint()` -- set coordinates to nearest integer values.
  - `l.to_floor()` -- set coordinates to nearest smaller integers.
  - `l.to_ceil()`  -- set coordinates to nearest larger integers.
  - `l.to_abs()` -- set coordinates to their absolute values.
  - `l.rotate2d(pc,a)` -- rotate around central point pc by angle a (rad, anticlockwise) in x-y plane.
  - `l.flatten()` -- project the line to x-y plane (set z to 0).
  - `l.close()` -- "close" the line (if it is not closed): add last point equals to the first one.
  - `l.open()` --  "open" the line: if the last point equals to the first one remove it.

  - `invert(l)`, `rint(l)`, `to_floor(l)`, `ceil(l)`, `abs(l)`,
    `rotate2d(l,pc,a)`, `flatten(l)`, `close(l)`, `open(l)` --
    do same operations, return modified line, keep original line unchanged.

  - `rect_to_line(r, closed=true)` -- convert a rectangle to line.
  - `dist(l1,l2)` -- "distance" between lines, `sqrt(sum(dist(l1[i],l2[i])^2))`,
                     returns +inf for lines fith different number of points.

- Line can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with zero or more points (example: `[[1,2,1],[3,4,2],[0,0]]`).

-----------------
## MultiLine class

Line with multiple segments (std::vector of Line).

- Constructors:
  - `MiltiLine()` -- empty line.
  - `MiltiLine(string)` -- read from std::string (see below).

- Typedefs:
  - `dMultiLine` is a `MultiLine<double>`.
  - `iMultiLine` is a `MultiLine<int>`.

- Arithmetic operations (`l` are MultiLines, `p` points, `k` numbers):
  - `l+=p, l-=p, l+p, p+l, l-p, -l`
  - `l*=k, l/=k, l*k, k*l, l/k`
  - `l*=p, l/=p, l*p, p*l, l/p`
  - `l==l, l!=l`,
  - `l<l, l<=l, l>=l, l>l`

- Other operations:
  - `l.add_point(pt)` -- Add point to the end of the last segment (create segment if needed).
  - `l.add_segment()` -- Add empty segment.
  - `l.del_last_point()` -- Remove last point and all empty segments at the end.
  - `dMultiLine(l)`, `iMultiLine(l)` -- cast to double- or integer-coordinate MultiLine.
  - `l.length()`, `length(l)` -- line length (sum of segments' lengths).
  - `l.npts()`, `npts(l)` -- number of points (sum of segments' point numbers).
  - `l.bbox()`, `bbox(l)` -- return a bounding box in x-y plane (Rect object).
  - `l.is_empty()` -- check if MultiLine is empty (has no segents or only empty segments).
  - `add_segment()` -- add a new empty segment
  - `add_point(p)` -- add a point to the end of last segment, or to a new segment if no one exists
  - `get_first_pt()` -- get first point, throw error if line is empty
  - `del_last_point()` -- remove last point and delete all empty segments at the end

  - `p.to_rint()` -- set coordinates to nearest integer values.
  - `p.to_floor()` -- set coordinates to nearest smaller integers.
  - `p.to_ceil()`  -- set coordinates to nearest larger integers.
  - `p.to_abs()` -- set coordinates to their absolute values.
  - `l.rotate2d(pc,a)` -- rotate around central point pc by angle a (rad, anticlockwise) in x-y plane.
  - `l.flatten()` -- project the line to x-y plane (set z to 0).
  - `l.close()` -- "close" the line (if it is not closed): add last point equals to the first one.
  - `l.open()` --  "open" the line: if the last point equals to the first one remove it.
  - `l.flip_x(x0=0)`, `l.flip_y(y0=0)` -- flip the line around x=x0 or y=y0 line.

  - `rint(l)`, `to_floor(l)`, `ceil(l)`, `abs(l)`, `rotate2d(l,pc,a)`,
    `flatten(l)`, `close(l)`, `open(l)`, `flip_x(x0)`, `flip_y(y0)` --
     do same operations, return modified point, keep original point unchanged.

  - `dist(l1,l2)` -- "distance" between multilines, `sqrt(sum(dist(l1[i],l2[i])^2))`,
                     returns +inf for multilines with different number of segments.

- MultiLine can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with zero or more lines (example: `[[[1,2,1],[3,4,2],[0,0]], [[1,1],[2,2]], []]`).
  JSON string with non-empty Line can be read as a single-segment MultiLine:
  `MultiLine("[[1,1],[2,2]]") == MultiLine("[[[1,1],[2,2]]]")`, but
  `MultiLine("[]") != MultiLine("[[]]")`.

-----------------
## LineWalker class

Class for walking alone a line (2D).

- Constructor: `LineWalker lw(dline, close=false)`.
- Other methods:
  - `lw.length()` -- get line length.
  - `lw.pt()`     -- get current point (initially it is set to the beginning of the line).
  - `lw.dist()`   -- get current distance from the line beginning.
  - `lw.tang()`   -- get unit tangent vector at current point.
  - `lw.norm()`   -- get unit normal vector at current point.
  - `lw.ang()`    -- get line direction (radians, from x axis).
  - `lw.get_points(d)` -- get part of line with `d` length, starting from current point, move current point by `d`.
  - `lw.move_begin()` --  move current point to the first node.
  - `lw.move_end()` -- move current point to the last node.
  - `lw.move_frw(d)` -- move current point forward by `d` distance.
  - `lw.move_bck(d)` -- move current point backward by `d` distance.
  - `lw.move_frw_to_node()` - move current point forward to the nearest node.
  - `lw.move_bck_to_node()` - move current point backward to the nearest node.
  - `lw.is_begin()` -- is current point at the first node.
  - `lw.is_end()`   -- is current point at the last node.

-----------------
## poly_tools.h -- extra polygon-related functions

* `template <typename T> class PolyTester` -- 
  Class for checking if a point is inside a polygon.

  - `PolyTester(const Line<T> & L, const bool horiz = true, const bool borders=true)` --
    Constructor, build the tester class for a given polygon.
    Arguments: L - polygon represented by Line object, horiz - test direction,
    borders -- include polygon borders.

  - `std::vector<T> get_cr(T y)` -- Get vector of crossings of horizontal
    (vertical) line y with the polygon.

  - `bool test_cr(const std::vector<T> & cr, T x)` test if point `(x,y)`
    is inside the polygon (cr is crossing vector build with y coordinate).

  Typedefs:

  - `dPolyTester` -- same as `PolyTester<double>`
  - `iPolyTester` -- same as `PolyTester<int>`

* `bool point_in_polygon(const Point<T> & P, const Line<T> & L, const bool borders=true)` --
  Check if one-segment polygon L covers point P.

* `int rect_in_polygon(const Rect<T> & R, const Line<T> & L, const bool borders=true)` --
  Check if one-segment polygon L covers (maybe partially) rectangle R.
  Return values: 0 - polygon and rectange are not crossing, 1 - border of the polygon is
  crossing rectangle boundary, 2 - rectangle is fully inside the polygon, 3 - polygon is
  fully inside the rectangle.

* `int rect_in_polygon(const Rect<T> & R, const MultiLine<T> & L, const bool borders=true)` --
  Same for multi-segment polygons.

* `Line<T> join_polygons(const MultiLine<T> & L)` -- Join a multi-segment
  polygon into a single-segment one using shortest cuts.

* `void remove_holes(MultiLine<T> & L)` -- Remove holes in a multi-segment
  polygon using shortest cuts.

* `figure_line(string)` -- Read a figure from the string and represent it as a MultiLine.
  The figure can be Point, Line/Multiline, Rect.

* `figure_bbox(string)` -- Read a figure from the string and get its bounding box.
  The figure can be Point, Line/Multiline, Rect.

* `nearest_pt(line or multiline, point)` -- Return point of the line which is
  nearest to the given point. Throw error if line is empty (or all points are
  non-finite).

* `nearest_dist(line or multiline, point)` -- Return distance to the nearest point
  of the line. Throw error if line is empty (or all points are
  non-finite).

-----------------
## line_int.h -- extra functions for handling iPoint sets

* `iPoint adjacent(const iPoint &p, int dir)` --  Find one of 8 adjecent points.
  Direction dir=0..7 corresponds to NW,N,NE,E,SE,S,SW,W. For dir>7 and
  dir<0 positive modulus is used: `adjacent(p, d+8*n) = adjacent(p, d)`

* `int is_adjacent(const iPoint & p1, const iPoint & p2)` --
  Check if points are adjecent. If yes, return direction from p1 to p2, -1
  overwise.

* `std::set<iPoint> border_set(const std::set<iPoint>& pset)` -- Make border
  of a set of points.

* `bool add_set_and_border(const iPoint & p,
    std::set<iPoint>& pset, std::set<iPoint>& bord)` --  Consistently add a
  point into a set and its border. Return true is the point was added (was
  not member of the set before). This is much faster then making a set and
  then using `border()`.

* `iMultiLine border_line(const std::set<iPoint>& pset)` --
  Convert set of points into its border line. ote: for point [0,0] border
  is [[0,0],[0,1],[1,1],[1,0]] (for dPoints one may want to shift it by -[0.5,0.5]).

* `std::set<iPoint> bres_line(iPoint p1, iPoint p2,
                      const int w=0, const int sh=0)` -- Bresenham
algorythm: draw a line from `p1` to `p2`, return set of points. Line
thickness is `2w+1` points, `sh` is shift to the right (or left if it is
negative). See details in https://github.com/slazav/bresenham/blob/master/br.c
