#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

#include <jansson.h>

#include "opt/opt.h"
#include "err/err.h"

#include "geo_data.h"


using namespace std;

/* see:
 * http://geojson.org/
 * https://tools.ietf.org/html/rfc7946
 * https://leafletjs.com/examples/geojson/
 */

void
write_json (const string &fname, const GeoData & data, const Opt & opts){

  bool v = opts.get("verbose", false);
  if (v) cerr << "Writing GeoJSON file: " << fname << endl;

  json_t *features = json_array();

  // tracks
  // Each track is a feature with MultiLineString objects.
  // First we write tracks to have them below points on leaflet maps.
  for (auto const & trk: data.trks) {
    if (v) cerr << "  Writing track: " << trk.name
           << " (" << trk.size() << " points)" << endl;

    json_t *j_trk = json_object();
    json_object_set_new(j_trk, "type", json_string("Feature"));

    // name, comment
    if (trk.name != "")
      json_object_set_new(j_trk, "name", json_string(trk.name.c_str()));
    if (trk.comm != "")
      json_object_set_new(j_trk, "comm", json_string(trk.comm.c_str()));

    // properties
    json_t *j_prop = json_object();
    for (auto const & o:trk.opts)
      json_object_set_new(j_prop, o.first.c_str(), json_string(o.second.c_str()));
    if (json_object_size(j_prop))
      json_object_set_new(j_trk, "properties", j_prop);
    else json_decref(j_prop);

    // coordinates
    json_t *j_crd = json_array();
    json_t *j_seg = json_array();
    for (auto const & tp: trk) {
      if (tp.start && json_array_size(j_seg)){
        json_array_append_new(j_crd, j_seg);
        j_seg = json_array();
      }
      json_t *j_pt = json_array();
      json_array_append_new(j_pt, json_real(tp.x));
      json_array_append_new(j_pt, json_real(tp.y));
      if (tp.t!=0 || tp.have_alt()) {
        json_array_append_new(j_pt, tp.have_alt()? json_real(tp.z) : json_null());
        if (tp.t!=0) json_array_append_new(j_pt, json_integer(tp.t));
      }
      json_array_append_new(j_seg, j_pt);
    }
    if (json_array_size(j_seg)) json_array_append_new(j_crd, j_seg);

    json_t *j_geom = json_object();
    json_object_set_new(j_geom, "type", json_string("MultiLineString"));
    json_object_set_new(j_geom, "coordinates", j_crd);
    json_object_set_new(j_trk, "geometry", j_geom);

    json_array_append_new(features, j_trk);
  }

  // waypoint lists
  for (auto const & wpl: data.wpts) {
    if (v) cerr << "  Writing waypoints: " << wpl.name
           << " (" << wpl.size() << " points)" << endl;

    json_t *j_wptl = json_object();
    json_object_set_new(j_wptl, "type", json_string("FeatureCollection"));

    // name, comment
    if (wpl.name != "")
      json_object_set_new(j_wptl, "name", json_string(wpl.name.c_str()));
    if (wpl.comm != "")
      json_object_set_new(j_wptl, "comm", json_string(wpl.comm.c_str()));

    // properties
    json_t *j_prop = json_object();
    for (auto const & o:wpl.opts)
      json_object_set_new(j_prop, o.first.c_str(), json_string(o.second.c_str()));
    if (json_object_size(j_prop))
      json_object_set_new(j_wptl, "properties", j_prop);
    else json_decref(j_prop);

    // waypoints
    json_t *j_wpts = json_array();
    for (auto const & wp: wpl ) {

      // waypoint
      json_t *j_wpt = json_object();
      json_object_set_new(j_wpt, "type", json_string("Feature"));

      // coordinate array
      json_t *j_pt = json_array();
      json_array_append_new(j_pt, json_real(wp.x));
      json_array_append_new(j_pt, json_real(wp.y));
      if (wp.t!=0 || wp.have_alt()) {
        json_array_append_new(j_pt, wp.have_alt()? json_real(wp.z) : json_null());
        if (wp.t!=0) json_array_append_new(j_pt, json_integer(wp.t));
      }

      // geometry
      json_t *j_geom = json_object();
      json_object_set_new(j_geom, "type", json_string("Point"));
      json_object_set_new(j_geom, "coordinates", j_pt);
      json_object_set_new(j_wpt, "geometry", j_geom);

      // name, comment
      if (wp.name != "")
        json_object_set_new(j_wpt, "name", json_string(wp.name.c_str()));
      if (wp.comm != "")
        json_object_set_new(j_wpt, "comm", json_string(wp.comm.c_str()));

      // properties
      json_t *j_prop = json_object();
      for (auto o:wp.opts)
        json_object_set_new(j_prop, o.first.c_str(), json_string(o.second.c_str()));
      if (json_object_size(j_prop))
        json_object_set_new(j_wpt, "properties", j_prop);
      else json_decref(j_prop);

      json_array_append_new(j_wpts, j_wpt);
    }
    json_object_set_new(j_wptl, "features", j_wpts);

    json_array_append_new(features, j_wptl);
  }


  // map lists -- FeatureCollection with ms2maps array
  for (auto const & mapl: data.maps) {
    if (v) cerr << "  Writing map list: " << mapl.name
           << " (" << mapl.size() << " maps)" << endl;

    // maps
    json_t *j_maps = json_array();
    for (auto const & m: mapl ) {

      json_t *j_map = json_object();

      // reference points (skip z coord)
      json_t *j_ref = json_array();
      for (auto const & r: m.ref) {
        json_t *j_pt = json_array();
        json_array_append_new(j_pt, json_real(r.first.x));
        json_array_append_new(j_pt, json_real(r.first.y));
        json_array_append_new(j_pt, json_real(r.second.x));
        json_array_append_new(j_pt, json_real(r.second.y));
        json_array_append_new(j_ref, j_pt);
      }
      if (json_array_size(j_ref)) json_object_set_new(j_map, "ref", j_ref);
      else json_decref(j_ref);

      // border (multi-segment line, skip z coord)
      json_t *j_brd = json_array();
      for (auto const & seg: m.border) {
        json_t *j_seg = json_array();
        for (auto const & p: seg) {
          json_t *j_pt = json_array();
          json_array_append_new(j_pt, json_real(p.x));
          json_array_append_new(j_pt, json_real(p.y));
          json_array_append_new(j_seg, j_pt);
        }
        if (json_array_size(j_seg)) json_array_append_new(j_brd, j_seg);
        else json_decref(j_seg);
      }
      if (json_array_size(j_brd)) json_object_set_new(j_map, "brd", j_brd);
      else json_decref(j_brd);

      // image_size (skip z coord)
      if (m.image_size.x!=0 && m.image_size.y!=0){
        json_t *j_pt = json_array();
        json_array_append_new(j_pt, json_real(m.image_size.x));
        json_array_append_new(j_pt, json_real(m.image_size.y));
        json_object_set_new(j_map, "image_size", j_pt);
      }

      // other fields (skip defaults)
      GeoMap mdef;
      if (m.proj != "") json_object_set_new(j_map, "proj",  json_string(m.proj.c_str()));
      if (m.name != "") json_object_set_new(j_map, "name",  json_string(m.name.c_str()));
      if (m.comm != "") json_object_set_new(j_map, "comm",  json_string(m.comm.c_str()));
      if (m.image != "") json_object_set_new(j_map, "image", json_string(m.image.c_str()));

      if (m.image_dpi!=mdef.image_dpi)   json_object_set_new(j_map, "image_dpi",  json_real(m.image_dpi));
      if (m.tile_size!=mdef.tile_size)   json_object_set_new(j_map, "tile_size",  json_integer(m.tile_size));
      if (m.tile_swapy!=mdef.tile_swapy) json_object_set_new(j_map, "tile_swapy", json_integer(m.tile_swapy));
      if (m.is_tiled!=mdef.is_tiled)     json_object_set_new(j_map, "is_tiled",   json_integer(m.is_tiled));
      if (m.tile_minz!=mdef.tile_minz)   json_object_set_new(j_map, "tile_minz",  json_integer(m.tile_minz));
      if (m.tile_maxz!=mdef.tile_maxz)   json_object_set_new(j_map, "tile_maxz",  json_integer(m.tile_maxz));

      json_array_append_new(j_maps, j_map);
    }

    json_t *j_mapl = json_object();
    json_object_set_new(j_mapl, "type", json_string("FeatureCollection"));
    if (mapl.name != "") json_object_set_new(j_mapl, "ms2maps_name",  json_string(mapl.name.c_str()));
    if (mapl.comm != "") json_object_set_new(j_mapl, "ms2maps_comm",  json_string(mapl.comm.c_str()));

    // properties
    json_t *j_prop = json_object();
    for (auto const & o:mapl.opts)
      json_object_set_new(j_prop, o.first.c_str(), json_string(o.second.c_str()));
    if (json_object_size(j_prop)) json_object_set_new(j_mapl, "ms2maps_properties", j_prop);
    else json_decref(j_prop);

    json_object_set_new(j_mapl, "ms2maps", j_maps);
    json_array_append_new(features, j_mapl);
  }


  json_t *J0 = json_object();
  json_object_set_new(J0, "type", json_string("FeatureCollection"));
  json_object_set(J0, "features", features);

  int flags = JSON_REAL_PRECISION(10);
  if (opts.get("json_sort_keys", 1)) flags |= JSON_SORT_KEYS;
  if (opts.get("json_compact", 1)) flags |= JSON_COMPACT;
  if (opts.get("json_indent", 0)) flags |= JSON_INDENT(2);

  char *ret = json_dumps(J0, flags);
  json_decref(J0);
  if (!ret) throw Err() << "Can't write data";

  ofstream f(fname);
  if (!f.good()) throw Err() << "Can't open file " << fname << " for writing";
  f<<ret;
  if (!f.good()) throw Err() << "Can't write to file " << fname;
}

