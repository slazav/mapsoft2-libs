#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>

#include "mapdb.h"
#include "mp/mp.h"
#include "read_words/read_words.h"

/**********************************************************/
/* Import/export of MP format */

using namespace std;

/**********************************************************/
void
ms2opt_add_mapdb_mp_imp(GetOptSet & opts){
  const char *g = "MAPDB_MP_IMP";
  opts.add("config", 1,'c',g,   "Configuration file for MP->MapDB conversion");
  opts.add("data_level", 1,0,g, "Read data from a specified MP data level");
}

/**********************************************************/
void
ms2opt_add_mapdb_mp_exp(GetOptSet & opts){
  const char *g = "MAPDB_MP_EXP";
  opts.add("config", 1,'c',g,
    "Configuration file for MapDB->MP conversion");
  opts.add("codepage", 1,0,g, "set MP codepage");
  opts.add("name",     1,0,g, "set MP map name");
  opts.add("id",       1,0,g, "ser MP map ID");

}

/**********************************************************/
// what can be done with unknown types
enum unknown_types_t {
  UNKNOWN_TYPES_SKIP,
  UNKNOWN_TYPES_WARN,
  UNKNOWN_TYPES_CNV,
  UNKNOWN_TYPES_ERR
};

/**********************************************************/
void
MapDB::import_mp(const string & mp_file, const Opt & opts){

  // type conversion table
  std::map<uint32_t, uint32_t> obj_map; // mp type -> mapdb type

  // If configuration file name is empty we will
  // use some default conversion.
  std::string cfg_file = opts.get<string>("config", "");

  // What do we want to do with objects wich are not listed explicetely in
  // the configuration file?
  unknown_types_t unknown_types = UNKNOWN_TYPES_CNV;
  std::set<uint32_t> unknowns;

  // data level
  int level = 0;

  // Read configuration file.
  if (cfg_file != ""){
    int line_num[2] = {0,0};
    ifstream ff(cfg_file);

    try {
      while (1){
        vector<string> vs = read_words(ff, line_num, true);
        if (vs.size()<1) break;

        // unknown_types setting: what to do with unknown types
        // default: "convert"
        if (vs[0] == "unknown_types"){
          if (vs.size()!=2) throw Err() << "unknown_types: one argument expected";
          if      (vs[1] == "skip")    unknown_types = UNKNOWN_TYPES_SKIP;
          else if (vs[1] == "warning") unknown_types = UNKNOWN_TYPES_WARN;
          else if (vs[1] == "convert") unknown_types = UNKNOWN_TYPES_CNV;
          else if (vs[1] == "error")   unknown_types = UNKNOWN_TYPES_ERR;
          else throw Err() << "unknown_types: skip, warning, error or convert expected";
          continue;
        }

        // data_level setting: which MP data level use
        // default: 0
        if (vs[0] == "data_level"){
          if (vs.size()!=2) throw Err() << "data_level: one argument expected";
          level = str_to_type<int>(vs[1]);
          continue;
        }

        // add other configuration commands here...

        // object conversion (src, dst, [label])
        if (vs.size() == 2) {
          uint32_t src_type = MapDBObj::make_type(vs[0]);
          uint32_t dst_type = (vs[1] == "-")? src_type : MapDBObj::make_type(vs[1]);
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
          continue;
        }
        throw Err() << "unknown command: " << vs[0];
      }
    } catch (Err & e){
        throw Err() << "MapDB::import_mp: bad configuration file at line "
                    << line_num[0] << ": " << e.str();
    }
  }

  if (opts.exists("data_level")) level   = opts.get<int>("data_level");

  // read MP file
  MP mp_data;
  ifstream in(mp_file);
  read_mp(in, mp_data);

  for (auto const & o:mp_data){

    uint16_t cl;
    switch (o.Class){
      case MP_POINT:   cl = MAPDB_POINT;   break;
      case MP_LINE:    cl = MAPDB_LINE;    break;
      case MP_POLYGON: cl = MAPDB_POLYGON; break;
      default:
        throw Err() << "wrong MP class: "<< o.Class;
    }
    uint16_t tnum = o.Type & 0xFFFF;
    uint32_t type = (cl<<24) + tnum;

    // convert type
    if (obj_map.count(type)==0){
      switch (unknown_types) {
      case UNKNOWN_TYPES_SKIP:
        continue;
      case UNKNOWN_TYPES_WARN:
        unknowns.insert(type);
        continue;
      case UNKNOWN_TYPES_ERR:
        throw Err() << "unknown type: " << MapDBObj::print_type(type);
      case UNKNOWN_TYPES_CNV:
        break;
      }
    }
    else {
      type = obj_map.find(type)->second;
    }

    MapDBObj o1(type);

    // name, comments
    o1.name = o.Label;
    o1.comm = o.Comment;

    // source
    if (o.Opts.exists("Source")) o1.tags.insert(o.Opts.get<string>("Source"));

    // choose data level (move to MP?)
    int l = -1;
    if (level < (int)o.Data.size() && o.Data[level].size()>0) l = level;
    if (level <= o.EndLevel){
      for (int i = level; i>0; i--){
        if (i<(int)o.Data.size() && o.Data[i].size()>0) {l=i; break;}
      }
    }
    if (l==-1) continue; // no data for the requested level

    // set coordinates
    o1.dMultiLine::operator=(o.Data[l]);

    // add object
    add(o1);

  }

}
/**********************************************************/

