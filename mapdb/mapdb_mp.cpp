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
  opts.add("config", 1,'c',g,
    "Configuration file for MP->MapDB conversion");

  opts.add("cnv_points", 1,0,g,
    "Type conversion rules for point objects: "
    "[[<type_in>, <type_out>], [<type_in>, <type_out>], ...] "
    "type_in==0 matches all types; type_out==0 sets output type "
    "same as input one. Default value: [[0,0]].");

  opts.add("cnv_lines", 1,0,g,
    "Type conversion rules for line objects. Same notation as in cnv_point.");

  opts.add("cnv_areas", 1,0,g,
    "Type conversion rules for area objects. Same notation as in cnv_point.");

  opts.add("data_level", 1,0,g,
    "Read data from a specified MP data level");

}

/**********************************************************/
void
ms2opt_add_mapdb_mp_exp(GetOptSet & opts){
  const char *g = "MAPDB_MP_EXP";
  opts.add("config", 1,'c',g,
    "Configuration file for MapDB->MP conversion");

  opts.add("cnv_points", 1,0,g,
    "Type conversion rules for point objects: "
    "[[<type_in>, <type_out>], [<type_in>, <type_out>], ...] "
    "type_in==0 matches all types; type_out==0 sets output type "
    "same as input one. Default value: [[0,0]].");

  opts.add("cnv_lines", 1,0,g,
    "Type conversion rules for line objects. Same notation as in cnv_point.");

  opts.add("cnv_areas", 1,0,g,
    "Type conversion rules for area objects. Same notation as in cnv_point.");

  opts.add("codepage", 1,0,g, "set MP codepage");
  opts.add("name",     1,0,g, "set MP map name");
  opts.add("id",       1,0,g, "ser MP map ID");

}

/**********************************************************/
void
MapDB::import_mp(const string & mp_file, const Opt & opts){

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

        if (vs[0]=="level"){
          if (vs.size()!=2) throw Err() << "level: one argument expected";
          level = str_to_type<int>(vs[1]);
          continue;
        }

        throw Err() << "unknown command: " << vs[0];
      }
    } catch (Err & e){
        throw Err() << "MapDB::import_mp: bad configuration file at line "
                    << line_num[0] << ": " << e.str();
    }

  }

  if (opts.exists("cnv_points"))  cnvs[0] = opts.get<dLine>("cnv_points");
  if (opts.exists("cnv_lines"))   cnvs[1] = opts.get<dLine>("cnv_lines");
  if (opts.exists("cnv_areas"))   cnvs[2] = opts.get<dLine>("cnv_areas");
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

    // convert type
    uint16_t tnum=0;
    for (auto const & cnv: cnvs[cl]){
      if (cnv.x == 0 || cnv.x == o.Type) {
        tnum = cnv.y? cnv.y : o.Type;
        break;
      }
    }

    // skip unknown types
    if (tnum==0) continue;

    MapDBObj o1(cl, tnum);


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

  // type conversion tables (point, line, polygon)
  vector<iLine> cnvs;
  // default configuration 0->0
  cnvs.resize(3);
  for (int i=0; i<3; i++)
    cnvs[i].push_back(iPoint(0,0));

  MP mp_data;

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

        throw Err() << "unknown command: " << vs[0];
      }
    } catch (Err & e){
        throw Err() << "MapDB::export_mp: bad configuration file at line "
                    << line_num[0] << ": " << e.str();
    }
  }


  if (opts.exists("cnv_points")) cnvs[0] = opts.get<dLine>("cnv_points");
  if (opts.exists("cnv_lines"))  cnvs[1] = opts.get<dLine>("cnv_lines");
  if (opts.exists("cnv_areas")) cnvs[2] = opts.get<dLine>("cnv_areas");
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
    uint16_t tnum = o.get_tnum();
    uint16_t cl = o.get_class();
    if (cl>2) throw Err() << "bad object class: " << cl;
    for (auto const & cnv: cnvs[cl]){
      if (cnv.x == 0 || cnv.x == tnum) {
        o1.Type = cnv.y? cnv.y : tnum;
        break;
      }
    }

    switch (cl){
      case MAPDB_POINT:   o1.Class = MP_POINT; break;
      case MAPDB_LINE:    o1.Class = MP_LINE; break;
      case MAPDB_POLYGON: o1.Class = MP_POLYGON; break;
      default:
        throw Err() << "wrong MapDB object class: "<< cl;
    }

    // skip unknown types
    if (o1.Type!=-1){

      // name, comments
      o1.Label = o.name;
      o1.Comment = o.comm;

      // source
      if (o.tags.size()>0) o1.Opts.put("Source", *o.tags.begin());

      // points
      o1.Data.push_back(o);

      if (o1.Data.size()) mp_data.push_back(o1);
    }

    str = objects.get_next(key);
  }
  ofstream out(mp_file);
  write_mp(out, mp_data);
}