/**************************************************************************/
// Some useful functions. Extract string/double/int from json or
// from json object element. Throw error if needed

// Get text value from from object element.
std::string read_json_text_field(json_t *jobj, const char * key){
  json_t *j = json_object_get(jobj, key);
  if (!j || json_is_null(j)) return std::string();
  if (!json_is_string(j)) throw Err() << key << ": JSON string expected";
  return json_string_value(j);
}

// Get real value from json (any number or string is supported)
double read_json_real(json_t *j, const char * name){
  if (j && json_is_number(j))
    return json_number_value(j);
  else if (j && json_is_string(j))
    return str_to_type<double>(json_string_value(j));
  else throw Err() << name << ": JSON number expected";
}

// Get double value from object element.
// Do nothing is element is missing.
void read_json_real_field(json_t *jobj, const char * key, double * val){
  json_t *j = json_object_get(jobj, key);
  if (!j || json_is_null(j)) return;
  *val = read_json_real(j, key);
}

// Get double value from array element.
// Do nothing is element is missing.
void read_json_real_arr(json_t *jarr, size_t i, double * val){
  json_t *j = json_array_get(jarr, i);
  if (!j || json_is_null(j)) return;
  *val = read_json_real(j, "");
}


// Get integer value from json (multiple integer types are supported).
template <typename T>
T read_json_int(json_t *j, const char * name){
  if (j && json_is_integer(j))
    return json_integer_value(j);
  if (j && json_is_string(j))
    return str_to_type<T>(json_string_value(j));
  throw Err() << name << ": JSON integer expected";
}

