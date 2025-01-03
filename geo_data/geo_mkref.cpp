#include <sstream>
#include <iomanip>

#include "geo_mkref.h"
#include "conv_geo.h"
#include "geo_io.h"
#include "geo_tiles/geo_tiles.h"
#include "geo_nom/geo_nom.h"
#include "geo_nom/geo_nom_fi.h"
#include "geo_utils.h"
#include "geom/multiline.h"
#include "geom/poly_tools.h"
#include "err/err.h"


using namespace std;

void
ms2opt_add_mkref_opts(GetOptSet & opts){
  const char *g = "MKREF_OPTS";
  opts.add("mkref", 1,0,g,
    "Choose map type (nom, nom_fi, google_tile, tms_tile, proj)");
  opts.add("name", 1,0,g,
    "Set map name. For --mkref=\"nom\" it should contain a "
    "valid Soviet nomenclature name. For --mkref=\"file\" it should contain map-file name.");
  opts.add("dpi", 1,0,g,
    "Map resolution, pixels per inch (\"nom\" and \"proj\" maps)");
  opts.add("mag", 1,0,g,
    "Map magnification (\"nom\" maps)");
  opts.add("north", 0,0,g,
    "Orient map to north (\"nom\" maps)");
  opts.add("margins",  1,0,g,
    "Map margins, pixels (\"nom\" and \"proj\" maps).");
  opts.add("top_margin", 1,0,g,
    "Override top margin value (\"nom\" and \"proj\" maps).");
  opts.add("left_margin", 1,0,g,
    "Override left margin value (\"nom\" and \"proj\" maps).");
  opts.add("right_margin", 1,0,g,
    "Override right margin value (\"nom\" and \"proj\" maps).");
  opts.add("bottom_margin", 1,0,g,
    "Override bottom margin value (\"nom\" and \"proj\" maps).");
  opts.add("zindex", 1,0,g,
    "Tile zindex (\"*_tile\" maps). Can be skipped if tile argument "
    "has the form [x,y,z]");
  opts.add("tiles", 1,0,g,
    "Tile or tile range (\"*_tile\" maps), [x,y], [x,y,z], or [x,y,w,h]");
  opts.add("coords", 1,0,g,
    "Figure in projection coordinates (rectangle or lines) to "
    "be covered by the map (\"proj\" maps). "
    "Figure can be a rectangle written as [x,y,w,h], or a line, "
    "[[x1,y1],[x2,y2], ...], or a multi-segment line, "
    "[<line>, <line>, ...].");
  opts.add("coords_wgs", 1,0,g,
    "Figure in wgs84 coordinates to be covered by the map "
    "(\"*_tile\" or \"proj\" maps), a rectangle, a line, or a multi-segment line.");
  opts.add("coords_file", 1,0,g,
    "Use tracks and points in the file to calculate map size "
    "(\"*_tile\" or \"proj\" maps).");
  opts.add("coords_nom", 1,0,g,
    "Use Soviet nomenclature name calculate map size "
    "(\"*_tile\" or \"proj\" maps).");
  opts.add("border", 1,0,g,
    "Map border in projection coordinates (For --mkref=\"proj\" maps), "
    "a line or a multi-segment line.");
  opts.add("proj", 1,0,g,
    "Projection setting, \"libproj\" parameter string "
    "(e.g. \"+datum=WGS84 +proj=lonlat\") "
    "or mapsoft2 alias (\"WGS\", \"WEB\", \"FI\", \"CH\", \"SU39\", etc.)."
    "Default is WGS.");
  opts.add("scale", 1,0,g,
    "Map scale, projection units per map cm (\"proj\" maps). "
    "Default value is 0.01 degree/cm for degree projections, "
    "1000m/cm for metric projections.");
}

void
ms2opt_add_mkref_brd(GetOptSet & opts){
  const char *g = "MKREF_BRD";
  opts.add("border_wgs", 1,0,g,
    "Set map border in wgs84 coordinates "
    "(argument is a line or a multi-segment line).");
  opts.add("border_file", 1,0,g,
    "Set map border from track in a file.");
}

