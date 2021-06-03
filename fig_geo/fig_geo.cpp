#include "fig_geo.h"
#include "fig_opt/fig_opt.h"
#include "geo_data/geo_utils.h"
#include "geo_data/conv_geo.h"


GeoMap
fig_get_ref(const Fig & F) {

  GeoMap ret;
  Opt O = fig_get_opts(F);

  ret.name = O.get("name");

  // Read REF and BRD objects
  for (const auto & o:F){
    if (o.comment.size()<1) continue;
    const auto & c = o.comment[0];

    // find objects where first comment is "REF <lon> <lat>"
    if (c.size()>=3 && c.substr(0,3) == "REF") {
      std::istringstream ss(c.substr(3));
      dPoint ptl;
      ss >> ptl.x >> ptl.y;
      if (ss.fail()) throw Err() << "FigGeo: can't read REF point: " << c;

      if (o.size()<1) throw Err() << "FigGeo: REF object does not contain fig coordinates: " << c;
      ret.ref.emplace(o[0], ptl);
    }

    if (c.size()>=3 && c.substr(0,3) == "BRD") {
      std::istringstream ss(c.substr(3));
      ss >> std::ws;
      std::string n;
      getline(ss, n);
      if (n == ret.name) ret.border.push_back((dLine)o);
    }
  }

  // set map projection
  std::ostringstream proj_s;
  std::string map_proj = O.get("map_proj", "tmerc");

  if (map_proj == "tmerc"){
    double lon0 = lon2lon0(ret.bbox_ref_wgs().cnt().x);
    lon0 = O.get("lon0", lon0);
    proj_s << "+datum=WGS84 +proj=tmerc +lon0=" << lon0;
  }
  else if (map_proj == "latlon"){
    proj_s << "+datum=WGS84 +proj=latlon";
  }
  else throw Err() << "GeoFig: unsupported projection setting: " << map_proj;
  ret.proj = proj_s.str();

  return ret;
}


void
fig_del_ref(Fig & F){

  Opt O = fig_get_opts(F);
  std::string name = O.get("name");
  O.erase("name");
  O.erase("map_proj");
  O.erase("lon0");
  fig_set_opts(F, O);

  for (auto i = F.begin(); i!=F.end(); i++){
    if (i->comment.size()<1) continue;
    const auto & c = i->comment[0];
    if (c.substr(0,3) == "REF") {
      F.erase(i--);
      continue;
    }
    if (c.substr(0,3) == "BRD") {
      std::istringstream ss(c.substr(3));
      ss >> std::ws;
      std::string n;
      getline(ss, n);
      if (n == name) F.erase(i--);
      continue;
    }
  }
}

void
fig_add_ref(Fig & F, const GeoMap & m){

  Opt O = fig_get_opts(F);
  O.put("name", m.name);

  // add ref points
  for (const auto & pp: m.ref){
     auto f = read_figobj_header("2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 0");
     f.push_back((iPoint)pp.first);
     std::ostringstream ss;
     ss << "REF " << std::setprecision(8)
        << pp.second.x << " " << pp.second.y;
     f.comment.push_back(ss.str());
     F.push_back(f);
  }

  // add border
  for (const auto & b:m.border){
    auto f = read_figobj_header("2 3 0 1 5 7 41 -1 -1 0.000 0 0 7 0 0 0");
    std::string cc("BRD");
    if (m.name.size()) cc+=" ";
    f.comment.push_back(cc + m.name);
    f.set_points(b);
    F.push_back(f);
  }

  // add map projection
  O.put("map_proj", get_proj_par(m.proj, "proj"));
  auto lon0 = get_proj_par(m.proj, "lon0");
  if (lon0 != "") O.put("lon0", lon0);

  fig_set_opts(F, O);
}