// Get integer value from object element.
// Do nothing is element is missing.
template <typename T>
void read_json_int_field(json_t *jobj, const char * key, T * val){
  json_t *j = json_object_get(jobj, key);
  if (!j || json_is_null(j)) return;
  *val = read_json_int<T>(j, key);
}

// Get integer value from array element.
// Do nothing is element is missing.
template <typename T>
void read_json_int_arr(json_t *jarr, size_t i, T * val){
  json_t *j = json_array_get(jarr, i);
  if (!j || json_is_null(j)) return;
  *val = read_json_real(j, "");
}


// get bool value
bool read_json_bool(json_t *j, const char * name){
  if (j && json_is_integer(j))
    return json_integer_value(j);
  if (j && json_is_string(j))
    return str_to_type<bool>(json_string_value(j));
  if (j && json_is_true(j))
    return true;
  if (j && json_is_false(j))
    return false;
  throw Err() << name << ": JSON integer expected";
}

void read_json_bool_field(json_t *jobj, const char * key, bool * val){
  json_t *j = json_object_get(jobj, key);
  if (!j || json_is_null(j)) return;
  *val = read_json_bool(j, key);
}

// Get options from object element.
Opt read_json_opt_field(json_t *jobj, const char * key){
  json_t *j = json_object_get(jobj, key);
  Opt ret;
  if (!j || json_is_null(j)) return ret;
  if (!json_is_object(j)) throw Err() << key << ": JSON object expected";
  const char *k;
  json_t *v;
  json_object_foreach(j, k, v) {
    if (json_is_string(v))
      ret.put(k, string(json_string_value(v)));
    else if (json_is_integer(v))
      ret.put(k, json_integer_value(v));
    else if (json_is_real(v))
      ret.put(k, json_real_value(v));
    else if (json_is_boolean(v))
      ret.put(k, json_is_true(v));
    else throw Err() << key << ": unsupported JSON type for " << k;
  }
  return ret;
}