void
MapDB::export_mp(const string & mp_file, const Opt & opts){

  // type conversion table
  std::map<uint32_t, uint32_t> obj_map; // mapdb type -> mp type

  // If configuration file name is empty we will
  // use some default conversion.
  std::string cfg_file = opts.get<string>("config", "");


  // What do we want to do with objects wich are not listed explicetely in
  // the configuration file?
  unknown_types_t unknown_types = UNKNOWN_TYPES_CNV;
  std::set<uint32_t> unknowns;

  MP mp_data;

  // Read configuration file.
  if (cfg_file != ""){
    ifstream ff(cfg_file);
    int line_num[2] = {0,0};

    try {
      while (1){
        vector<string> vs = read_words(ff, line_num, true);
        if (vs.size()<1) break;

        if (vs[0]=="codepage"){
          if (vs.size()!=2) throw Err() << "codepage: one argument expected";
          mp_data.Codepage = vs[1];
          continue;
        }

        if (vs[0]=="id"){
          if (vs.size()!=2) throw Err() << "id: one argument expected";
          mp_data.ID = str_to_type<int>(vs[1]);
          continue;
        }

        if (vs[0]=="name"){
          if (vs.size()!=2) throw Err() << "name: one argument expected";
          mp_data.Name = vs[1];
          continue;
        }

        // add other configuration commands here...

        // object conversion (src, dst)
        if (vs.size() == 2) {
          uint32_t src_type = MapDBObj::make_type(vs[0]);
          uint32_t dst_type = (vs[1] == "-")? src_type : MapDBObj::make_type(vs[1]);
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
    } catch (Err & e){
        throw Err() << "MapDB::export_mp: bad configuration file at line "
                    << line_num[0] << ": " << e.str();
    }
  }

  if (opts.exists("codepage"))   mp_data.Codepage = opts.get("codepage");
  if (opts.exists("name"))       mp_data.Name = opts.get("name");
  if (opts.exists("id"))         mp_data.ID = opts.get("id", 0);

  uint32_t key = 0;
  std::string str = objects.get_first(key);
  while (key!=0xFFFFFFFF){
    MapDBObj o;
    o.unpack(str);

    MPObj o1;

    // convert type
    uint32_t type = o.type;

    // convert type
    if (obj_map.count(type)==0){
      switch (unknown_types) {
      case UNKNOWN_TYPES_SKIP:
        goto end;
      case UNKNOWN_TYPES_WARN:
        unknowns.insert(type);
        goto end;
      case UNKNOWN_TYPES_ERR:
        throw Err() << "unknown type: " << MapDBObj::print_type(type);
      case UNKNOWN_TYPES_CNV:
        break;
      }
    }
    else {
      type = obj_map.find(type)->second;
    }

    // convert type to mp format
    switch (type>>24){
      case MAPDB_POINT:   o1.Class = MP_POINT; break;
      case MAPDB_LINE:    o1.Class = MP_LINE; break;
      case MAPDB_POLYGON: o1.Class = MP_POLYGON; break;
      default: goto end;
    }
    o1.Type = type & 0xFFFF;

    // name, comments
    o1.Label = o.name;
    o1.Comment = o.comm;

    // source
    if (o.tags.size()>0) o1.Opts.put("Source", *o.tags.begin());

    // points
    o1.Data.push_back(o);

    if (o1.Data.size()) mp_data.push_back(o1);

    end:
    str = objects.get_next(key);
  }
  ofstream out(mp_file);
  write_mp(out, mp_data);
}

