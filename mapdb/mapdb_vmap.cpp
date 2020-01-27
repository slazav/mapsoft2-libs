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

  opts.add("cnv_points", 1,0,g,
    "Type conversion rules for point objects: "
    "[[<type_in>, <type_out>], [<type_in>, <type_out>], ...] "
    "type_in==0 matches all types; type_out==0 sets output type "
    "same as input one. Default value: [[0,0]].");

  opts.add("cnv_lines", 1,0,g,
    "Type conversion rules for line objects. Same notation as in cnv_point.");

  opts.add("cnv_areas", 1,0,g,
    "Type conversion rules for area objects. Same notation as in cnv_point.");

}

/**********************************************************/
void
ms2opt_add_mapdb_vmap_exp(GetOptSet & opts){
  const char *g = "MAPDB_VMAP_EXP";
  opts.add("config", 1,'c',g,
    "Configuration file for MapDB->VMAP conversion");

 opts.add("cnv_points", 1,0,g,
    "Type conversion rules for point objects: "
    "[[<type_in>, <type_out>], [<type_in>, <type_out>], ...] "
    "type_in==0 matches all types; type_out==0 sets output type "
    "same as input one. Default value: [[0,0]].");

  opts.add("cnv_lines", 1,0,g,
    "Type conversion rules for line objects. Same notation as in cnv_point.");

  opts.add("cnv_areas", 1,0,g,
    "Type conversion rules for area objects. Same notation as in cnv_point.");
}