/**************************************************************************/
// read a single point (x,y,t,z) from a JSON array
template <typename T>
void read_geojson_pt(json_t *coord, T & pt){
  read_json_real_arr(coord, 0, &pt.x);
  read_json_real_arr(coord, 1, &pt.y);
  read_json_real_arr(coord, 2, &pt.z);
  read_json_int_arr(coord,  3, &pt.t);
}


// construct a track from GeoJSON coordinates and properties
GeoTrk read_geojson_trk(json_t *coord, json_t *prop, const bool multi){
  GeoTrk ret;
  // set properties
  const char *key;
  json_t *val;
  json_object_foreach(prop, key, val) {
    if (!json_is_string(val)) continue;
    else if (strcasecmp(key, "name")==0) ret.name = json_string_value(val);
    else if (strcasecmp(key, "comm")==0) ret.comm = json_string_value(val);
    else ret.opts.put(key, string(json_string_value(val)));
  }
  // coordinates
  size_t i;
  json_t *c1;
  json_array_foreach(coord, i, c1) {
    if (multi){
      size_t j;
      json_t *c2;
      json_array_foreach(c1, j, c2) {
        GeoTpt pt;
        if (j==0) pt.start=1;
        read_geojson_pt(c2, pt);
        ret.push_back(pt);
      }
    }
    else {
      GeoTpt pt;
      if (i==0) pt.start=1;
      read_geojson_pt(c1, pt);
      ret.push_back(pt);
    }
  }
  return ret;
}


