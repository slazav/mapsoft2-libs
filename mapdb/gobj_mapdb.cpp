#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>

#include "gobj_mapdb.h"
#include "read_words/read_words.h"

using namespace std;

/**********************************************************/
void
ms2opt_add_drawmapdb(GetOptSet & opts){
//  int m = MS2OPT_DRAWMAPDB;
//  opts.add("config", 1,'c',m, "Configuration file for vector map rendering.");
}
/**********************************************************/

GObjMapDB::GObjMapDB(MapDB & map): map(map){
  // Read configuration file.
  if (!opt->exists("config")) throw Err()
    << "GObjMapDB: a configuration file should be provided (--config option)";

  ifstream ff(opt->get<string>("config"));
  if (!ff) throw Err()
    << "GObjMapDB: can't open configuration file: "
    << opt->get<string>("config");

  int line_num[2] = {0,0};
  while (1){
    DrawingStep st;
    vector<string> vs = read_words(ff, line_num, true);

    // draw a point object
    if (vs.size() > 2 && vs[0] == "point") {
      st.action = STEP_DRAW_POINT;
      st.type = str_to_type<int>(vs[1]);
      vs.erase(vs.begin(), vs.begin()+2);
      steps.push_back(st);
    }

    // draw a line object
    else if (vs.size() > 2 && vs[0] == "line") {
      st.action = STEP_DRAW_LINE;
      st.type = str_to_type<int>(vs[1]);
      vs.erase(vs.begin(), vs.begin()+2);
      steps.push_back(st);
    }

    // draw an area object
    else if (vs.size() > 2 && vs[0] == "area") {
      st.action = STEP_DRAW_AREA;
      st.type = str_to_type<int>(vs[1]);
      vs.erase(vs.begin(), vs.begin()+2);
      steps.push_back(st);
    }

    // add feature to a previous step
    else if (steps.size() > 0 && vs.size() > 1 && vs[0] == "+") {
      vs.erase(vs.begin(),vs.begin()+1);
    }

    else {
      throw Err() << "Can't parse configuration file at line "
                  << line_num[0];
    }

    add_feature(*steps.rbegin(), vs, line_num[0]);
  }
}

void
GObjMapDB::add_feature(DrawingStep & st, const std::vector<std::string> & vs, int ln){
  if (vs.size()==0) throw Err() << "GObjMapDB::add_feature: empty data";

  if (vs[0] == "stroke"){
    if (st.action != STEP_DRAW_POINT &&
        st.action != STEP_DRAW_LINE &&
        st.action != STEP_DRAW_AREA )
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << "feature is only valud in "
                  << "point, line, and area drawing steps";
    if (vs.size()!=3)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature should have 2 arguments (color, line width)";

    struct fdata_t {int col; double th; char z;} fdata =
      { str_to_type<int>(vs[1]), str_to_type<double>(vs[2]), 0 };
    st.features.emplace(FEATURE_STROKE, (const char*) &fdata);
    return;
  }

  if (vs[0] == "fill"){
    if (st.action != STEP_DRAW_AREA )
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << "feature is only valud in "
                  << "area drawing steps";
    if (vs.size()!=2)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature should have 1 argument (color)";

    struct fdata_t {int col; char z;} fdata =
      { str_to_type<int>(vs[1]), 0 };
    st.features.emplace(FEATURE_STROKE, (const char*) &fdata);
    return;
  }

  if (vs[0] == "dash"){
    if (st.action != STEP_DRAW_LINE && 
        st.action != STEP_DRAW_AREA)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << "feature is only valud in "
                  << "line and area drawing steps";
    if (vs.size()<1)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature should at least 1 argument";
    // ....

    return;
  }

  throw Err() << "GObjMapDB: can't parse feature at line " << ln << ": " << vs[0];
}


int
GObjMapDB::draw(const CairoWrapper & cr, const dRect & draw_range) {

  for (auto const & st:steps){
    switch (st.action){
      case STEP_DRAW_POINT:
      break;
      case STEP_DRAW_LINE:
      break;
      case STEP_DRAW_AREA:
      break;
    }
  }
  return GObj::FILL_PART;

/*
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
    while (1){
      vector<string> vs = read_words(ff, line_num, true);
      if (vs.size()<1) continue;

      int cl = -1;
      if (vs[0]=="point" || vs[0]=="poi") cl=0;
      if (vs[0]=="line" || vs[0]=="multiline") cl=1;
      if (vs[0]=="polygon") cl=2;

      if (cl>=0 &&  vs.size()>1 && vs.size()<3){
        cnvs[cl].push_back(iPoint(
          str_to_type<int>(vs[1]),
          vs.size()<3? 0:str_to_type<int>(vs[2])));
        continue;
      }

      throw Err() << "bad configuration file at line "
                  << line_num[0];
    }
  }

  if (opts.exists("cnv_point"))   cnvs[0] = opts.get<dLine>("cnv_point");
  if (opts.exists("cnv_line"))    cnvs[0] = opts.get<dLine>("cnv_line");
  if (opts.exists("cnv_polygon")) cnvs[0] = opts.get<dLine>("cnv_polygon");

  VMap vmap_data;
  uint32_t key = 0;
  std::string str = objects.get_first(key);

  while (key!=0xFFFFFFFF){
    MapDBObj o;
    o.unpack(str);

    VMapObj o1;

    // convert type
    for (auto const & cnv: cnvs[o.cl]){
      if (cnv.x == 0 || cnv.x == o.type) {
        o1.type = cnv.y? cnv.y : o.type;
        break;
      }
    }

    // skip unknown types
    if (!o1.type) continue;

    if (o.cl == MAPDB_LINE)    o1.type |= 0x100000;
    if (o.cl == MAPDB_POLYGON) o1.type |= 0x200000;

    // name
    o1.text = o.name;

    // comments
    if (o.comm.size()){
      int pos1=0, pos2=0;
      do {
        pos2 = o.comm.find('\n', pos1);
        o1.comm.push_back(o.comm.substr(pos1,pos2));
        pos1 = pos2+1;
      } while (pos2!=string::npos);
    }

    // direction
    o1.dir = o.dir;

    // source
    if (o.tags.size()>0) o1.opts.put("Source", *o.tags.begin());

    // angle (deg->deg)
    if (o.angle!=0) o1.opts.put("Angle", o.angle);

    // points
    o1.dMultiLine::operator=(get_coord(key));

    vmap_data.push_back(o1);
    str = objects.get_next(key);
  }

  // map border (only first segment)
  dMultiLine brd = get_map_brd();
  if (brd.size()>0) vmap_data.brd = *brd.begin();

  // map name
  vmap_data.name = get_map_name();

  // write vmap file
  ofstream out(vmap_file);
  write_vmap(out, vmap_data);
*/

}

