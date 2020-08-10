#ifndef CONV_MULTI_H
#define CONV_MULTI_H

#include "conv_base.h"
#include <list>
#include <memory>

///\addtogroup libmapsoft
///@{

/// Composite conversion
class ConvMulti : public ConvBase {

  std::list<std::pair<bool, std::shared_ptr<ConvBase> > > cnvs;

public:

  /// constructor - trivial transformation
  ConvMulti(){}

  /// constructor - 2 conversions
  ConvMulti(const ConvBase & cnv1, const ConvBase & cnv2,
            bool frw1=true, bool frw2=true){
    cnvs.push_back(std::make_pair(frw1, cnv1.clone()));
    cnvs.push_back(std::make_pair(frw2, cnv2.clone()));
  }

  // reset to trivial conversion
  void reset(){
    cnvs.clear();
    set_scale_src(1.0);
    set_scale_dst(1.0);
  }

  /// add a conversion in front of the list
  void push_front(const ConvBase & cnv, bool frw=true){
    cnvs.push_front(std::make_pair(frw, cnv.clone()));
  }

  /// add a conversion at the end of the list
  void push_back(const ConvBase & cnv, bool frw=true){
    cnvs.push_back(std::make_pair(frw, cnv.clone()));
  }

  /// redefine a forward point conversion
  void frw_pt(dPoint & p) const override {
    p.x*=sc_src.x; p.y*=sc_src.y;
    for (auto i = cnvs.begin(); i!=cnvs.end(); ++i)
      if (i->first) i->second->frw(p); else i->second->bck(p);
    p.x*=sc_dst.x; p.y*=sc_dst.y;
  }

  /// redefine a backward point conversion
  void bck_pt(dPoint & p) const override {
    p.x/=sc_dst.x; p.y/=sc_dst.y;
    for (auto i = cnvs.rbegin(); i!=cnvs.rend(); ++i)
      if (i->first) i->second->bck(p); else i->second->frw(p);
    p.x/=sc_src.x; p.y/=sc_src.y;
  }

  // redefine clone() method
  virtual std::shared_ptr<ConvBase> clone() const override{
    return std::shared_ptr<ConvBase>(new ConvMulti(*this));
  }

  /// Try to substitude all conversions by a single ConvAff.
  //  Algorythm:
  //  - make NxN points in the src_box, convert src->dst
  //  - build ConvAff using these points
  //  - convert points back to src coordinates
  //  - measure error (mean square distance, in src coordinates)
  //  - if error < E substitude all conversions with the new one
  //  - return true if substitution was none, false otherwise.
  bool simplify(const dRect & src_box, int N, double E = 1);

  int size() const {return cnvs.size();}

};

///@}
#endif
