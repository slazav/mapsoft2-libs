#include <proj_api.h>

#include "err/err.h"
#include "conv_geo.h"
#include "geo_utils.h"

std::string expand_proj_aliases(const std::string & pars){

  // a few predefined projections
  if (pars == "WGS") // default projection: lon-lat in WGS84 datum
    return "+datum=WGS84 +proj=lonlat";

  if (pars == "WEB") // web mercator
  // in PROJ4 webmerc projection is not supported. 
#if PJ_VERSION < 500
    return "+proj=merc +a=6378137 +b=6378137 +nadgrids=@null +no_defs";
#else
    return "+proj=webmerc +datum=WGS84";
#endif

  if (pars == "EWEB") // elliptical web mercator (used by Yandex)
    return "+proj=merc +datum=WGS84 +no_defs";

  if (pars == "FI" || pars == "KKJ") // Finnish maps, KKJ (EPSG:2393?)
    return "+proj=tmerc +lon_0=27 +x_0=3500000 +ellps=intl"
      " +towgs84=-90.7,-106.1,-119.2,4.09,0.218,-1.05,1.37";

  if (pars == "ETRS-TM35FIN" || pars == "ETRS89") // Finnish maps (EPSG:3067?)
    return "+proj=utm +zone=35 +ellps=GRS80 +units=m +no_defs";

  if (pars == "GB") // Breat Britain maps (EPSG:27700)
    return "+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717"
           " +x_0=400000 +y_0=-100000 +datum=OSGB36 +units=m +no_defs";

  if (pars == "CH") // Swiss maps
    return "+proj=somerc +lat_0=46.95240555555556"\
      " +lon_0=7.439583333333333 +x_0=600000 +y_0=200000"\
      " +ellps=bessel +towgs84=674.374,15.056,405.346,0,0,0,0"\
      " +units=m +no_defs";

  if (pars == "SU_LL") // Soviet datum (lon-lat)
    return "+ellps=krass +towgs84=+28,-130,-95 +proj=lonlat";

  // Soviet coordinate system with automatic 6-degree zones
  // Here we use same settings as in SU_LL, but additional transformation
  // Is applied in frw_pt/bck_pt.
  if (pars == "SU") 
    return "+ellps=krass +towgs84=+28,-130,-95 +proj=lonlat";

  // Soviet coordinate system with central meridian 0.
  // Used in calculations of SU projection
  if (pars == "SUZ")
    return "+ellps=krass +towgs84=+28,-130,-95 +proj=tmerc +lon_0=0 +x_0=500000";

  // SU<N>  -- Soviet coordinate system with central meridian N.
  // SU<N>N -- Same, but coordinates do not have zone prefix.
  if (pars.length()>2 &&
      pars.substr(0,2) == "SU"){
    bool nopref = (pars[pars.length()-1] == 'N');
    int lon0 = str_to_type<int>(pars.substr(2,pars.length() - 2 - (nopref?1:0)));
    if (lon2lon0(lon0) != lon0) throw Err()
      << "Bad central meridian for " << pars << " system. Should have 3+n*6 form.";

    int pref = (lon0<0 ? 60:0) + (lon0-3)/6 + 1;
    return "+ellps=krass +towgs84=+28,-130,-95 +proj=tmerc"
           " +lon_0=" + type_to_str(lon0) +
           " +x_0=" + (nopref?"":type_to_str(pref)) + "500000";
  }


  return pars;
}


std::string
get_proj_par(const std::string & proj,
             const std::string & key, const std::string & def){
  auto exp = expand_proj_aliases(proj);

  auto kv = std::string("+") + key + "=";
  size_t kl = kv.size();

  size_t n1 = exp.find(kv);
  size_t n2 = exp.find(" ", n1);
  return n1!=std::string::npos ? exp.substr(n1+kl,n2-n1-kl) : def;
}


ConvGeo::ConvGeo(const std::string & src,
       const std::string & dst, const bool use2d){
  cnv2d = use2d;

  su_src  = (src == "SU");
  su_dst  = (dst == "SU");

  if (src==dst) {
    pj_src = pj_dst = NULL;
    return;
  }

  // build PROJ handlers
  pj_src = std::shared_ptr<void>(
             pj_init_plus(expand_proj_aliases(src).c_str()), pj_free);
  if (!pj_src)
    throw Err() << "Can't create projection \""
                << src << "\": " << pj_strerrno(pj_errno);

  pj_dst = std::shared_ptr<void>(
             pj_init_plus(expand_proj_aliases(dst).c_str()), pj_free);
  if (!pj_dst)
    throw Err() << "Can't create projection \""
                << dst << "\": " << pj_strerrno(pj_errno);

  // Should we use automatic Soviet 6-degree zone selection?
  // Then we need an additional system (with lon0=0).
  if (su_src || su_dst)
    pj_su = std::shared_ptr<void>(
              pj_init_plus(expand_proj_aliases("SUZ").c_str()), pj_free);

  if (is_src_deg()) sc_src.x = sc_src.y = M_PI/180.0;
  if (is_dst_deg()) sc_dst.x = sc_dst.y = 180.0/M_PI;
}


