#include "vmap2io.h"
#include "geo_data/geo_io.h"
#include "geom/poly_tools.h"

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
  auto cl = VMap2obj::get_class(trk_type);

  // Convert tracks.
  // - one segment -> one object
  // - no names
  if (cl!=VMAP2_NONE){
    VMap2obj o1(trk_type);
    for (auto const & tr:data.trks){
      for (auto const & pts:(dMultiLine)tr){
        if (pts.size()==0) continue;
        o1.set_coords(pts);

        // for polygons try to find holes
        if (cl == VMAP2_POLYGON) {
          for (auto const i:vmap2.find(trk_type, pts.bbox())){

            auto o2 = vmap2.get(i);
            if (o2.size()<1) continue; // we will use o2[0] below

            // If o1 has no name and comment and inside
            // first loop of some other object o2, merge it to o2. Note that
            // name and comments are not supported in gpx
            if (check_hole(o2[0], pts)){
             o2.push_back(pts);
             vmap2.put(i, o2);
             o1.clear();
             break;
            }

            // If all loops of other object o2 with empty name and comment
            // are inside o1, merge it to o1.
            // We should check name and comment,
            // because o2 can belong to old data.
            if (o2.name != "" || o2.comm != "") continue;
            bool all_in = true;
            for (const auto & pts2: o2) {
              if (!check_hole(pts, pts2)){
                all_in = false;
                break;
              }
            }
            if (all_in){
              o1.insert(o1.end(), o2.begin(), o2.end());
              vmap2.del(i);
            }
          }
        }
        if (o1.size()==0) continue;
        vmap2.add(o1);
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

