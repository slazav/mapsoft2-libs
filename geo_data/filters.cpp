#include "geo_nom/geo_nom.h"
#include "geom/poly_tools.h"
#include "conv_geo.h"
#include "geo_utils.h"
#include "filters.h"

#include <iterator>

using namespace std;

/********************************************************************/

// add MS2OPT_GEOFLT options
void
ms2opt_add_geoflt(GetOptSet & opts){
  const char *g = "GEOFLT";
  opts.add("skip",    1, 0, g, "Skip part of geodata. Argument is a string "
                               "which letters show what to skip: W - waypoints, "
                               "T - tracks, M - maps, t - time, z - altitude, "
                               "b - map borders." );
  opts.add("join",     0, 0, g, "Join all waypoint lists, tracks, map lists.");

  opts.add("trk_num",  1, 0, g, "Select track for trk_name/trk_comm options (starting from 1, default is 0 which means all tracks)");
  opts.add("trk_name", 1, 0, g, "Set name for track(s)");
  opts.add("trk_comm", 1, 0, g, "Set comment for track(s)");

  opts.add("maplist_num",  1, 0, g, "Select map list for maplist_name/maplist_comm options (starting from 1, default is 0 which means all map lists)");
  opts.add("maplist_name", 1, 0, g, "Set name for map list(s)");
  opts.add("maplist_comm", 1, 0, g, "Set comment for map list(s)");

  opts.add("wptlist_num",  1, 0, g, "Select wpt list for wptlist_name/wptlist_comm options (starting from 1, default is 0 which means all wpt lists)");
  opts.add("wptlist_name", 1, 0, g, "Set name for wpt list(s)");
  opts.add("wptlist_comm", 1, 0, g, "Set comment for wpt list(s)");

  opts.add("nom_brd", 0, 0, g, "Set map border for a Soviet nomenclature map "
                               "(the map should have a valid name).");
  opts.add("rescale_maps", 1, 0, g, "Rescale image part of map references by some factor.");
  opts.add("shift_maps",   1, 0, g, "Shift image part of map references by some (x,y) vector.");
//  opts.add("trk_reduce",   1, 0, g, "Reduce number of track points. Argument is accuracy in meters, default: 0");
}

/********************************************************************/

void
geo_filters(GeoData & data, const Opt & opt){

  if (opt.exists("skip"))
    filter_skip(data, opt);

  if (opt.exists("join"))
    filter_join(data, opt);

  if (opt.exists("trk_name") || opt.exists("trk_comm"))
    filter_trk(data, opt);

  if (opt.exists("maplist_name") || opt.exists("maplist_comm"))
    filter_map(data, opt);

  if (opt.exists("wptlist_name") || opt.exists("wptlist_comm"))
    filter_wpt(data, opt);

  if (opt.exists("nom_brd"))
    filter_nom_brd(data, opt);

  if (opt.exists("rescale_maps")){
    double sc = opt.get("rescale_maps", 1.0);
    for (auto & ml:data.maps){
      for (auto & m:ml) m*=sc;
    }
  }

  if (opt.exists("shift_maps")){
    dPoint sh = opt.get("shift_maps", dPoint());
    for (auto & ml:data.maps){
      for (auto & m:ml) m+=sh;
    }
  }

//  if (opt.exists("trk_reduce")){
//    double e = opt.get("trk_reduce", 0);
//    int np = 0;
//    for (auto & t:data.trks){
//      line_filter_v1(t, e, np, geo_dist_2d);
//    }
//  }

}


/********************************************************************/

void
filter_skip(GeoData & data, const Opt & opt){

  string sk = opt.get("skip", "");
  if (sk == "") return;

  if (opt.exists("verbose")) cerr << "filter_skip: skip data: " << sk << endl;

  if (sk.find("M")!=string::npos) data.maps.clear();

  if (sk.find("W")!=string::npos) data.wpts.clear();

  if (sk.find("T")!=string::npos) data.trks.clear();

  if (sk.find("t")!=string::npos){
    for (auto & trk:data.trks)
      for (auto & seg:trk)
        for (auto & pt:seg)
          pt.t = 0;
    for (auto & wpl:data.wpts)
      for (auto & pt:wpl) pt.t = 0;
  }

  if (sk.find("z")!=string::npos){
    for (auto & trk:data.trks)
      for (auto & seg:trk)
        for (auto & pt:seg)
          pt.clear_alt();
    for (auto & wpl:data.wpts)
      for (auto & pt:wpl)
        pt.clear_alt();
  }

  if (sk.find("b")!=string::npos){
    for (auto & ml:data.maps)
      for (auto & m:ml) m.border.clear();
  }

}

/********************************************************************/