void
ConvGeo::frw_pt(dPoint & p) const{
  if (sc_src.x!=1.0) {p.x*=sc_src.x;}  // this if increases speed...
  if (sc_src.y!=1.0) {p.y*=sc_src.y;}
  if (sc_src.z!=1.0) {p.z*=sc_src.z;}

  if (pj_src!=pj_dst) {

    // should we use altitude in the conversion?
    double *z = (cnv2d || isnan(p.z))? NULL:&p.z;

    // for SU projection with automatic 6-degree zones
    if (su_src){
      // remove zone prefix and calculate lon0 (see also geo_utils.h)
      int pref = floor(p.x/1e6);
      double lon0 = (pref-(pref>30 ? 60:0))*6 - 3;
      p.x-=pref*1e6;
      // convert from SUZ to SU_LL
      if (pj_transform(pj_su.get(), pj_src.get(), 1, 1, &p.x, &p.y, z)!=0)
        throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);

      // return to origianal meredian
      p.x +=lon0*M_PI/180.0;
    }

    // do the main conversion
    if (pj_transform(pj_src.get(), pj_dst.get(), 1, 1, &p.x, &p.y, z)!=0)
      throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);
    if (!isfinite(p.x) || !isfinite(p.y))
      throw Err() << "Can't convert coordinates: non-numeric result";

    // for SU projection with automatic 6-degree zones
    if (su_dst){
      double lon0 =floor( 180.0/M_PI/6.0*p.x ) * 6 + 3;
      int pref =  (lon0<0 ? 60:0) + (lon0-3)/6 + 1;
      p.x-=lon0*M_PI/180.0; // shift to lon0=0

      // convert from SU_LL to SUZ
      if (pj_transform(pj_dst.get(), pj_su.get(), 1, 1, &p.x, &p.y, z)!=0)
        throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);

      // set prefix
      p.x += pref*1e6;
    }
  }

  if (sc_dst.x!=1.0) {p.x*=sc_dst.x;}
  if (sc_dst.y!=1.0) {p.y*=sc_dst.y;}
  if (sc_dst.z!=1.0) {p.z*=sc_dst.z;}
}

void
ConvGeo::bck_pt(dPoint & p) const{
  if (sc_dst.x!=1.0) {p.x/=sc_dst.x;}
  if (sc_dst.y!=1.0) {p.y/=sc_dst.y;}
  if (sc_dst.z!=1.0) {p.z/=sc_dst.z;}

  if (pj_src!=pj_dst){
    // should we use altitude in the conversion?
    double *z = (cnv2d || isnan(p.z))? NULL:&p.z;

    // for SU projection with automatic 6-degree zones
    if (su_dst){
      // remove zone prefix and calculate lon0 (see also geo_utils.h)
      int pref = floor(p.x/1e6);
      double lon0 = (pref-(pref>30 ? 60:0))*6 - 3;
      p.x-=pref*1e6;

      // convert from SUZ to SU_LL
      if (pj_transform(pj_su.get(), pj_dst.get(), 1, 1, &p.x, &p.y, z)!=0)
        throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);

      p.x +=lon0*M_PI/180.0; // shift to corect lon0
    }

    // do the main conversion
    if (pj_transform(pj_dst.get(), pj_src.get(), 1, 1, &p.x, &p.y, z)!=0)
      throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);
    if (!isfinite(p.x) || !isfinite(p.y))
      throw Err() << "Can't convert coordinates: non-numeric result";

    // for SU projection with automatic 6-degree zones
    if (su_src){
      double lon0 =floor( 180.0/M_PI*p.x/6.0 ) * 6 + 3;
      int pref =  (lon0<0 ? 60:0) + (lon0-3)/6 + 1;
      p.x-=lon0*M_PI/180.0; // shift to lon0=0

      // convert from SU_LL to SUZ
      if (pj_transform(pj_src.get(), pj_su.get(), 1, 1, &p.x, &p.y, z)!=0)
        throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);

      // add prefix
      p.x += pref*1e6;
    }

  }
  if (sc_src.x!=1.0) {p.x/=sc_src.x;}
  if (sc_src.y!=1.0) {p.y/=sc_src.y;}
  if (sc_src.z!=1.0) {p.z/=sc_src.z;}
}

bool
ConvGeo::is_src_deg() const {
  return !su_src && pj_is_latlong(pj_src.get());}

bool
ConvGeo::is_dst_deg() const {
  return !su_dst && pj_is_latlong(pj_dst.get());}

/**********************************************************/

ConvMap::ConvMap(const GeoMap & m, const std::string & dst) {

  // convert refpoints to map projection
  ConvGeo map2wgs(m.proj);
  std::map<dPoint,dPoint> refpts = m.ref;
  for (auto & pp:refpts) map2wgs.bck(pp.second);

  push_back(ConvAff2D(refpts)); // image -> map proj
  push_back(ConvGeo(m.proj, dst)); // map proj -> dst
}


