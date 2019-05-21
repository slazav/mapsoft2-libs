-----------------
## Point class

`Point<T>` is a 3D point with coordinates of arbitrary numerical type T.

- Constructors:
  - `Point()` -- point with zero coordinates,
  - `Point(x,y,z=0)` -- point with coordinates x and y,
  - `Point(string)` -- read from std::string (see below).

- Typedefs:
  - `dPoint` is a `Point<double>`,
  - `iPoint` is a `Point<int>`.

- Arithmetic operations (`p` are points, `k` are numbers
  of same type as point coordinates):
  - `p+=p`, `p-=p`, `p+p`, `p-p`, `-p`,
  - `p*=k`, `p/=k`, `p*k`, `k*p`, `p/k`,
  - `p==p`, `p!=p`,
  - `p<p`, `p<=p`, `p>=p`, `p>p`

- Other operations:
  - `p1.swap(p2)` -- swap two points,
  - `dPoint(p)`, `iPoint(p)` -- cast to double- or integer-coordinate point,
  - `p.mlen()`, `mlen(p)` -- manhattan length: abs(x) + abs(y),
  - `p.len()`, `len(p)` -- calculate length: sqrt(x^2 + y^2),
  - `p.norm()`, `norm(p)` -- normalize: p/len(p),
  - `p.rint()`, `rint(p)` -- set coordinates to nearest integer values,
  - `p.floor()`, `floor(p)` -- set coordinates to nearest smaller integers,
  - `p.ceil()`, `ceil(p)` -- set coordinates to nearest larger integers,
  - `p.abs()`,  `abs(p)` -- set coordinates to their absolute values,
  - `p.rotate2d(pc,a)`,  `rotate2d(p,pc,a)` -- rotate around central point pc by angle a (rad, clockwise) in x-y plane,
  - `p.flatten()`,  `flatten(p)` -- project the point to x-y plane.
  - `pscal(p1,p2)` -- scalar product: p1.x*p2.x + p1.y*p2.y + p1.z*p2.z,
  - `dist(p1,p2)` -- distance between points: (p1-p2).len().

- Point can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with two numerical fields `[x,y,z]` or `[x,y]` if `z==0`.

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

- If 3D points are used in operations with 2D rectangles, only `x` and `y`
coordinates are involved.

- Constructors:
  - `Rect()` -- empty rectangle,
  - `Rect(x,y,w,h)` -- non-empty rectangle with x,y,w,h values
     (it is possible to use negative w and h here, then rectangle will be
      put on top of y or on the left of x),
  - `Rect(p1,p2)` -- non-empty rectangle with opposite corners p1,p2.
  - `Rect(string)` -- read from std::string (see below)

- Typedefs:
  - `dRect` is a `Rect<double>`,
  - `iRect` is a `Rect<int>`.

- Arithmetic operations (`r` are rectangles, `p` points, `k` numbers):
  - `r+=p, r-=p, r+p, p+r, r-p, -r`
  - `r*=k, r/=k, r*k, k*r, r/k`
  - `r==r, r!=r`
  - `r<r, r<=r, r>=r, r>r`

- Other operations:
  - `r1.swap(r2)` -- swap rectangles
  - `r.empty()` -- check if rectangle is empty
  - `r.zsize()` -- check if rectangle has zero size (w==0 or h==0, but not empty)
  - `r.tcl(), r.trc(), r.brc(), r.blc(), r.cnt()` -- top-left, top-right, bottom-left, bottom-left corners and central point,
  - `dRect(r)`, `iRect(r)` -- cast to double- or integer-coordinate rectangle
  - `r.rint()`, `rint(r)` -- set coordinates to nearest integer values,
  - `r.floor()`, `floor(r)` -- shrink the rectangle to nearest integers,
  - `r.ceil()`, `ceil(r)` -- expand the rectangle to nearest integers,
  - `r.pump(k)`, `pump(r,k)` -- pump rectangle to each side by k value,
  - `r.pump(kx,ky)`, `pump(r,kx,ky)` -- pump by kx and ky in x and y directions,
  - `r.expand(p)`, `expand(r,p)` -- expand rectangle to cover point p,
  - `r1.expand(r2)`, `expand(r1,r2)` -- expand rectangle to cover rectangle r2,
  - `r1.intersect(r2)`, `intersect(r1,r2)` -- intersection with rectangle r2,
  - `r.contains_l(p)`, `contains_l(r,p)` -- check if rectangle contains a point, only lower bound is included,
  - `r.contains_u(p)`, `contains_u(r,p)` -- check if rectangle contains a point, only upper bound is included,
  - `r.contains_n(p)`, `contains_n(r,p)` -- check if rectangle contains a point, bounds are not included,
  - `r.contains(p)`, `contains(r,p)` -- check if rectangle contains a point, all bounds are included,
  - `r1.contains(r2)`, `contains(r1,r2)` -- check if rectangle contains another rectangle,

