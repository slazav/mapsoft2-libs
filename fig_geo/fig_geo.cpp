#include "fig_geo.h"
#include "fig_opt/fig_opt.h"
#include "geo_data/geo_utils.h"
#include "geo_data/conv_geo.h"

#include "image/io.h"

std::string
fig_read_name_(const std::string & c){
  std::istringstream ss(c.substr(3));
  ss >> std::ws;
  std::string n;
  getline(ss, n);
  return n;
}


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
      if (fig_read_name_(c) == ret.name)
        ret.border.push_back((dLine)o);
    }
  }

  // set map projection
  ret.proj = O.get("map_proj", "tmerc");

  // old-style proj settings, default value
  if (ret.proj == "tmerc"){
    std::ostringstream proj_s;
    double lon0 = lon2lon0(ret.bbox_ref_wgs().cnt().x);
    lon0 = O.get("lon0", lon0);
    proj_s << "+datum=WGS84 +proj=tmerc +lon0=" << lon0;
    ret.proj = proj_s.str();
  }
  else if (ret.proj == "latlon"){
    ret.proj = "+datum=WGS84 +proj=latlon";
  }

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
      if (fig_read_name_(c) == name) F.erase(i--);
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
     auto f = figobj_template("2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0");
     f.push_back((iPoint)pp.first);
     std::ostringstream ss;
     ss << "REF " << std::setprecision(8)
        << pp.second.x << " " << pp.second.y;
     f.comment.push_back(ss.str());
     F.push_back(f);
  }

  // add border
  for (const auto & b:m.border){
    auto f = figobj_template("2 3 0 1 5 7 41 -1 -1 0.000 0 0 7 0 0");
    std::string cc("BRD");
    if (m.name.size()) cc+=" ";
    f.comment.push_back(cc + m.name);
    f.set_points(b);
    F.push_back(f);
  }

  // add map projection
  O.put("map_proj", m.proj);

  fig_set_opts(F, O);
}

void
fig_get_wpts(const Fig & F, const GeoMap & m, GeoData & d){
  ConvMap cnv(m); // fig -> wgs
  GeoWptList wpts;
  for (const auto & o:F) {
    if (o.comment.size()<1 || o.size()<1) continue;
    const auto & c = o.comment[0];
    if (c.substr(0,3) != "WPT") continue;
    GeoWpt w(o[0]);
    w.name = fig_read_name_(c);
    cnv.frw(w);
    wpts.push_back(w);
  }
  if (wpts.size()>0) d.wpts.push_back(wpts);
}

void
fig_get_trks(const Fig & F, const GeoMap & m, GeoData & d){
  ConvMap cnv(m); // fig -> wgs
  for (const auto & o:F) {
    if (o.comment.size()<1) continue;
    const auto & c = o.comment[0];
    if (c.substr(0,3) != "TRK") continue;
    GeoTrk t(cnv.frw_pts((dLine)o));
    t.name = fig_read_name_(c);
    d.trks.push_back(t);
  }
}

void
fig_get_maps(const Fig & F, const GeoMap & m, GeoData & d){
  ConvMap cnv(m); // fig -> wgs
  GeoMapList maps;
  for (const auto & o:F) {
    if (o.comment.size()<1) continue; // 1 comment line
    if (o.size()<4) continue; // image object has 4 coordinates

    const auto & c = o.comment[0];
    if (c.substr(0,3) != "MAP") continue;
    GeoMap map;
    map.name  = fig_read_name_(c);
    map.image = o.image_file;
    map.proj  = m.proj;

    // image size can be found only by reading the file:
    map.image_size = image_size(map.image);

    dLine crd_fig(o);
    dLine crd_img = rect_to_line(dRect(dPoint(), map.image_size));
    dLine crd_wgs = cnv.frw_pts(crd_fig);

    map.add_ref(crd_img, crd_wgs);

    // find border
    for (const auto & b:F){
      if (b.comment.size()<1) continue;
      const auto & c = b.comment[0];
      if (c.substr(0,3) == "BRD") {
        if (fig_read_name_(c) == map.name)
          map.border.push_back((dLine)b);
      }
    }
    // convert border fig -> img coords
    ConvAff2D cnv1(crd_fig, crd_img);
    cnv1.frw(map.border);
    maps.push_back(map);
  }
  d.maps.push_back(maps);
}


void
fig_del_wpts(Fig & F){
  for (auto i=F.begin(); i!=F.end(); ++i) {
    if (i->comment.size()<1) continue;
    if (i->comment[0].substr(0,3) == "WPT" ||
        i->comment[0].substr(0,3) == "WPL")
      F.erase(i--);
  }
}

void
fig_del_trks(Fig & F){
  for (auto i=F.begin(); i!=F.end(); ++i) {
    if (i->comment.size()>0 && i->comment[0].substr(0,3) == "TRK")
      F.erase(i--);
  }
}

void
fig_del_maps(Fig & F){
  // delete maps, remember all names
  std::set<std::string> names;
  for (auto i=F.begin(); i!=F.end(); ++i) {
    if (i->comment.size()<1) continue;
    const auto & c = i->comment[0];
    if (c.substr(0,3) == "MAP"){
      names.insert(fig_read_name_(c));
      F.erase(i--);
    }
  }
  // delete borders with same names
  for (auto i=F.begin(); i!=F.end(); ++i) {
    if (i->comment.size()<1) continue;
    const auto & c = i->comment[0];
    if (c.substr(0,3) == "BRD" &&
        names.count(fig_read_name_(c)))
      F.erase(i--);
  }
}

void
fig_add_wpts(Fig & F, const GeoMap & m, const GeoData & d, const Opt & o){
  ConvMap cnv(m); // fig -> wgs
  bool raw = o.get("raw", false);
  std::string wpt_mask = o.get<std::string>("wpt_mask",
    "2 1 0 2 0 7 6 0 -1 1 1 1 -1 0 0");
  std::string txt_mask = o.get<std::string>("txt_mask",
    "4 0 8 5 -1 18 6 0.0000 4");

  auto wpt_tmpl = figobj_template(wpt_mask);
  auto txt_tmpl = figobj_template(txt_mask);

  for (const auto & wl:d.wpts){
    for (const auto & w:wl) {
      iPoint p(flatten(cnv.bck_pts(w)));
      auto wpt_obj(wpt_tmpl);
      wpt_obj.push_back(p);
      if (!raw) wpt_obj.comment.push_back(std::string("WPT ") + w.name);
      F.push_back(wpt_obj);

      if (w.name=="") continue;
      auto txt_obj(txt_tmpl);
      txt_obj.push_back(p+iPoint(30,30));
      txt_obj.text = w.name;
      if (!raw) txt_obj.comment.push_back(std::string("WPL"));
      F.push_back(txt_obj);
    }
  }

}

void
fig_add_trks(Fig & F, const GeoMap & m, const GeoData & d, const Opt & o){
  ConvMap cnv(m); // fig -> wgs

  bool raw = o.get("raw", false);
  std::string trk_mask = o.get<std::string>("trk_mask",
    "2 1 0 2 0 7 6 0 -1 1 1 1 -1 0 0");

  auto trk_tmpl = figobj_template(trk_mask);

  for (const auto & t:d.trks){
    iLine l(flatten(cnv.bck_pts((dLine)t)));
    auto trk_obj(trk_tmpl);
    trk_obj.set_points(l);
    if (!raw) trk_obj.comment.push_back(std::string("TRK ") + t.name);
    F.push_back(trk_obj);
  }
}

