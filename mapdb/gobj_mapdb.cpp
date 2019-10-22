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

GObjMapDB::GObjMapDB(const std::string & mapdir): mapdir(mapdir){

  map = std::shared_ptr<MapDB>(new MapDB(mapdir, false));

  // Read configuration file.
  std::string cfgfile = opt->get<string>("config", mapdir + "/render.cfg");

  ifstream ff(cfgfile);
  if (!ff) throw Err()
    << "GObjMapDB: can't open configuration file: " << cfgfile;

  int line_num[2] = {0,0};
  int depth = 0;
  std::shared_ptr<DrawingStep> st(NULL);
  std::string ftr; // current feature

  while (1){
    vector<string> vs = read_words(ff, line_num, true);
    if (vs.size()==0) break;

    // draw a point object
    if (vs.size() > 2 && vs[0] == "point") {
      st.reset(new DrawingStep(map.get()));
      st->action = STEP_DRAW_POINT;
      st->etype = (uint32_t)str_to_type<uint16_t>(vs[1]) | (MAPDB_POINT<<16);
      ftr = vs[2];
      vs.erase(vs.begin(), vs.begin()+3);
      add(depth--, st);
    }

    // draw a line object
    else if (vs.size() > 2 && vs[0] == "line") {
      st.reset(new DrawingStep(map.get()));
      st->action = STEP_DRAW_LINE;
      st->etype = (uint32_t)str_to_type<uint16_t>(vs[1]) | (MAPDB_LINE<<16);
      ftr = vs[2];
      vs.erase(vs.begin(), vs.begin()+3);
      add(depth--, st);
    }

    // draw an area object
    else if (vs.size() > 2 && vs[0] == "area") {
      st.reset(new DrawingStep(map.get()));
      st->action = STEP_DRAW_AREA;
      st->etype = (uint32_t)str_to_type<uint16_t>(vs[1]) | (MAPDB_POLYGON<<16);
      ftr = vs[2];
      vs.erase(vs.begin(), vs.begin()+3);
      add(depth--, st);
    }

    // draw on the whole map
    else if (vs.size() > 2 && vs[0] == "map") {
      st.reset(new DrawingStep(map.get()));
      st->action = STEP_DRAW_MAP;
      ftr = vs[1];
      vs.erase(vs.begin(), vs.begin()+2);
      add(depth--, st);
    }

    // add feature to a previous step
    else if (st && vs[0] == "+" && vs.size() > 1) {
      ftr = vs[1];
      vs.erase(vs.begin(),vs.begin()+2);
    }

    else {
      throw Err() << "Can't parse configuration file at line "
                  << line_num[0];
    }

    // Add feature
    try{

      // stroke <color> <thickness>
      if (ftr == "stroke"){
        if (st->action != STEP_DRAW_POINT &&
            st->action != STEP_DRAW_LINE &&
            st->action != STEP_DRAW_AREA )
          throw Err() << "feature is only valid in point, line, and area drawing steps";
        st->features.emplace(FEATURE_STROKE,
          std::shared_ptr<Feature>(new FeatureStroke(vs)));
        continue;
      }

      // fill <color>
      if (ftr == "fill"){
        if (st->action != STEP_DRAW_AREA &&
            st->action != STEP_DRAW_MAP)
          throw Err() << "feature is only valid in area and map drawing steps";
        st->features.emplace(FEATURE_FILL,
          std::shared_ptr<Feature>(new FeatureFill(vs)));
        continue;
      }

      // patt <file> <scale>
      if (ftr == "patt"){
        if (st->action != STEP_DRAW_AREA )
          throw Err() << "feature is only valid in area drawing steps";
        st->features.emplace(FEATURE_PATT,
          std::shared_ptr<Feature>(new FeaturePatt(mapdir, vs)));
        continue;
      }

      // img <file> <scale>
      if (ftr == "img"){
        if (st->action != STEP_DRAW_POINT &&
            st->action != STEP_DRAW_AREA )
          throw Err() << "feature is only valid in point and area drawing steps";
        st->features.emplace(FEATURE_IMG,
          std::shared_ptr<Feature>(new FeaturePatt(mapdir, vs)));
        continue;
      }

      // smooth <distance>
      if (ftr == "smooth"){
        if (st->action != STEP_DRAW_AREA &&
            st->action != STEP_DRAW_LINE)
          throw Err() << "feature is only valid in line and area drawing steps";
        st->features.emplace(FEATURE_SMOOTH,
          std::shared_ptr<Feature>(new FeatureSmooth(vs)));
        continue;
      }

      // dash <length1> <length2> ...
      if (ftr == "dash"){
        if (st->action != STEP_DRAW_AREA &&
            st->action != STEP_DRAW_LINE)
          throw Err() << "feature is only valid in line and area drawing steps";
        st->features.emplace(FEATURE_DASH,
          std::shared_ptr<Feature>(new FeatureDash(vs)));
        continue;
      }

      // cap round|butt|square
      if (ftr == "cap"){
        if (st->action != STEP_DRAW_AREA &&
            st->action != STEP_DRAW_LINE &&
            st->action != STEP_DRAW_POINT)
          throw Err() << "feature is only valid in point, line, and area drawing steps";
        st->features.emplace(FEATURE_CAP,
          std::shared_ptr<Feature>(new FeatureCap(vs)));
        continue;
      }

      // join round|miter
      if (ftr == "join"){
        if (st->action != STEP_DRAW_AREA &&
            st->action != STEP_DRAW_LINE)
          throw Err() << "feature is only valid in line and area drawing steps";
        st->features.emplace(FEATURE_JOIN,
          std::shared_ptr<Feature>(new FeatureJoin(vs)));
        continue;
      }

      // operator <op>
      if (ftr == "operator"){
        if (st->action != STEP_DRAW_AREA &&
            st->action != STEP_DRAW_LINE &&
            st->action != STEP_DRAW_POINT &&
            st->action != STEP_DRAW_MAP)
          throw Err() << "feature is only valid in line, area, point, map drawing steps";
        st->features.emplace(FEATURE_OP,
          std::shared_ptr<Feature>(new FeatureOp(vs)));
        continue;
      }

      throw Err() << "unknown feature";
    }
    catch (Err e) {
      throw Err() << "GObjMapDB: configuration file, line " << line_num[0] << ": "
                  << "feature " << (ftr==""? "": "\"" + ftr + "\": ")
                  << e.str();
    }

  }
}

