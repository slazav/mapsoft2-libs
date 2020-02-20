#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>

#include "vmap/vmap.h"
#include "read_words/read_words.h"
#include "geom/poly_tools.h"

#include "mapdb.h"

/**********************************************************/
/* Import/export of old mapsoft VMAP format */

using namespace std;

/**********************************************************/
void
ms2opt_add_mapdb_vmap_imp(GetOptSet & opts){
  const char *g = "MAPDB_VMAP_IMP";
  opts.add("config", 1,'c',g,
    "Configuration file for VMAP->MapDB conversion");
}

/**********************************************************/
void
ms2opt_add_mapdb_vmap_exp(GetOptSet & opts){
  const char *g = "MAPDB_VMAP_EXP";
  opts.add("config", 1,'c',g,
    "Configuration file for MapDB->VMAP conversion");
}


/**********************************************************/
/// Import objects from VMAP file.
void
MapDB::import_vmap(const std::string & vmap_file, const Opt & opts){

  // type conversion tables (point, line, polygon)
  std::map<uint32_t, uint32_t> obj_map; // vmap type -> mapdb type
  std::map<uint32_t, uint32_t> lbl_map; // vmap type -> mapdb type

  // If configuration file does not exist we will
  // use some default conversion.
  std::string cfg_file = opts.get<string>("config", "");

  // Read configuration file.
  if (cfg_file != ""){

    int line_num[2] = {0,0};
    ifstream ff(cfg_file);
    try {
      while (1){
        vector<string> vs = read_words(ff, line_num, true);
        if (vs.size()<1) break;

        // add other configuration commands here...

        // object conversion (src, dst, [label])
        if (vs.size() == 2 || vs.size() == 3) {
          uint32_t src_type = MapDBObj::make_type(vs[0]);
          uint32_t dst_type = MapDBObj::make_type(vs[1]);
          MapDBObjClass scl = (MapDBObjClass)(src_type>>24);
          MapDBObjClass dcl = (MapDBObjClass)(dst_type>>24);

          if (scl != MAPDB_POINT &&
              scl != MAPDB_LINE  &&
              scl != MAPDB_POLYGON )
            throw Err() << "point, line, or area object type expected"
                           " in the first column: " << vs[0];

          if (dcl != MAPDB_POINT &&
              dcl != MAPDB_LINE  &&
              dcl != MAPDB_POLYGON )
            throw Err() << "point, line, or area object type expected"
                           " in the second column: " << vs[1];

          if ((scl == MAPDB_POINT) &&
              (dcl == MAPDB_LINE  ||
               dcl == MAPDB_POLYGON) )
            throw Err() << "can't convert point to line or area: "
                           << vs[0] << " -> " << vs[1];

          if ((dcl == MAPDB_POINT) &&
              (scl == MAPDB_LINE  ||
               scl == MAPDB_POLYGON) )
            throw Err() << "can't convert line or area to point: "
                           << vs[0] << " -> " << vs[1];

           obj_map.insert(make_pair(src_type, dst_type));
          if (vs.size() == 3){
            uint32_t lbl_type = MapDBObj::make_type(vs[2]);
            if ((lbl_type>>24) != MAPDB_TEXT)
              throw Err() << "text object type expected"
                             " in the third column: " << vs[2];
            lbl_map.insert(make_pair(src_type, lbl_type));
          }
          continue;
        }

        throw Err() << "unknown command: " << vs[0];
      }
    } catch (Err e){
        throw Err() << cfg_file << ":"
                    << line_num[0] << ": " << e.str();
    }
  }

  // read VMAP file
  ifstream in(vmap_file);
  VMap vmap_data = read_vmap(in);
  for (auto const & o:vmap_data){

    // skip empty objects
    if (o.is_empty()) continue;

    // get type in MapDB format
    uint16_t cl = o.type >> 20;
    if (cl>2) throw Err() << "bad object class: " << cl;
    uint16_t tnum = o.type & 0xFFFF;
    uint32_t type = (cl<<24) + tnum;

    // convert type
    if (cfg_file != "") {
      if (obj_map.count(type)==0) continue;
      type = obj_map.find(type)->second;
    }

    // make object
    MapDBObj o1(type);

    // name and comments
    o1.name = o.text;
    o1.comm = o.comm;

    // source
    if (o.opts.exists("Source")) o1.tags.insert(o.opts.get<string>("Source"));

    // angle (deg -> deg)
    // In my vmap maps angle has a strange meaning: atan2(dlat,dlon)
    // (lonlat coordinates have different scales in horizontal and vertical directions)
    // We want to convert the angle to real angle from north direction
    // We can do it only if we know coordinates (object has at least one point).
    // Direction of angle is also different (ccw instead of cw)
    if (o.opts.exists("Angle")){
      o1.angle=-o.opts.get<float>("Angle");
      if (o.size()>0 && o[0].size()>0){
        dPoint pt = o.bbox().cnt();
        o1.angle = 180/M_PI*atan2(sin(M_PI/180*o1.angle), cos(M_PI/180*pt.y)*cos(M_PI/180*o1.angle));
      }
    }

    // set coordinates
    o1.dMultiLine::operator=(o);

    // labels
    for (auto const & l:o.labels){

      uint32_t ltype = (MAPDB_TEXT<<24) + 1;
      if (cfg_file != "") {
        if (lbl_map.count(type)==0) continue;
        ltype = lbl_map.find(type)->second;
      }

      MapDBObj l1(ltype);
      l1.name = o.text;

      // angle: same conversion as above
      if (l.hor){
        l1.angle = std::nan("");
      }
      else {
        l1.angle = -l.ang;
        l1.angle = 180/M_PI*atan2(sin(M_PI/180*l1.angle), cos(M_PI/180*l.pos.y)*cos(M_PI/180*l1.angle));
      }

      dLine pts; pts.push_back(l.pos);
      l1.push_back(pts);
      switch (l.dir){
        case 0: l1.align = MAPDB_ALIGN_SW; break;
        case 1: l1.align = MAPDB_ALIGN_S; break;
        case 2: l1.align = MAPDB_ALIGN_SE; break;
      }
      // todo: l.fsize ?

      // add label and put its ID to object's children:
      o1.children.insert(add(l1));
    }

    // add object
    add(o1);

  }

  // border
  dMultiLine brd;
  brd.push_back(vmap_data.brd);
  set_map_brd(brd);

  // map name
  set_map_name(vmap_data.name);

}