// construct map list from GeoJSON
GeoMap read_geojson_map(json_t *json){
  if (!json_is_object(json))
     throw Err() << "ms2maps: object expected";
  GeoMap ret;

  ret.name  = read_json_text_field(json, "name");
  ret.comm  = read_json_text_field(json, "comm");
  ret.proj  = read_json_text_field(json, "proj");
  ret.image = read_json_text_field(json, "image");

  read_json_real_field(json, "image_dpi",   &ret.image_dpi);
  read_json_int_field(json,  "tile_size",   &ret.tile_size);
  read_json_bool_field(json, "tile_swapy",  &ret.tile_swapy);
  read_json_bool_field(json, "is_tiled",    &ret.is_tiled);
  read_json_int_field(json,  "tile_minz",   &ret.tile_minz);
  read_json_int_field(json,  "tile_maxz",   &ret.tile_maxz);

  // ref
  json_t *j;
  j = json_object_get(json, "ref");
  if (j && !json_is_null(j)){
    if (!json_is_array(j)) throw Err() << "ref: JSON array expected";
    size_t pti;
    json_t *pt;
    json_array_foreach(j, pti, pt) {
      if (!json_is_array(pt) || json_array_size(pt)<4)
        throw Err() << "ref point: array with at least 4 numbers expected";
      dPoint pr, pg;
      read_json_real_arr(pt, 0, &pr.x);
      read_json_real_arr(pt, 1, &pr.y);
      read_json_real_arr(pt, 2, &pg.x);
      read_json_real_arr(pt, 3, &pg.y);
      ret.ref.emplace(pr,pg);
    }
  }

  // image_size
  j = json_object_get(json, "image_size");
  if (j && !json_is_null(j)){
    if (!json_is_array(j) || json_array_size(j)<2)
       throw Err() << "image_size: array with at least 2 values expected";
    read_json_int_arr(j, 0, &ret.image_size.x);
    read_json_int_arr(j, 1, &ret.image_size.y);
  }

  // border
  j = json_object_get(json, "brd");
  if (j && !json_is_null(j)){
    if (!json_is_array(j)) throw Err() << "brd: JSON array of border segments expected";
    size_t segi;
    json_t *seg;
    json_array_foreach(j, segi, seg) {
      if (!json_is_array(seg)) throw Err() << "brd segment: JSON array of points expected";
      dLine brd_seg;
      size_t pti;
      json_t *pt;
      json_array_foreach(seg, pti, pt) {
        if (!json_is_array(pt) || json_array_size(pt)<2)
          throw Err() << "brd point: array with at least 2 numbers expected";
        dPoint p;
        read_json_real_arr(pt, 0, &p.x);
        read_json_real_arr(pt, 1, &p.y);
        brd_seg.push_back(p);
      }
      ret.border.push_back(brd_seg);
    }
  }

  return ret;
}