/**********************************************************/

int
GObjMapDB::DrawingStep::draw(const CairoWrapper & cr, const dRect & range){

  std::set<uint32_t> ids;

  // calculate range for object selecting
  dRect sel_range(range);
  {
    // expand by line width
    if (features.count(FEATURE_STROKE)){
      auto ftr = (FeatureStroke *)features.find(FEATURE_STROKE)->second.get();
      sel_range.expand(ftr->th);
    }
    // expand by image size
    if (features.count(FEATURE_IMG)){
      auto ftr = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
      sel_range.expand(ftr->img.width(), ftr->img.height());
    }
  }
  // convert to wgs84
  if (cnv) sel_range = cnv->frw_acc(sel_range);

  // Select objects in the range.
  if (action == STEP_DRAW_POINT ||
      action == STEP_DRAW_LINE ||
      action == STEP_DRAW_AREA){

    ids = map->find(etype, sel_range);
    if (ids.size()==0) return GObj::FILL_NONE;
  }

  // Make drawing path
  cr->begin_new_path();
  if (features.count(FEATURE_STROKE) ||
      features.count(FEATURE_FILL) ||
      features.count(FEATURE_PATT)){
    double sm = 0;
    if (features.count(FEATURE_SMOOTH)){
      auto ftr = (FeatureSmooth *)features.find(FEATURE_SMOOTH)->second.get();
      sm = ftr->dist;
    }
    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      cr->mkpath_smline(cnv? cnv->bck_acc(O) : O,
             action == STEP_DRAW_AREA, sm);
    }
  }

  // Operator feature
  if (features.count(FEATURE_OP)){
    auto data = (FeatureOp *)features.find(FEATURE_OP)->second.get();
    cr->set_operator(data->op);
  }

  // Pattern feature
  if (features.count(FEATURE_PATT)){
    auto data = (FeaturePatt *)features.find(FEATURE_PATT)->second.get();
    cr->set_source(data->patt);
    cr->fill();
  }

  // Fill feature
  if (features.count(FEATURE_FILL)){
    auto data = (FeatureFill *)features.find(FEATURE_FILL)->second.get();
    cr->set_color_a(data->col);
    if (action == STEP_DRAW_MAP)
      cr->paint();
    else
      cr->fill_preserve();
  }

  // Stroke feature
  if (features.count(FEATURE_STROKE)){
    // Setup dashed line
    if (features.count(FEATURE_DASH)){
      auto data = (FeatureDash *)features.find(FEATURE_DASH)->second.get();
      cr->set_dash(data->vd, 0);
    }

    // Setup line cap
    if (features.count(FEATURE_CAP)){
      auto data = (FeatureCap *)features.find(FEATURE_CAP)->second.get();
      cr->set_line_cap(data->cap);
    }
    else
      cr->set_line_cap(Cairo::LINE_CAP_ROUND);

    // Setup line join
    if (features.count(FEATURE_JOIN)){
      auto data = (FeatureJoin *)features.find(FEATURE_JOIN)->second.get();
      cr->set_line_join(data->join);
    }
    else
      cr->set_line_join(Cairo::LINE_JOIN_ROUND);

    auto data = (FeatureStroke *)features.find(FEATURE_STROKE)->second.get();
    cr->set_color_a(data->col);
    cr->set_line_width(data->th);
    cr->stroke_preserve();
  }

  // Image feature (points)
  if (features.count(FEATURE_IMG) && action == STEP_DRAW_POINT){
    auto data = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      for (auto const & l:O){
        for (dPoint p:l){
          if (cnv) cnv->bck(p);
          cr->translate(p.x, p.y);
          cr->set_source(data->patt);
          cr->paint();
          cr->translate(-p.x, -p.y);
        }
      }
    }
  }

  // Image feature (areas)
  if (features.count(FEATURE_IMG) && action == STEP_DRAW_AREA){
    auto data = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      // each segment
      for (auto const & l:O){
        dPoint p = l.bbox().cnt();
        if (cnv) cnv->bck(p);
        cr->translate(p.x, p.y);
        cr->set_source(data->patt);
        cr->paint();
        cr->translate(-p.x, -p.y);
      }
    }
  }

  return GObj::FILL_PART;
}

/**********************************************************/