void
ms2opt_add_mkref_data(GetOptSet & opts){
  const char *g = "MKREF_DATA";
  opts.add("mag", 1,0,g,
    "When using existing map as a reference source, rescale it.");
}


/********************************************************************/
// helper function: read a line (w/o z coord) from a geofile,
// use first track.
dMultiLine read_geoline(const std::string & fname){
  GeoData d;
  read_geo(fname, d);
  if (d.trks.size()<1) throw Err()
      << "mkref: can't read any track from file: " << fname;
  dMultiLine ret = *d.trks.begin();
  ret.flatten(); // remove z component
  return ret;
}

// Get bbox (tracks and wpts) from a geofile,
// use first track.
dRect read_bbox(const std::string & fname){
  dRect bbox;
  GeoData data;
  read_geo(fname, data);
  for (auto const & t:data.trks) bbox.expand(t.bbox());
  for (auto const & w:data.wpts) bbox.expand(w.bbox());
  return bbox;
}

/********************************************************************/

GeoMap
geo_mkref_nom(const std::string & name,
              double dpi, double mag, bool north,
              int mt, int ml, int mr, int mb){

  GeoMap map;
  map.name = name;

  // Map range (in pulkovo coordinates)
  nom_scale_t sc;
  dRect R = nom_to_range(map.name, sc, true);

  // map projection (use a bit shifted longitude to calculate boundary lon0)
  map.proj = "SU" + type_to_str(lon2lon0(R.x + 1e-6));

  string proj_pulk = "SU_LL";
  // conversion map_projection -> pulkovo
  ConvMulti cnv1;
  cnv1.push_back(ConvGeo(map.proj, proj_pulk));
  // conversion pulkovo -> wgs84
  ConvGeo cnv2(proj_pulk);

  // map resolution
  map.image_dpi = dpi;

  // factor (map coordinates (m))/(map point)
  double k = (int)sc/mag * 25.4e-3 /*m/in*/ / map.image_dpi;
  cnv1.rescale_src(k); // now cnv1: map points -> pulkovo

  // Orient to north (R in LL coordinates)
  if (north){
    double a = cnv1.bck_ang(R.cnt(), 0, 0.1);
    cnv1.push_front(ConvAff2D(R.cnt(), a));
  }

  // Border in map points (1pt accuracy);
  // We convert a closed line, then removing the last point.
  dLine brd = open(cnv1.bck_acc(rect_to_line(R, true)));

  // image size
  iRect image_bbox = ceil(cnv1.bck_acc(R));

  // Refpoints:
  dLine pts_r = rect_to_line(R, false);
  cnv1.bck(pts_r); // pulkovo -> map points
  pts_r.to_floor();

  dLine pts_w = pts_r;
  cnv1.frw(pts_w); // map points -> pulkovo
  cnv2.frw(pts_w);  // pulkovo -> wgs

  // margins
  image_bbox = iRect(image_bbox.x-ml, image_bbox.y-mb,
                     image_bbox.w+ml+mr, image_bbox.h+mt+mb);

  brd -= image_bbox.tlc();
  pts_r -= image_bbox.tlc();
  brd.flip_y(image_bbox.h);
  pts_r.flip_y(image_bbox.h);
  map.image_size = iPoint(image_bbox.w, image_bbox.h);

  // Add map border:
  map.border.push_back(brd);

  // Add refpoints:
  map.add_ref(pts_r, pts_w);
  return map;
}