/**********************************************************/
/// Export objects to VMAP file.
void
MapDB::export_vmap(const std::string & vmap_file, const Opt & opts){

  // type conversion tables (point, line, polygon)
  std::map<uint32_t, uint32_t> obj_map; // mapdb type -> vmap type

  // If configuration file does not exist we will
  // use some default conversion.
  std::string cfg_file = opts.get<string>("config", "");

  // Read configuration file.
  if (cfg_file != ""){

    ifstream ff(cfg_file);
    int line_num[2] = {0,0};

    try {
      while (1){
        vector<string> vs = read_words(ff, line_num, true);
        if (vs.size()<1) break;

        // add other configuration commands here...

        // object conversion (src, dst)
        if (vs.size() == 2) {
          uint32_t src_type = MapDBObj::make_type(vs[0]);
          uint32_t dst_type = MapDBObj::make_type(vs[1]);
          MapDBObjClass scl = (MapDBObjClass)(src_type>>24);
          MapDBObjClass dcl = (MapDBObjClass)(dst_type>>24);

          if (scl != MAPDB_POINT   && scl != MAPDB_LINE &&
              scl != MAPDB_POLYGON && scl != MAPDB_TEXT)
            throw Err() << "point, line, area, or text object type expected"
                           " in the first column: " << vs[0];

          if (dcl != MAPDB_POINT   && dcl != MAPDB_LINE &&
              dcl != MAPDB_POLYGON)
            throw Err() << "point, line, area object type expected"
                           " in the second column: " << vs[1];

          if ((scl == MAPDB_POINT || scl == MAPDB_TEXT) &&
              (dcl == MAPDB_LINE  || dcl == MAPDB_POLYGON) )
            throw Err() << "can't convert point or text to line or area: "
                           << vs[0] << " -> " << vs[1];

          if ((dcl == MAPDB_POINT || dcl == MAPDB_TEXT) &&
              (scl == MAPDB_LINE  || scl == MAPDB_POLYGON) )
            throw Err() << "can't convert line or area to point or text: "
                           << vs[0] << " -> " << vs[1];

          obj_map.insert(make_pair(src_type, dst_type));
          continue;
        }

        throw Err() << "unknown command: " << vs[0];
      }
    } catch (Err e){
        throw Err() << cfg_file << ":"
                    << line_num[0] << ": " << e.str();
    }
  }

  VMap vmap_data;
  uint32_t key = 0;
  std::string str = objects.get_first(key);

  while (key!=0xFFFFFFFF){
    MapDBObj o;
    o.unpack(str);

    VMapObj o1;

    // convert type
    uint32_t type = o.type;
    if (cfg_file != "") {
      if (obj_map.count(type)==0) goto end;
      type = obj_map.find(type)->second;
    }

    // convert type to vmap format
    switch (type>>24){
      case MAPDB_POINT:   o1.type = (type & 0xFFFF); break;
      case MAPDB_LINE:    o1.type = (type & 0xFFFF) | 0x100000; break;
      case MAPDB_POLYGON: o1.type = (type & 0xFFFF) | 0x200000; break;
      default:  goto end;
    }

    // name
    o1.text = o.name;
    o1.comm = o.comm;

    // source
    if (o.tags.size()>0) o1.opts.put("Source", *o.tags.begin());

    // angle (deg->deg)
    // See note in vmap_import
    if (!std::isnan(o.angle)){
      double a = -o.angle;
      if (o.size()>0 && o[0].size()>0){
        dPoint pt = o.bbox().cnt();
        a = 180/M_PI*atan2(cos(M_PI/180*pt.y)*sin(M_PI/180*a), cos(M_PI/180*a));
      }
      o1.opts.put("Angle", a);
    }

    // points
    o1.dMultiLine::operator=(o);

    // labels
    for (auto const & li:o.children){
      auto l = get(1);
      VMapLab l1;
      if (l.get_class() != MAPDB_TEXT) continue;
      if (l.size()==0 || l[0].size()==0) continue;
      l1.pos = l[0][0];
      l1.ang = std::isnan(l.angle)? 0:l.angle;
      l1.hor = std::isnan(l.angle);
      switch (l.align) {
        case MAPDB_ALIGN_NE:
        case MAPDB_ALIGN_E:
        case MAPDB_ALIGN_SE:
          l1.dir = 0; break;
        case MAPDB_ALIGN_N:
        case MAPDB_ALIGN_C:
        case MAPDB_ALIGN_S:
          l1.dir = 1; break;
        case MAPDB_ALIGN_NW:
        case MAPDB_ALIGN_W:
        case MAPDB_ALIGN_SW:
          l1.dir = 2; break;
      }
      l1.fsize = 0; // todo: convert scale?
      o1.labels.push_back(l1);
    }
    vmap_data.push_back(o1);

    end:

    // we can't use get_next because cursor is reset by getting labels:
    key++;
    str = objects.get_range(key);
  }

  // map border (convert to single-segment line)
  vmap_data.brd = join_polygons(get_map_brd());

  // map name
  vmap_data.name = get_map_name();

  // write vmap file
  ofstream out(vmap_file);
  write_vmap(out, vmap_data);
}