// read GeoJSON Feature of FeatureCollection (recursively)
void
read_geojson_feature(json_t *feature, GeoData & data,
                     GeoWptList & wptl, const bool v){
    if (!json_is_object(feature))
      throw Err() << "JSON object expected";
    json_t *j_type = json_object_get(feature, "type");
    if (!json_is_string(j_type))
      throw Err() << "Wrong/missing type in a GeoJSON object";
    string type = json_string_value(j_type);

    if (type == "FeatureCollection"){

      // always construct a new waypoint list for a FeatureCollection
      GeoWptList wptl1;
      // name and comm
      wptl1.name = read_json_text_field(feature, "name");
      wptl1.comm = read_json_text_field(feature, "comm");
      // properties
      wptl1.opts = read_json_opt_field(feature, "properties");

      // read sub-features (if any)
      json_t *sub_features = json_object_get(feature, "features");
      if (sub_features) {
        if (!json_is_array(sub_features))
          throw Err() << "features: array expected";
        size_t i;
        json_t *sub_feature;
        json_array_foreach(sub_features, i, sub_feature) {
          read_geojson_feature(sub_feature, data, wptl1, v);
        }
      }

      // read map list (if any)
      json_t *mapl = json_object_get(feature, "ms2maps");
      if (mapl) {
        if (!json_is_array(mapl))
          throw Err() << "ms2maps: array expected";
        GeoMapList ret;

        // read name and comm
        ret.name = read_json_text_field(feature, "ms2maps_name");
        ret.comm = read_json_text_field(feature, "ms2maps_comm");
        // read ms2maps_properties
        ret.opts = read_json_opt_field(feature, "ms2maps_properties");

        // read maps
        size_t i;
        json_t *map;
        json_array_foreach(mapl, i, map) 
          ret.push_back(read_geojson_map(map));

        data.maps.push_back(ret);
      }

      // Add waypoint list if it is not empty.
      // Add empty waypoint list if "features" and "ms2maps" objects are missing or
      // fully empty (to tracks, no maps, no other FeatureCollections)
      if (wptl1.size()>0 || (
          json_array_size(sub_features) == 0 &&
          json_array_size(mapl) == 0)) {
        if (v) cerr << "  Reading waypoints: " << wptl1.name
                    << " (" << wptl1.size() << " points)" << endl;
        data.wpts.push_back(wptl1);
      }
      return;
    }

    else if (type == "Feature"){
      // geometry
      json_t * j_geom = json_object_get(feature, "geometry");
      if (!json_is_object(j_geom))
        throw Err() << "Wrong/missing geometry object in a GeoJSON Feature";

      json_t * j_geom_type = json_object_get(j_geom, "type");
      if (!json_is_string(j_geom_type))
        throw Err() << "Wrong/missing type in a GeoJSON geometry";
      string geom_type = json_string_value(j_geom_type);

      json_t * j_geom_coord = json_object_get(j_geom, "coordinates");
      if (!json_is_array(j_geom_coord))
        throw Err() << "Wrong/missing coordinades in a GeoJSON geometry";

      // Waypoint
      if (geom_type == "Point") {
        GeoWpt wpt;
        // name, comm, opts
        wpt.name = read_json_text_field(feature, "name");
        wpt.comm = read_json_text_field(feature, "comm");
        wpt.opts = read_json_opt_field(feature, "properties");

        // coordinates
        read_geojson_pt(j_geom_coord, wpt);
        wptl.push_back(wpt);
      }

      // Track
      else if (geom_type == "MultiLineString" || geom_type == "LineString") {
        GeoTrk trk;
        // name, comm, opts
        trk.name = read_json_text_field(feature, "name");
        trk.comm = read_json_text_field(feature, "comm");
        trk.opts = read_json_opt_field(feature, "properties");

        // coordinates
        size_t i;
        json_t *c1;
        json_array_foreach(j_geom_coord, i, c1) {
          if (geom_type == "MultiLineString"){
            size_t j;
            json_t *c2;
            json_array_foreach(c1, j, c2) {
              GeoTpt pt;
              if (j==0) pt.start=1;
              read_geojson_pt(c2, pt);
              trk.push_back(pt);
            }
          }
          else {
            GeoTpt pt;
            if (i==0) pt.start=1;
            read_geojson_pt(c1, pt);
            trk.push_back(pt);
          }
        }
        if (v) cerr << "  Reading track: " << trk.name
                    << " (" << trk.size() << " points)" << endl;
        data.trks.push_back(trk);
      }
      else
        throw Err() << "Unknown geometry type in a GeoJSON: " << geom_type;
    }
    else throw Err() << "Unknown type in a GeoJSON feature: " << type;
}

void
read_json(const string &fname, GeoData & data, const Opt & opts) {
  bool v = opts.get("verbose", false);
  if (v) cerr << "Reading GeoJSON file: " << fname << endl;

  ifstream f(fname);
  istreambuf_iterator<char> begin(f), end;
  string buf(begin, end);
  if (!f.good()) throw Err() << "Can't read file " << fname;

  size_t flags = 0;
  json_error_t e;
  json_t *J = json_loads(buf.c_str(), flags, &e);
  if (!J) throw Err() << "Can't parse JSON: " << e.text;

  GeoWptList tmp;
  try { read_geojson_feature(J, data, tmp, v); }
  catch(Err & e){
    json_decref(J);
    throw;
  }
  json_decref(J);
}