- Rect can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with four numerical fields `[x,y,w,h]` or empty array.

-----------------
## Line class

Line is a std::vector of Point.

- Constructors:
  - `Line()` -- empty line,
  - `Line(string)` -- read from std::string (see below)

- Typedefs:
  - `dLine` is a `Line<double>`,
  - `iLine` is a `Line<int>`.

- Arithmetic operations (`l` are lines, `p` points, `k` numbers):
  - `l+=p, l-=p, l+p, p+l, l-p, -l`
  - `l*=k, l/=k, l*k, k*l, l/k`
  - `l==l, l!=l`
  - `l<l, l<=l, l>=l, l>l`

- Other operations:
  - `dLine(l)`, `iLine(l)` -- cast to double- or integer-coordinate line
  - `l.length()`, `length(l)` -- line length
  - `l.invert()`, `invert(l)` -- invert the line
  - `l1.is_shifted(l2, sh)`, `is_shifted(l1, l2, sh)` -- check if line l2 is a shifted version of l1, return the shift
  - `l.rint()`, `rint(l)` -- set coordinates to nearest integer values,
  - `l.bbox2d()`, `bbox2d(l)` -- return a bounding box (Rect object) in x-y plane,
  - `l.rotate2d(pc,a)`,  `rotate2d(l,pc,a)` -- rotate around central point pc by angle a (rad, clockwise) in x-y plane,
  - `l.flatten()`,  `flatten(l)` -- project the line to x-y plane.
  - `rect_to_line(r)` -- convert a rectangle to line.

- Line can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with zero or more points (example: "[[1,2,1],[3,4,2],[0,0]]").

-----------------
## MultiLine class

Line with multiple segments (std::vector of Line).

- Constructors:
  - `MiltiLine()` -- empty line,
  - `MiltiLine(string)` -- read from std::string (see below).

- Typedefs:
  - `dMultiLine` is a `MultiLine<double>`,
  - `iMultiLine` is a `MultiLine<int>`.

- Arithmetic operations (`l` are MultiLines, `p` points, `k` numbers):
  - `l+=p, l-=p, l+p, p+l, l-p, -l`,
  - `l*=k, l/=k, l*k, k*l, l/k`,
  - `l==l, l!=l`,
  - `l<l, l<=l, l>=l, l>l`.

- Other operations:
  - `dMultiLine(l)`, `iMultiLine(l)` -- cast to double- or integer-coordinate MultiLine,
  - `l.length()`, `length(l)` -- line length (sum of segments' lengths),
  - `l.bbox2d()`, `bbox2d(l)` -- return a bounding box in x-y plane (Rect object),
  - `l.rint()`, `rint(l)` -- set coordinates to nearest integer values,
  - `l.rotate2d(pc,a)`,  `rotate2d(l,pc,a)` -- rotate around central point pc by angle a (rad, clockwise) in x-y plane.
  - `l.flatten()`,  `flatten(l)` -- project the multiline to x-y plane.

- MultiLine can be converted to a string and back
  (and thus used inside Opt class). String representation is a
  JSON array with zero or more lines.

-----------------
## LineWalker class

Class for walking alone a line (2D).

- Constructor: `LineWalker lw(dline)`.
- Other methods:
  - `lw.length()` -- get line length,
  - `lw.pt()`     -- get current point,
  - `lw.dist()`   -- get current distance from the line beginning,
  - `lw.tang()`   -- get unit tangent vector at current point,
  - `lw.norm()`   -- get unit normal vector at current point,
  - `lw.get_points(d)` -- get part of line with `d` length, starting from current point, move current point by `d`,
  - `lw.move_begin()` --  move current point to the first node,
  - `lw.move_end()` -- move current point to the last node,
  - `lw.move_frw(d)` -- move current point forward by `d` distance,
  - `lw.move_bck(d)` -- move current point backward by `d` distance,
  - `lw.move_frw_to_node()` - move current point forward to the nearest node,
  - `lw.move_bck_to_node()` - move current point backward to the nearest node,
  - `lw.is_begin()` -- is current point at the first node,
  - `lw.is_end()`   -- is current point at the last node.