GeoMap
geo_mkref_nom_fi(const std::string & name,
                 double dpi, double mag,
                 int mt, int ml, int mr, int mb){

  GeoMap map;
  map.name = name;

  // Map range (in ETRS-TM35FIN meters)
  dRect R = nom_to_range_fi(map.name);

  // map projection
  map.proj = "ETRS-TM35FIN";

  // conversion map -> wgs84
  ConvGeo cnv(map.proj);

  // map resolution
  map.image_dpi = dpi;

  // factor (map coordinates (m))/(map point)
  double k = 100000.0 * 25.4e-3/*m/in*/ /mag / map.image_dpi;
  cnv.rescale_src(k); // now cnv1: map points -> meters

  // image size (rounded)
  iRect image_bbox = rint(R/k);

  // Border in map points (exact)
  dLine brd = rect_to_line(R/k);

  // Refpoints:
  dLine pts_r = rect_to_line(R/k, false);
  dLine pts_w = pts_r;
  cnv.frw(pts_w);  // map points -> wgs

  // margins
  image_bbox = iRect(image_bbox.x-ml, image_bbox.y-mb,
                     image_bbox.w+ml+mr, image_bbox.h+mt+mb);

  brd -= image_bbox.tlc();
  pts_r -= image_bbox.tlc();
  brd.flip_y(image_bbox.h);
  pts_r.flip_y(image_bbox.h);
  map.image_size = iPoint(image_bbox.w, image_bbox.h);

  // Add map border:
  map.border.push_back(brd);

  // Add refpoints:
  map.add_ref(pts_r, pts_w);
  return map;
}

GeoMap
geo_mkref_tiles(const iRect & tile_range, int z, bool G,
                const dMultiLine & border, double mag){

  GeoMap map;
  GeoTiles tcalc;
  map.name = type_to_str(tile_range);
  map.proj = "WEB";
  map.border = border;

  // find coordinates of opposite corners:
  dPoint tlc = G ? tcalc.gtile_to_range(tile_range.blc(),z).blc():
                   tcalc.tile_to_range(tile_range.tlc(),z).tlc();
  dPoint brc = G ? tcalc.gtile_to_range(tile_range.trc(),z).blc():
                   tcalc.tile_to_range(tile_range.brc(),z).tlc();

  // Refpoints. Corners correspond to points 0,image_size.
  // Should it be -0.5 .. image_size-0.5
  map.image_size = iPoint(tile_range.w, tile_range.h)*tcalc.get_tsize() * mag;
  dLine pts_w = rect_to_line(dRect(tlc,brc), false);
  dLine pts_r = rect_to_line(dRect(dPoint(0,0),map.image_size), false);

  pts_r.flip_y(map.image_size.y);
  map.add_ref(pts_r, pts_w);

  if (map.border.size()!=0){
    ConvMap cnv(map);
    map.border = cnv.bck_acc(map.border);
  }
  else
    map.border.push_back(pts_r);
  return map;
}

/********************************************************************/

