#ifndef CONV_BASE_H
#define CONV_BASE_H

#include <memory> // shared_ptr
#include "geom/point.h"
#include "geom/line.h"
#include "geom/multiline.h"
#include "geom/rect.h"

///\addtogroup libmapsoft
///@{

/// Trivial point transformation. Children can
/// redefine frw_pt() and bck_pt() methods.
/// Also sc_src and sc_dst parameters should be used (or
/// rescale_src/rescale_dst redifined).
struct ConvBase{

  /// constructor - trivial transformation
  ConvBase(double sc=1.0): sc_src(1.0, 1.0, 1.0), sc_dst(1.0, 1.0, 1.0){}

  protected:
    // forward point conversion (can be redefined)
    virtual void frw_pt(dPoint & p) const {
      p.x*=sc_src.x*sc_dst.x; p.y*=sc_src.y*sc_dst.y; p.z*=sc_src.z*sc_dst.z;}

    // backward point conversion (can be redefined)
    virtual void bck_pt(dPoint & p) const {
      p.x/=sc_src.x*sc_dst.x; p.y/=sc_src.y*sc_dst.y; p.z/=sc_src.z*sc_dst.z;}

  public:

  // Get copy of the object. Should be redefined in derived classes.
  // Allows to copy Conv* class without knowing its actual type.
  virtual std::shared_ptr<ConvBase> clone() const {
    return std::shared_ptr<ConvBase>(new ConvBase(*this));
  }

  /* conversions, in-place versions */

  /// Forward point transformation.
  virtual void frw(dPoint & p) const {frw_pt(p);}

  /// Backward point transformation.
  virtual void bck(dPoint & p) const {bck_pt(p);}

  /// Convert a Line, point to point.
  virtual void frw(dLine & l) const { for (auto & p:l) frw(p); }

  /// Convert a Line, point to point.
  virtual void bck(dLine & l) const { for (auto & p:l) bck(p); }

  /// Convert a MultiLine, point to point.
  virtual void frw(dMultiLine & ml) const { for (auto & l:ml) frw(l); }

  /// Convert a MultiLine, point to point.
  virtual void bck(dMultiLine & ml) const { for (auto & l:ml) bck(l); }

  /* conversions, no modification of original object */

  /// Forward Point/Line/MultiLine transformation.
  template <typename T>
  T frw_pts(const T & p) const { T ret(p); frw(ret); return ret;}

  /// Backward Point/Line/MultiLine transformation.
  template <typename T>
  T bck_pts(const T & p) const { T ret(p); bck(ret); return ret;}

  /* conversions, with accuracy setting */

  /// Convert a line. Each segment can be divided to provide
  /// accuracy <acc> in source units.
  /// If acc<=0 then point-to-point conversion is used.
  virtual dLine frw_acc(const dLine & l, double acc = 0.5) const;

  /// Convert a line. Each segment can be divided to provide
  /// accuracy <acc> in source units.
  /// If acc<=0 then point-to-point conversion is used.
  /// Note that bck_acc and frw_acc are not symmetric
  /// because accuracy is always calculated on the src side.
  virtual dLine bck_acc(const dLine & l, double acc = 0.5) const;

  /// Convert a MultiLine. Each segment of each line
  /// can be divided to provide accuracy <acc> in source units.
  virtual dMultiLine frw_acc(const dMultiLine & l, double acc = 0.5) const;

  /// Convert a MultiLine. Each segment of each line can be
  /// divided to provide accuracy <acc> in source units.
  // Note that bck_acc and frw_acc are not symmetric
  // because accuracy is always calculated on the src side.
  virtual dMultiLine bck_acc(const dMultiLine & l, double acc = 0.5) const;

  /// Convert a rectagle and return bounding box of resulting figure.
  /// Accuracy <acc> is measured in x-y plane in source units.
  virtual dRect frw_acc(const dRect & R, double acc = 0.5) const {
    return frw_acc(rect_to_line(R), acc).bbox(); }

  /// Convert a rectagle and return bounding box of resulting figure.
  /// Accuracy <acc> is measured in x-y plane in source units (and
  /// thus bck_acc and frw_acc are not symmetric).
  virtual dRect bck_acc(const dRect & R, double acc = 0.5) const {
    return bck_acc(rect_to_line(R), acc).bbox(); }

  /// Forward conversion of angle a at point p.
  /// Angle is measured in radians from x-axis in the direction of y axis.
  /// Point p is in src coordinates.
  /// x and y axes are assumed to be perpendicular.
  virtual double frw_ang(dPoint p, double a, double dx) const;

  /// Backward conversion of angle a at point p.
  /// Angle is measured in radians from x-axis in the direction of y axis.
  /// Point p is in dst coordinates.
  /// x and y axes are assumed to be perpendicular.
  virtual double bck_ang(dPoint p, double a, double dx) const;

  /// Convert angle (degrees, ccw from y=const) at point p (in dst coords).
  virtual double frw_angd(dPoint p, double a, double dx) const;
  virtual double bck_angd(dPoint p, double a, double dx) const;

  /// Linear scales, destination units per source units in x and y direction.
  /// box is given in source coordinates.
  dPoint scales(const dRect & box) const;


  // Scaling functions. Children should use sc_src/sc_dst
  // parameters or redefine this functions.

  /// set sc_src (scaling before conversion) parameter
  virtual void set_scale_src(const dPoint & s) { sc_src=s; }

  /// get sc_src (scaling before conversion) parameter
  virtual dPoint get_scale_src() const { return sc_src; }

  /// set sc_dst (scaling after conversion) parameter
  virtual void set_scale_dst(const dPoint & s) { sc_dst=s; }

  /// get sc_dst (scaling after conversion) parameter
  virtual dPoint get_scale_dst() const { return sc_dst; }


  // derived scale functions

  /// set sc_src (scaling before conversion), same in x and y, 1 in z
  void set_scale_src(const double s) { set_scale_src(dPoint(s,s,1));}

  /// set sc_src (scaling after conversion), same in x and y
  void set_scale_dst(const double s) { set_scale_dst(dPoint(s,s,1));}

  /// relative change of sc_src parameter
  void rescale_src(const dPoint & s) {
    dPoint s0 = get_scale_src();
    set_scale_src(dPoint(s0.x*s.x, s0.y*s.y, s0.z*s.z));
  }

  /// relative change of sc_dst parameter
  void rescale_dst(const dPoint & s) {
    dPoint s0 = get_scale_dst();
    set_scale_dst(dPoint(s0.x*s.x, s0.y*s.y, s0.z*s.z));
  }

  /// relative change of sc_src parameter, same in x and y, 1 in z
  void rescale_src(const double s) { rescale_src(dPoint(s,s,1));}

  /// relative change of sc_dst parameter, same in x and y
  void rescale_dst(const double & s) { rescale_dst(dPoint(s,s,1));}

protected:
  dPoint sc_src, sc_dst;
};

///@}
#endif
