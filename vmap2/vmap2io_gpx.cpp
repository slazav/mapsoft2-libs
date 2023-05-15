#include "vmap2io.h"
#include "geo_data/geo_io.h"

/****************************************************************************/

void
gpx_to_vmap2(const std::string & ifile, VMap2 & vmap2, const Opt & opts){

  if (!opts.exists("trk_type") && !opts.exists("wpt_type")) throw Err() <<
    "Use --trk_type or --wpt_type for importing gpx data";
  auto trk_type = VMap2obj::make_type(opts.get("trk_type", "none"));
  auto wpt_type = VMap2obj::make_type(opts.get("wpt_type", "none"));
  auto wpt_pref = opts.get("wpt_pref", "=");

  GeoData data;
  read_gpx(ifile, data, opts);

  // Convert tracks.
  // - one segment -> one object
  // - no names
  if (VMap2obj::get_class(trk_type)!=VMAP2_NONE){
    VMap2obj o(trk_type);
    for (auto const & tr:data.trks){
      for (auto const & l:(dMultiLine)tr){
        if (l.size()==0) continue;
        o.set_coords(l);
        vmap2.add(o);
      }
    }
  }

  // Convert waypoints
  // - one point - one object
  // - set object name from waypoint name
  if (VMap2obj::get_class(wpt_type)!=VMAP2_NONE){
    VMap2obj o(wpt_type);
    for (auto const & wptl:data.wpts){
      for (auto const & w:wptl){
        o.set_coords(w);
        if (w.name.size() > wpt_pref.size() &&
            w.name.substr(0,wpt_pref.size()) == wpt_pref)
          o.name = w.name.substr(wpt_pref.size());
        else o.name = "";
        vmap2.add(o);
      }
    }
  }

}

/****************************************************************************/

void
vmap2_to_gpx(VMap2 & vmap2, const std::string & ofile, const Opt & opts){

  if (!opts.exists("trk_type") && !opts.exists("wpt_type")) throw Err() <<
    "Use --trk_type or --wpt_type for exporting gpx data";
  auto trk_type = VMap2obj::make_type(opts.get("trk_type", "none"));
  auto wpt_type = VMap2obj::make_type(opts.get("wpt_type", "none"));
  auto wpt_pref = opts.get("wpt_pref", "=");

  GeoData data;

  // Convert tracks:
  if (VMap2obj::get_class(trk_type)!=VMAP2_NONE){
    dMultiLine ml;
    for (const auto i: vmap2.find(trk_type)){
      auto o = vmap2.get(i);
      ml.insert(ml.end(), o.begin(), o.end());
    }
    data.trks.push_back(GeoTrk(ml));
  }

  // Convert waypoints:
  if (VMap2obj::get_class(wpt_type)!=VMAP2_NONE){
    GeoWptList wpts;
    for (const auto i: vmap2.find(wpt_type)){
      auto o = vmap2.get(i);
      if (o.size()<1 || o[0].size()<1) continue;
      GeoWpt w((dPoint)o[0][0]);
      if (o.name!="") w.name = wpt_pref + o.name;
      wpts.push_back(w);
    }
    data.wpts.push_back(wpts);
  }

  write_gpx(ofile, data, opts);
}

