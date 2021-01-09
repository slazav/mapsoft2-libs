#ifndef CONV_AFF_H
#define CONV_AFF_H

#include "conv_base.h"
#include <map>

///\addtogroup libmapsoft
///@{

/// 2D affine transformation
class ConvAff2D : public ConvBase {

  std::vector<double> k_frw; ///< transformation parameters (6 numbers)
  std::vector<double> k_bck; ///< parameters of inverse transformation
  void bck_recalc(); ///< recalculate k_bck matrix
  double src_err_x, src_err_y; //< errors in source coordinates.
  double dst_err_x, dst_err_y; //< errors in destination coordinates.

public:
  /// constructor - trivial transformation
  ConvAff2D() {reset();}

  /// constructor - rotation (rad, ccw)
  ConvAff2D(const dPoint & cnt, const double & a) {
    reset(); rotate_src(cnt, a);}

  /// constructor - from a point-to-point reference
  ConvAff2D(const std::map<dPoint, dPoint> & ref) {reset(ref);}

  /// reset to trivial
  void reset();

  /// reset from a point-to-point reference
  void reset(const std::map<dPoint, dPoint> & ref);

  /// point transformation
  void frw_pt(dPoint & p) const override{
    double x = k_frw[0]*p.x + k_frw[1]*p.y + k_frw[2];
    double y = k_frw[3]*p.x + k_frw[4]*p.y + k_frw[5];
    p.x=x; p.y=y; p.z*=sc_src.z*sc_dst.z;
  }

  /// point transformation
  void bck_pt(dPoint & p) const override{
    double x = k_bck[0]*p.x + k_bck[1]*p.y + k_bck[2];
    double y = k_bck[3]*p.x + k_bck[4]*p.y + k_bck[5];
    p.x=x; p.y=y; p.z/=sc_src.z*sc_dst.z;
  }

  // redefine clone() method
  virtual std::shared_ptr<ConvBase> clone() const override{
    return std::shared_ptr<ConvBase>(new ConvAff2D(*this));
  }

  /// forward conversion determinant
  double det() const { return k_frw[0] * k_frw[4] - k_frw[1] * k_frw[3];}

  /// shift before the transformation
  void shift_src(const dPoint & p);

  /// shift after the transformation
  void shift_dst(const dPoint & p);

  // rotate before the transformation (rad, ccw)
  void rotate_src(const dPoint & cnt, const double & a);

  // rotate after the transformation (rad, ccw)
  void rotate_dst(const dPoint & cnt, const double & a);


  ////// rescaling

  /// scale x and y before the transformation
  void set_scale_src(const dPoint & s) override;

  /// scale x and y after the transformation
  void set_scale_dst(const dPoint & s) override;


  // errors
  double get_src_err() const {
    return sqrt(pow(src_err_x,2)+pow(src_err_y,2));}

  double get_dst_err() const {
    return sqrt(pow(dst_err_x,2)+pow(dst_err_y,2));}

};

///@}
#endif
