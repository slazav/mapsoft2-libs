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

GObjMapDB::GObjMapDB(const std::string & mapdir){

  map = std::shared_ptr<MapDB>(new MapDB(mapdir, false));

  // Read configuration file.
  std::string cfgfile = opt->get<string>("config", mapdir + "/render.cfg");

  ifstream ff(cfgfile);
  if (!ff) throw Err()
    << "GObjMapDB: can't open configuration file: " << cfgfile;

  int line_num[2] = {0,0};
  int depth = 0;
  std::shared_ptr<DrawingStep> st(NULL);
  while (1){
    vector<string> vs = read_words(ff, line_num, true);
    if (vs.size()==0) break;

    // draw a point object
    if (vs.size() > 2 && vs[0] == "point") {
      st.reset(new DrawingStep(map.get()));
      st->action = STEP_DRAW_POINT;
      st->etype = (uint32_t)str_to_type<uint16_t>(vs[1]);
      vs.erase(vs.begin(), vs.begin()+2);
      add(depth--, st);
    }

    // draw a line object
    else if (vs.size() > 2 && vs[0] == "line") {
      st.reset(new DrawingStep(map.get()));
      st->action = STEP_DRAW_LINE;
      st->etype = (uint32_t)str_to_type<uint16_t>(vs[1]) | (MAPDB_LINE<<16);
      vs.erase(vs.begin(), vs.begin()+2);
      add(depth--, st);
    }

    // draw an area object
    else if (vs.size() > 2 && vs[0] == "area") {
      st.reset(new DrawingStep(map.get()));
      st->action = STEP_DRAW_AREA;
      st->etype = (uint32_t)str_to_type<uint16_t>(vs[1]) | (MAPDB_POLYGON<<16);
      vs.erase(vs.begin(), vs.begin()+2);
      add(depth--, st);
    }

    // add feature to a previous step
    else if (st && vs[0] == "+") {
      vs.erase(vs.begin(),vs.begin()+1);
    }

    else {
      throw Err() << "Can't parse configuration file at line "
                  << line_num[0];
    }
    add_feature(*st.get(), vs, line_num[0]);

  }
}

void
GObjMapDB::add_feature(DrawingStep & st, const std::vector<std::string> & vs, int ln){
  if (vs.size()==0) throw Err() << "GObjMapDB::add_feature: empty data";

  // stroke <color> <thickness>
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

    st.put_feature<uint32_t, double>(FEATURE_STROKE, vs[1], vs[2]);
    return;
  }

  // fill <color>
  if (vs[0] == "fill"){
    if (st.action != STEP_DRAW_AREA )
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature is only valud in "
                  << "area drawing steps";
    if (vs.size()!=2)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature should have 1 argument (color)";

    st.put_feature<uint32_t>(FEATURE_FILL, vs[1]);
    return;
  }

  // smooth <distance>
  if (vs[0] == "smooth"){
    if (st.action != STEP_DRAW_AREA &&
        st.action != STEP_DRAW_LINE)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature is only valud in "
                  << "line and area drawing steps";
    if (vs.size()!=2)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature should have 1 argument (distance)";

    st.put_feature<double>(FEATURE_SMOOTH, vs[1]);
    return;
  }

  // dash <v1> <v2> ...
  if (vs[0] == "dash"){
    if (st.action != STEP_DRAW_LINE &&
        st.action != STEP_DRAW_AREA)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature is only valud in "
                  << "line and area drawing steps";
    if (vs.size()<1)
      throw Err() << "GObjMapDB: configuration line " << ln
                  << ": " << vs[0] << " feature should at least 1 argument";
    // ....

    return;
  }

  throw Err() << "GObjMapDB: can't parse feature at line " << ln << ": " << vs[0];
}

/**********************************************************/

int
GObjMapDB::DrawingStep::draw(const CairoWrapper & cr, const dRect & range){

  std::set<uint32_t> ids;
  dRect wgs_range = cnv? cnv->frw_acc(range) : range;

  // Select objects in the drawing range.
  // Make drawing path
  if (action == STEP_DRAW_POINT ||
      action == STEP_DRAW_LINE ||
      action == STEP_DRAW_AREA){

    ids = map->find(etype, wgs_range);
    if (ids.size()==0) return GObj::FILL_NONE;
    cr->begin_new_path();

    double sm = 0;
    if (features.count(FEATURE_SMOOTH)){
      sm = *(double*)features.find(FEATURE_SMOOTH)->second.data();
    }

    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), wgs_range)) continue;
      cr->mkpath_smline(cnv? cnv->bck_acc(O) : O,
             action == STEP_DRAW_AREA, sm);
    }
  }

  // Stroke feature
  if (features.count(FEATURE_STROKE)){
    struct fdata_t {uint32_t col; double th;} * fdata =
      (fdata_t*)features.find(FEATURE_STROKE)->second.data();
    cr->set_color_a(fdata->col);
    cr->set_line_width(fdata->th);
    cr->stroke_preserve();
  }

  // Fill feature
  if (features.count(FEATURE_FILL)){
    struct fdata_t {uint32_t col;} * fdata =
      (fdata_t*)features.find(FEATURE_FILL)->second.data();
    cr->set_color_a(fdata->col);
    cr->fill_preserve();
  }

  return GObj::FILL_PART;
}

/**********************************************************/
