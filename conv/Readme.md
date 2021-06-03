-----------------
## ConvBase class

Trivial 3D point transformation with factors for scaling before
and after the transformation. Children can redefine frw_pt() and bck_pt()
methods to build more complicated transformations. Scaling factors
are applied in a following way:
`dst = f(src*k_src)*k_dst`, `src = f^(-1)(dst/k_dst)/k_src`

Note that in some cases forward and backward conversions are non-symmetric
(accuracy is always calculated in source units).

- `ConvBase()` -- Constructor.

- `frw_pt(dPoint &), bck_pt(dPoint &)` -- Protected functions to be
  redefined in children, forward and backward in-place point conversion. By
  default is is just a rescaling with `rescale_src*rescale_dst` factor.

- `clone()` -- make a std::shared_ptr copy of the object. Should
  be redefined in all derived classes. Allows to make a copy of
  a transformation without knowing it's type.

- `frw(dPoint &), bck(dPoint &), frw(dLine &), bck(dLine &),
   frw(dMultiLine &), bck(MultidLine &)` -- Convert points
    (same as frw_pt, bck_pt), lines and multilines (without changing number of points).

- `frw_pts(const T &)`, `bck_pts(const T &)` -- Convert points, lines and multilines
   without modification of the original object. Return result of the conversion.

- `dLine frw_acc(const dLine & l, double acc) const`
- `dLine bck_acc(const dLine & l, double acc) const` --
  Convert a line. Each segment can be divided to provide
  accuracy `<acc>` in source units (both for `frw_acc` and `bck_acc`).
  If acc<=0 then point-to-point conversion is used.

- `dMultiLine frw_acc(const dMultiLine & l, double acc) const`
- `dMultiLine bck_acc(const dMultiLine & l, double acc) const` --
  Convert a MultiLine. Each sub-line is converted by frw_acc/bck_acc.
  Accuracy `<acc>` is in source units (both for `frw_acc` and `bck_acc`).
  If acc<=0 then point-to-point conversion is used.

- `dRect frw_acc(const dRect & R, double acc) const`,
- `dRect bck_acc(const dRect & R, double acc) const` --
  Convert a rectagle and return bounding box of resulting figure.
  Accuracy `<acc>` is measured in source units (both for frw_acc and bck_acc).
  If acc<=0 then point-to-point conversion is used.

- `virtual double frw_ang(dPoint p, double a, double dx) const`
- `virtual double bck_ang(dPoint p, double a, double dx) const` --
  Forward/backward conversion of angle a at point p.
  Angle is measured in radians from x-axis in the direction of y axis.
  Point p is in src/dst coordinates.
  x and y axes are assumed to be perpendicular.

- `virtual double frw_angd(dPoint p, double a, double dx) const`
  `virtual double bck_angd(dPoint p, double a, double dx) const` --
  Convert angle (degrees, ccw from y=const) at point p.

- `dPoint scales(const dRect & box) const;` --
  Linear scales, destination units per source units in x and y direction.
  box is given in source coordinates.

Scaling factors:

- `void set_scale_src(const dPoint & s)`
- `dPoint get_scale_src() const` -- set/get source scaling factors (x,y,z).

- `void set_scale_dst(const dPoint & s)`
- `dPoint get_scale_dst() const` -- set/get destination scaling factors (x,y,z).

Scaling factors (derived functions)

- `void set_scale_src(const double s)`
- `void set_scale_dst(const double s)` - set scaling factors (x=y=s, z=1).

- `void rescale_src(const dPoint & s)`
- `void rescale_dst(const dPoint & s)` - relative change of scaling factors (x,y,z).

- `void rescale_src(const double s)`
- `void rescale_dst(const double s)` -  relative change of scaling factors (x=y=s, z=1).



-----------------
## ConvMulti class

Composite point transformation, child of ConvBase.

Methods (cnv here has type std::shared_ptr<const ConvBase>, frw is
a boolean flag for direction of the transformation, `true` means forward):
- `ConvMulti()` -- empty (trivial transformation),
- `ConvMulti(cnv1, cnv2, frw1, frw2)` -- Combine two transformations.
- `push_front(cnv, frw)` -- Add a transformation to the beginning of the list.
- `push_back(cnv, frw)`  -- Add a transformation to the end of the list.
- `simplify(box, N, err)` -- Try to substitude all transformation by a single ConvAff.
- `size()` -- Return number of transformations.
- `reset()` -- Reset to the trivial transformation.

-----------------
## ConvAff2D class

2D affine transformation, child of ConvBase.
Works only with `x` and `y` coordinates.

Methods (map is a std::map(dPoint,dPoint)):
 - `ConvAff2D()` -- constructor, trivial transformation,
 - `ConvAff2D(const double & a)` -- constructor, rotation (rad, ccw),
 - `ConvAff2D(map)` -- build a transformation using the map (`map<dPoint,dPoint>`),
 - `ConvAff2D(l1,l2)` -- build a transformation using two lines (`dLine`),
 - `reset()` -- reset to the trivial transformation,
 - `reset(map)` -- reset using the map,
 - `det()` -- forward conversion determinant,
 - `shift_src(p)` -- shift by vector `p` before the transformation,
 - `shift_dst(p)` -- shift by vector `p` after the transformation,
 - `rotate_src(cnt,p)` -- rotate before the transformation (rad, ccw),
 - `rotate_dst(cnt,p)` -- rotate after the transformation (rad, ccw),
 - `rescale_src(kx,ky)` -- rescale `x` and `y` before thetransformation,
 - `rescale_dst(kx,ky)` -- rescale `x` and `y` after thetransformation,
 - `get_src_err()` -- get error in source coordinates
 - `get_dst_err()` -- get error in destination coordinates