/**********************************************************/
/// Import objects from VMAP file.
void
MapDB::import_vmap(const std::string & vmap_file, const Opt & opts){

  // type conversion tables (point, line, polygon)
  vector<iLine> cnvs;
  cnvs.resize(3);
  // default configuration 0->0
  for (int i=0; i<3; i++)
    cnvs[i].push_back(iPoint(0,0));

  // data level
  int level = 0;

  // Read configuration file.
  if (opts.exists("config")){
    int line_num[2] = {0,0};

    // reset default configuration:
    for (int i=0; i<3; i++) cnvs[i] = iLine();


    ifstream ff(opts.get<string>("config"));
    try {
      while (1){
        vector<string> vs = read_words(ff, line_num, true);
        if (vs.size()<1) break;

        if (vs[0]=="point"){
          if (vs.size()!=2 && vs.size()!=3)
            throw Err() << "point: two or three arguments expected: "
                           "<type_in> [<type_out>]";
          cnvs[0].push_back(iPoint(
            str_to_type<int>(vs[1]), vs.size()<3? 0:str_to_type<uint16_t>(vs[2])));
          continue;
        }

        if (vs[0]=="line"){
          if (vs.size()!=2 && vs.size()!=3)
            throw Err() << "line: two or three arguments expected: "
                           "<type_in> [<type_out>]";
          cnvs[1].push_back(iPoint(
            str_to_type<int>(vs[1]), vs.size()<3? 0:str_to_type<uint16_t>(vs[2])));
          continue;
        }

        if (vs[0]=="area"){
          if (vs.size()!=2 && vs.size()!=3)
            throw Err() << "area: two or three arguments expected: "
                           "<type_in> [<type_out>]";
          cnvs[2].push_back(iPoint(
            str_to_type<int>(vs[1]), vs.size()<3? 0:str_to_type<uint16_t>(vs[2])));
          continue;
        }

        throw Err() << "unknown command: " << vs[0];
      }
    } catch (Err e){
        throw Err() << "MapDB::import_mp: bad configuration file at line "
                    << line_num[0] << ": " << e.str();
    }
  }

  if (opts.exists("cnv_points")) cnvs[0] = opts.get<dLine>("cnv_points");
  if (opts.exists("cnv_lines"))  cnvs[1] = opts.get<dLine>("cnv_lines");
  if (opts.exists("cnv_areas"))  cnvs[2] = opts.get<dLine>("cnv_areas");


  // read VMAP file
  ifstream in(vmap_file);
  VMap vmap_data = read_vmap(in);
  for (auto const & o:vmap_data){

    // skip empty objects
    if (o.is_empty()) continue;

    MapDBObj o1;
    int cl = o.type >> 20;
    switch (cl){
      case 0: o1.cl = MAPDB_POINT; break;
      case 1: o1.cl = MAPDB_LINE; break;
      case 2: o1.cl = MAPDB_POLYGON; break;
      default:
        throw Err() << "wrong VMAP object class: "<< cl;
    }

    // convert type
    o1.type = 0;
    for (auto const & cnv: cnvs[o1.cl]){
      if (cnv.x == 0 || cnv.x == (o.type & 0xFFFFF)) {
        o1.type = cnv.y? cnv.y : (o.type & 0xFFFFF);
        break;
      }
    }

    // skip unknown types
    if (!o1.type) continue;


    // name and comments
    o1.name = o.text;
    o1.comm = o.comm;

    // source
    if (o.opts.exists("Source")) o1.tags.insert(o.opts.get<string>("Source"));

    // angle (deg -> deg)
    if (o.opts.exists("Angle")) o1.angle=o.opts.get<float>("Angle");

    // set coordinates
    o1.dMultiLine::operator=(o);

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
  vector<iLine> cnvs;
  // default configuration 0->0
  cnvs.resize(3);
  for (int i=0; i<3; i++)
    cnvs[i].push_back(iPoint(0,0));

  // Read configuration file.
  if (opts.exists("config")){

    // reset default configuration:
    for (int i=0; i<3; i++) cnvs[i] = iLine();

    int line_num[2] = {0,0};
    ifstream ff(opts.get<string>("config"));
    try {
      while (1){
        vector<string> vs = read_words(ff, line_num, true);
        if (vs.size()<1) break;

        if (vs[0]=="point"){
          if (vs.size()!=2 && vs.size()!=3)
            throw Err() << "point: two or three arguments expected: "
                           "<type_in> [<type_out>]";
          cnvs[0].push_back(iPoint(
            str_to_type<int>(vs[1]), vs.size()<3? 0:str_to_type<uint16_t>(vs[2])));
          continue;
        }

        if (vs[0]=="line"){
          if (vs.size()!=2 && vs.size()!=3)
            throw Err() << "line: two or three arguments expected: "
                           "<type_in> [<type_out>]";
          cnvs[1].push_back(iPoint(
            str_to_type<int>(vs[1]), vs.size()<3? 0:str_to_type<uint16_t>(vs[2])));
          continue;
        }

        if (vs[0]=="area"){
          if (vs.size()!=2 && vs.size()!=3)
            throw Err() << "area: two or three arguments expected: "
                           "<type_in> [<type_out>]";
          cnvs[2].push_back(iPoint(
            str_to_type<int>(vs[1]), vs.size()<3? 0:str_to_type<uint16_t>(vs[2])));
          continue;
        }

        throw Err() << "unknown command: " << vs[0];
      }
    } catch (Err e){
        throw Err() << "MapDB::export_mp: bad configuration file at line "
                    << line_num[0] << ": " << e.str();
    }
  }

  if (opts.exists("cnv_points")) cnvs[0] = opts.get<dLine>("cnv_points");
  if (opts.exists("cnv_lines"))  cnvs[1] = opts.get<dLine>("cnv_lines");
  if (opts.exists("cnv_areas"))  cnvs[2] = opts.get<dLine>("cnv_areas");

  VMap vmap_data;
  uint32_t key = 0;
  std::string str = objects.get_first(key);

  while (key!=0xFFFFFFFF){
    MapDBObj o;
    o.unpack(str);

    VMapObj o1;
    o1.type = 0;

    // convert type
    for (auto const & cnv: cnvs[o.cl]){
      if (cnv.x == 0 || cnv.x == o.type) {
        o1.type = cnv.y? cnv.y : o.type;
        break;
      }
    }

    // skip unknown types
    if (o1.type != 0) {

      switch (o.cl){
        case MAPDB_POINT:   break;
        case MAPDB_LINE:    o1.type |= 0x100000; break;
        case MAPDB_POLYGON: o1.type |= 0x200000; break;
        default:
          throw Err() << "wrong MapDB object class: "<< o.cl;
      }

      // name
      o1.text = o.name;
      o1.comm = o.comm;

      // source
      if (o.tags.size()>0) o1.opts.put("Source", *o.tags.begin());

      // angle (deg->deg)
      if (o.angle!=0) o1.opts.put("Angle", o.angle);

      // points
      o1.dMultiLine::operator=(o);

      vmap_data.push_back(o1);
    }

    str = objects.get_next(key);
  }

  // map border (convert to single-segment line)
  vmap_data.brd = join_polygons(get_map_brd());

  // map name
  vmap_data.name = get_map_name();

  // write vmap file
  ofstream out(vmap_file);
  write_vmap(out, vmap_data);
}