GeoMap
geo_mkref_opts(const Opt & o){

  if (!o.exists("mkref")) return GeoMap();
  string reftype = o.get("mkref","");

  auto name = o.get("name",string());
  auto dpi  = o.get<double>("dpi",300.0);
  auto mag  = o.get<double>("mag",1.0);
  bool north = o.exists("north");

  // margins
  int mt,ml,mr,mb;
  mt=ml=mr=mb=o.get("margins", 0);
  mt=o.get("top_margin", mt);
  ml=o.get("left_margin", ml);
  mr=o.get("right_margin", mr);
  mb=o.get("bottom_margin", mb);

  /***************************************/
  if (reftype == "nom"){
    if (name == "") throw Err() <<
      "geo_mkref: nomenclature name should be set (name option)";
    return geo_mkref_nom(name, dpi, mag, north, mt, ml, mr, mb);
  }

  /***************************************/
  else if (reftype == "nom_fi"){
    if (name == "") throw Err() <<
      "geo_mkref: nomenclature name should be set (name option)";
    return geo_mkref_nom_fi(name, dpi, mag, mt, ml, mr, mb);
  }

  /***************************************/
  else if (reftype == "tms_tile" || reftype == "google_tile"){

    GeoTiles tcalc;
    bool G = (reftype == "google_tile");
    int  z = o.get("zindex",-1);
    list<string> confl = {"tiles", "coords"};
    o.check_conflict(confl);

    // get tile range
    iRect tile_range;

    if (o.exists("tiles")){
      try {
        iPoint p = o.get("tiles", iPoint());
        if (p.z>0) z = p.z;
        tile_range = iRect(p, p+iPoint(1,1));
      }
      catch (Err & e){
        tile_range = o.get("tiles", iRect());
      }

      if (tile_range.is_zsize())
        throw Err() << "geo_mkref: empty tile range: " << o.get("tiles", string());

    }

    dMultiLine border;
    o.check_conflict({"coords_wgs", "coords_file", "coord_nom"});

    if (o.exists("coords_wgs")){
      dRect r = figure_bbox<double>(o.get("coords_wgs",""));
      if (G) tile_range = tcalc.range_to_gtiles(r, z);
      else   tile_range = tcalc.range_to_tiles(r, z);
    }
    if (o.exists("coords_file")){
      dRect r = read_bbox(o.get("coords_file",""));
      if (G) tile_range = tcalc.range_to_gtiles(r, z);
      else   tile_range = tcalc.range_to_tiles(r, z);
    }
    if (o.exists("coords_nom")){
      nom_scale_t sc;
      ConvGeo cnv2("SU_LL");
      dRect r = nom_to_range(o.get("coords_nom",""), sc, true);
      if (G) tile_range = tcalc.range_to_gtiles(cnv2.frw_acc(r), z);
      else   tile_range = tcalc.range_to_tiles(cnv2.frw_acc(r), z);
      border.push_back(cnv2.frw_acc(rect_to_line(r)));
    }

    // here tile_range should be set to non-zero rectangle
    if (tile_range.is_zsize())
      throw Err() << "geo_mkref: empty tile range, use coords_* or tiles options";

    // z-index should be set here
    if (z<0) throw Err() << "geo_mkref: z-index not set";

    return geo_mkref_tiles(tile_range, z, G, border, mag);
  }

  /***************************************/
  else if (reftype == "proj"){
    GeoMap map;

    // map projection
    map.proj = o.get("proj", "WGS");
    if (map.proj == "") throw Err() << "Option --proj is not set";

    // try to build conversion, proj -> wgs84
    ConvGeo cnv(map.proj);

    // map resolution
    double scale = ConvGeo::is_deg(map.proj)? 0.01:1000;
    scale = o.get("scale",scale);
    map.image_dpi = o.get("dpi",300);

    // factor (map coordinates)/(pixels)
    double k = scale * 2.54/ map.image_dpi;
    cnv.rescale_src(k); // now cnv1: map pixels -> wgs84

    // get bounding box (in map pixels)
    dRect range;

    o.check_conflict({"coords", "coords_wgs", "coords_file", "coords_nom"});
    if (o.exists("coords"))
      range = figure_bbox<double>(o.get("coords",""))/k;
    if (o.exists("coords_wgs"))
        range = cnv.bck_acc(
          figure_line<double>(o.get("coords_wgs",""))).bbox();
    if (o.exists("coords_file"))
        range = cnv.bck_acc(
          read_bbox(o.get("coords_file","")));
    if (o.exists("coords_nom")){
      nom_scale_t sc;
      ConvGeo cnv2("SU_LL");
      dRect r = nom_to_range(o.get("coords_nom",""), sc, true);
      range = cnv2.frw_acc(r);
      range = cnv.bck_acc(range);
      map.border.push_back(cnv2.frw_acc(rect_to_line(r)));
      cnv.bck_acc(map.border);
    }

    // check if range is set
    if (range.is_zsize())
      throw Err() << "geo_mkref: empty coordinate range";

    // expand range to closiest integers
    range.to_ceil();

    // set margins
    range = dRect(range.x-ml, range.y-mb,
                  range.w+ml+mr, range.h+mt+mb);

    // Refpoints
    dLine pts_r = rect_to_line(range, false);
    dLine pts_w(pts_r);
    pts_r -= range.tlc();
    pts_r.flip_y(range.h);
    cnv.frw(pts_w);
    map.add_ref(pts_r, pts_w);

    // Border in proj coordinates
    if (o.exists("border")){
      dMultiLine brd = o.get("border", dMultiLine())/k;
      map.border = rint(brd - range.tlc());
      map.border.flip_y(range.h);
    }
    else if (map.border.size()==0){
      map.border.push_back(pts_r);
    }

    // image_size
    map.image_size = dPoint(range.w, range.h);
    return map;
  }

  else if (reftype == "file"){
    std::string name = o.get("name", "");
    if (name == "") throw Err() << "Option --name is not set for --mkref=file";
    GeoData d;
    read_geo(name, d);
    if (d.maps.size()<1 || d.maps.begin()->size()<1) throw Err()
      << "mkref: can't read any map reference from file: " << name;
    return (*d.maps.begin())[0];
  }

  else {
    throw Err() << "geo_mkref: unknown reference type: " << reftype;
  }

  return GeoMap();
}