void
filter_join(GeoData & data, const Opt & opt){
  if (opt.exists("verbose")) cerr << "filter_join: "
    "join all tracks, waypoint lists, map lists" << endl;

  //join maps:
  if (data.maps.size()>1){
    auto m0 = data.maps.begin();
    auto m1 = m0; ++m1;
    while (m1!=data.maps.end()){
      m0->insert(m0->end(), m1->begin(), m1->end());
      m1=data.maps.erase(m1);
    }
  }

  //join wpts:
  if (data.wpts.size()>1){
    auto w0 = data.wpts.begin();
    auto w1 = w0; ++w1;
    while (w1!=data.wpts.end()){
      w0->insert(w0->end(), w1->begin(), w1->end());
      w1=data.wpts.erase(w1);
    }
  }

  //join trks:
  if (data.trks.size()>1){
    auto t0 = data.trks.begin();
    auto t1 = t0; ++t1;
    while (t1!=data.trks.end()){
      t0->insert(t0->end(), t1->begin(), t1->end());
      t1=data.trks.erase(t1);
    }
  }
}

/********************************************************************/

void
filter_trk(GeoData & data, const Opt & opt){
  if (opt.exists("verbose")) cerr << "filter_trk: set track name/comment" << endl;

  int n = opt.get("trk_num", 0);
  if (opt.exists("trk_name")){
    auto name = opt.get("trk_name");
    if (n<=0) for (auto & o: data.trks) o.name = name;
    else {
      if (n>data.trks.size())
        throw Err() << "trk_num is too large, " << data.trks.size() << " tracks available";
      auto i = data.trks.begin();
      std::advance(i, n-1);
      i->name = name;
    }
  }
  if (opt.exists("trk_comm")){
    auto comm = opt.get("trk_comm");
    if (n<=0) for (auto & o: data.trks) o.comm = comm;
    else {
      if (n>data.trks.size())
        throw Err() << "trk_num is too large, " << data.trks.size() << " tracks available";
      auto i = data.trks.begin();
      std::advance(i, n-1);
      i->comm = comm;
    }
  }
}

void
filter_map(GeoData & data, const Opt & opt){
  if (opt.exists("verbose")) cerr << "filter_map: set map list name/comment" << endl;

  int n = opt.get("maplist_num", 0);
  if (opt.exists("maplist_name")){
    auto name = opt.get("maplist_name");
    if (n<=0) for (auto & o: data.maps) o.name = name;
    else {
      if (n>data.maps.size())
        throw Err() << "maplist_num is too large, " << data.maps.size() << " map lists available";
      auto i = data.maps.begin();
      std::advance(i, n-1);
      i->name = name;
    }
  }
  if (opt.exists("maplist_comm")){
    auto comm = opt.get("maplist_comm");
    if (n<=0) for (auto & o: data.maps) o.comm = comm;
    else {
      if (n>data.maps.size())
        throw Err() << "maplist_num is too large, " << data.maps.size() << " map lists available";
      auto i = data.maps.begin();
      std::advance(i, n-1);
      i->comm = comm;
    }
  }
}

void
filter_wpt(GeoData & data, const Opt & opt){
  if (opt.exists("verbose")) cerr << "filter_wpt: set wpt list name/comment" << endl;

  int n = opt.get("wptlist_num", 0);
  if (opt.exists("wptlist_name")){
    auto name = opt.get("wptlist_name");
    if (n<=0) for (auto & o: data.wpts) o.name = name;
    else {
      if (n>data.wpts.size())
        throw Err() << "wptlist_num is too large, " << data.wpts.size() << " wpt lists available";
      auto i = data.wpts.begin();
      std::advance(i, n-1);
      i->name = name;
    }
  }
  if (opt.exists("wptlist_comm")){
    auto comm = opt.get("wptlist_comm");
    if (n<=0) for (auto & o: data.wpts) o.comm = comm;
    else {
      if (n>data.wpts.size())
        throw Err() << "wptlist_num is too large, " << data.wpts.size() << " wpt lists available";
      auto i = data.wpts.begin();
      std::advance(i, n-1);
      i->comm = comm;
    }
  }
}


/********************************************************************/

void
filter_nom_brd(GeoData & data, const Opt & opt){

  if (opt.exists("verbose")) cerr << "filter_nom_brd: "
            "set border for Soviet nomenclature maps" << endl;

  for (auto & ml:data.maps){
    for (auto & m: ml){

      nom_scale_t sc;
      dRect r = nom_to_range(m.name, sc, true);

      if (opt.exists("verbose")){
        if (r) cerr << " - setting border for the nom map " << m.name << endl;
        else cerr << " - skipping non-nomenclature map " << m.name << endl;
      }

      if (!r) return;
      double lon1 = r.x;
      double lat1 = r.y;
      double lon2 = lon1 + r.w;
      double lat2 = lat1 + r.h;

      // Map -> Pulkovo lat-lon
      ConvMap cnv(m, "SU_LL");

      dLine brd;
      brd.push_back(dPoint(lon1,lat2));
      brd.push_back(dPoint(lon2,lat2));
      brd.push_back(dPoint(lon2,lat1));
      brd.push_back(dPoint(lon1,lat1));
      brd.close();
      m.border.clear();
      m.border.push_back(cnv.bck_acc(brd));
    }
  }
}

