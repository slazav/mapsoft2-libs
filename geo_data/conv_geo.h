#ifndef CONV_GEO_H
#define CONV_GEO_H

#include <memory>
#include <string>
#include "conv/conv_aff.h"
#include "conv/conv_multi.h"
#include "geo_data.h"

///\addtogroup libmapsoft
///@{


/**
Geo transformation, libproj wrapper.

Conversions are constructed using <src> and <dst> strings
which are libproj option string (such as "+datum=WGS84 +proj=lonlat").

A few aliases are supported:
 WGS -- default projection: lon-lat in WGS84 datum
 WEB -- web mercator
 EWEB -- elliptical web mercator (used by Yandex)
 FI, KKJ  -- transverse mercator projection for Finnish maps KKJ (EPSG:2393?)
 ETRS-TM35FIN, ETRS89 -- Finnish maps (EPSG:3067?)
 GB  -- Breat Britain maps (EPSG:27700)
 CH  -- transverse mercator projection for Swiss maps
 SU_LL -- longlat in Soviet map datum
 SU<N> -- transverse mercator projection for Soviet maps
          (N is central meridian: 3, 9, 15, etc.)

Additional parameter 2d switches altitude conversions
(by default altitude is not converted).

*/

// expand proj aliases (such as "WGS", "WEB", "FI", "SU39")
std::string expand_proj_aliases(const std::string & pars);

// Extract component from proj string
std::string get_proj_par(const std::string & proj,
                         const std::string & key, const std::string & def = "");

class ConvGeo: public ConvBase {
public:

  /// Create a transformation. `src` and `dst` are libproj parameters.
  ConvGeo(const std::string & src, const std::string & dst = "WGS", const bool use2d = true);

  /// Forward point conversion.
  void frw_pt(dPoint & p) const override;

  /// Backward point conversion.
  void bck_pt(dPoint & p) const override;

  // redefine clone() method
  virtual std::shared_ptr<ConvBase> clone() const override{
    return std::shared_ptr<ConvBase>(new ConvGeo(*this));
  }

  // get/set 2d flag
  bool get_2d() const {return cnv2d;}
  void set_2d(const bool v = true) { cnv2d = v; }

  // is projection coordinates are in degrees/radians
  static bool is_deg(const std::string & str);
  static bool is_rad(const std::string & str);

private:
  std::shared_ptr<void> pc; // proj context
  std::shared_ptr<void> pj; // crs_to_crs, should be destroyed after context
  bool cnv2d; // Do 2D or 3D conversion
//  bool su_src, su_dst; // Use automatic 6-degree zones (for SU coordinate system)
};


/// Geo transformation from GeoMap
class ConvMap: public ConvMulti {
public:

  /// Create a transformation. `m` is a GeoMap and `dst` is
  /// libproj parameter string.
  ConvMap(const GeoMap & m, const std::string & dst = "WGS");

  // redefine clone() method
  virtual std::shared_ptr<ConvBase> clone() const override{
    return std::shared_ptr<ConvBase>(new ConvMap(*this));
  }

};


///@}
#endif