// update map border from options
void
geo_mkref_brd(GeoMap & ref, const Opt & o){
  o.check_conflict({"border_wgs", "border_file"});

  if (o.exists("border_wgs")){
    ref.border = o.get<dMultiLine>("border_wgs");
    if (!ref.empty()){
      ConvMap cnv(ref);
      ref.border = cnv.bck_acc(ref.border);
    }
    return;
  }

  if (o.exists("border_file")){
    ref.border = read_geoline(o.get("border_file"));
    if (!ref.empty()){
      ConvMap cnv(ref);
      ref.border = cnv.bck_acc(ref.border);
    }
    return;
  }

  return;
}

// try to get some information from GeoData if there is
// no "mkref" option.
GeoMap geo_mkref_data(const GeoData & data, const Opt & o){

  // If there is at list one map - use its reference.
  // This map will have the best quality.
  if (data.maps.size()>0 && data.maps.begin()->size()>0){
    GeoMap map = *(data.maps.begin()->begin());
    map.update_size();

    // apply --mag option
    if (o.exists("mag")) map*=o.get<double>("mag", 1.0);

    // join borders and bboxes of all maps (in wgs coords):
    map.border = dMultiLine();
    dRect bbox;
    for (auto const & ml:data.maps){
      for (auto const & m:ml){
        ConvMap cnv(m);
        auto b = cnv.frw_acc(m.border);
        map.border.insert(map.border.end(), b.begin(), b.end());
        bbox.expand(cnv.frw_acc(m.bbox()));
      }
    }
    // convert to the map reference
    ConvMap cnv(map);
    map.border = cnv.bck_acc(map.border);
    bbox = cnv.bck_acc(bbox);

    // shift map reference to include bbox, set image size
    map -= bbox.tlc();
    map.image_size = bbox.brc()-bbox.tlc();
    return map;
  }

  // Here we should have something smart: smaller scales
  // for large areas, different projections, etc.
  Opt opts(o);
  opts.put("mkref", "proj");

  // data bounding box
  dRect bbox;
  for (auto const & t:data.trks) bbox.expand(t.bbox());
  for (auto const & w:data.wpts) bbox.expand(w.bbox());

  if (bbox.is_empty()) return GeoMap();

  if (!opts.exists("coords") &&
      !opts.exists("coords_wgs") &&
      !opts.exists("coords_file"))
    opts.put("coords_wgs", bbox);

  return geo_mkref_opts(opts);
}

GeoMap
geo_mkref_web(){
  GeoMap map;
  // max latitude is set to have square map. cnv(mlon,mlat) = (1,1)*20037508.342789244
  double mlat = 85.0511288, mlon=180.0;
  double wr = 256;
  map.image_size = iPoint(wr,wr);
  map.proj = "WEB";
  map.name = "default";
  map.ref.emplace(dPoint( 0, 0),dPoint(-mlon, mlat));
  map.ref.emplace(dPoint(wr, 0),dPoint( mlon, mlat));
  map.ref.emplace(dPoint(wr,wr),dPoint( mlon, -mlat));
  map.ref.emplace(dPoint( 0,wr),dPoint(-mlon, -mlat));
  return map;
}
