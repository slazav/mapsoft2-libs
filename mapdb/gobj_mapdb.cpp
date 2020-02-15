#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>

#include "geom/line_walker.h"
#include "gobj_mapdb.h"
#include "read_words/read_words.h"
#include "geo_data/geo_io.h"
#include "geo_mkref/geo_mkref.h"
#include "filename/filename.h"

using namespace std;

/**********************************************************/
void
ms2opt_add_mapdb_render(GetOptSet & opts){
  const char *g = "MAPDB_RENDER";
  opts.add("config", 1,'c',g, "Configuration file for vector map rendering.");
}
/**********************************************************/

GObjMapDB::GObjMapDB(const std::string & mapdir, const Opt &o) {

  opt = std::shared_ptr<Opt>(new Opt(o));
  map = std::shared_ptr<MapDB>(new MapDB(mapdir));

  // Read configuration file.
  std::string cfgfile = opt->get<string>("config", mapdir + "/render.cfg");
  cfgdir = file_get_prefix(cfgfile);

  ifstream ff(cfgfile);
  if (!ff) throw Err()
    << "GObjMapDB: can't open configuration file: " << cfgfile;

  int line_num[2] = {0,0};
  int depth = 0;
  std::shared_ptr<DrawingStep> st(NULL);
  std::string ftr; // current feature

  while (1){
    vector<string> vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;
    ftr = "";

    try{

      // draw an object (point, line, area)
      if (vs.size() > 1 && vs[0].find(':')!=std::string::npos) {
        st.reset(new DrawingStep(map.get()));
        st->etype = MapDBObj::make_type(vs[0]);
        switch (st->etype >> 24) {
          case MAPDB_POINT:   st->action = STEP_DRAW_POINT; break;
          case MAPDB_LINE:    st->action = STEP_DRAW_LINE; break;
          case MAPDB_POLYGON: st->action = STEP_DRAW_AREA; break;
          case MAPDB_TEXT:    st->action = STEP_DRAW_TEXT; break;
          default: throw Err() << "Bad map object type: " << hex << st->etype;
        }
        st->step_name = vs[0];
        ftr = vs[1];
        vs.erase(vs.begin(), vs.begin()+2);
        add(depth--, st);
      }

      // draw on the whole map
      else if (vs.size() > 2 && vs[0] == "map") {
        st.reset(new DrawingStep(map.get()));
        st->action = STEP_DRAW_MAP;
        st->step_name = vs[0];
        ftr = vs[1];
        vs.erase(vs.begin(), vs.begin()+2);
        add(depth--, st);
      }

      // add feature to a previous step
      else if (vs.size() > 1 && vs[0] == "+" && st) {
        ftr = vs[1];
        vs.erase(vs.begin(),vs.begin()+2);
      }

      // setref command
      else if (vs.size() > 1 && vs[0] == "set_ref") {
        st.reset(); // "+" should not work after the command
        if (vs[1] == "file") {
          if (vs.size()!=3) throw Err()
            << "wrong number of arguments: setref file <filename>";
          GeoData d;
          read_geo(vs[2], d);
          if (d.maps.size()<1 || d.maps.begin()->size()<1) throw Err()
            << "setref: can't read any reference from file: " << vs[2];
          ref = (*d.maps.begin())[0];
        }
        else if (vs[1] == "nom") {
          if (vs.size()!=4) throw Err()
            << "wrong number of arguments: setref nom <name> <dpi>";
          Opt o;
          o.put("mkref", "nom");
          o.put("name", vs[2]);
          o.put("dpi", vs[3]);
          ref = geo_mkref(o);
        }
        else throw Err() << "setref command: 'file' or 'nom' word is expected";
        vs.erase(vs.begin(),vs.begin()+2);
        continue;
      }

      else {
        throw Err() << "Unknown command or drawing step: " << vs[0];
      }

      /// Parse features

      // stroke <color> <thickness>
      if (ftr == "stroke"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_STROKE,
          std::shared_ptr<Feature>(new FeatureStroke(vs)));
        continue;
      }

      // fill <color>
      if (ftr == "fill"){
        st->check_type(STEP_DRAW_LINE | STEP_DRAW_AREA | STEP_DRAW_MAP | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_FILL,
          std::shared_ptr<Feature>(new FeatureFill(vs)));
        continue;
      }

      // patt <file> <scale>
      if (ftr == "patt"){
        st->check_type(STEP_DRAW_LINE | STEP_DRAW_AREA | STEP_DRAW_MAP | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_PATT,
          std::shared_ptr<Feature>(new FeaturePatt(cfgdir, vs)));
        continue;
      }

      // img <file> <scale>
      if (ftr == "img"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_POINT);
        st->features.emplace(FEATURE_IMG,
          std::shared_ptr<Feature>(new FeaturePatt(cfgdir, vs)));
        continue;
      }

      // img <file> <scale>
      if (ftr == "img_filter"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_POINT | STEP_DRAW_MAP | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_IMG_FILTER,
          std::shared_ptr<Feature>(new FeatureImgFilter(vs)));
        continue;
      }

      // smooth <distance>
      if (ftr == "smooth"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE);
        st->features.emplace(FEATURE_SMOOTH,
          std::shared_ptr<Feature>(new FeatureSmooth(vs)));
        continue;
      }

      // dash <length1> <length2> ...
      if (ftr == "dash"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_DASH,
          std::shared_ptr<Feature>(new FeatureDash(vs)));
        continue;
      }

      // cap round|butt|square
      if (ftr == "cap"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_CAP,
          std::shared_ptr<Feature>(new FeatureCap(vs)));
        continue;
      }

      // join round|miter
      if (ftr == "join"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_JOIN,
          std::shared_ptr<Feature>(new FeatureJoin(vs)));
        continue;
      }

      // operator <op>
      if (ftr == "operator"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE |
                       STEP_DRAW_POINT | STEP_DRAW_TEXT | STEP_DRAW_MAP);
        st->features.emplace(FEATURE_OP,
          std::shared_ptr<Feature>(new FeatureOp(vs)));
        continue;
      }


      // lines <lines> ...
      if (ftr == "lines"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA);
        st->features.emplace(FEATURE_LINES,
          std::shared_ptr<Feature>(new FeatureLines(vs)));
        continue;
      }

      // circles <circle> ...
      if (ftr == "circles"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA);
        st->features.emplace(FEATURE_CIRCLES,
          std::shared_ptr<Feature>(new FeatureCircles(vs)));
        continue;
      }

      // draw_pos (point|begin|end|dist|edist) ...
      if (ftr == "draw_pos"){
        st->check_type(STEP_DRAW_LINE|STEP_DRAW_AREA);
        st->features.emplace(FEATURE_DRAW_POS,
          std::shared_ptr<Feature>(new FeatureDrawPos(vs)));
        continue;
      }

      // draw_dist <dist> [<dist0>]
      if (ftr == "draw_dist"){
        st->check_type(STEP_DRAW_LINE|STEP_DRAW_AREA);
        st->features.emplace(FEATURE_DRAW_DIST,
          std::shared_ptr<Feature>(new FeatureDrawDist(vs)));
        continue;
      }

      // sel_range
      if (ftr == "sel_range"){
        st->check_type(STEP_DRAW_POINT|STEP_DRAW_LINE|STEP_DRAW_AREA);
        st->features.emplace(FEATURE_SEL_RANGE,
          std::shared_ptr<Feature>(new FeatureSelRange(vs)));
        continue;
      }

      // move_to area|line <type> <max distance>
      if (ftr == "move_to"){
        st->check_type(STEP_DRAW_POINT);
        st->features.emplace(FEATURE_MOVETO,
          std::shared_ptr<Feature>(new FeatureMoveTo(vs, false)));
        continue;
      }

      // rotate_to area|line <type> <max distance>
      if (ftr == "rotate_to"){
        st->check_type(STEP_DRAW_POINT);
        st->features.emplace(FEATURE_MOVETO,
          std::shared_ptr<Feature>(new FeatureMoveTo(vs, true)));
        continue;
      }

      // font <size> <font>
      if (ftr == "font"){
        st->check_type(STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_FONT,
          std::shared_ptr<Feature>(new FeatureFont(vs)));
        continue;
      }

      // write <color>
      if (ftr == "write"){
        st->check_type(STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_WRITE,
          std::shared_ptr<Feature>(new FeatureWrite(vs)));
        continue;
      }

      // group <name>
      if (ftr == "group"){
        FeatureGroup f(vs); // parse parameters, but do not save feature data
        st->group_name = f.name;

        // if the group is not in the group list, add it there
        bool known_group = false;
        for (auto const & gr:groups)
          if (gr == f.name){ known_group = true; break;}
        if (!known_group) groups.push_back(f.name);
        continue;
      }

      // name <name>
      if (ftr == "name"){
        FeatureName f(vs); // parse parameters, but do not save feature data
        st->step_name = f.name;
        continue;
      }

      throw Err() << "unknown feature";
    }
    catch (Err e) {
      throw Err() << "GObjMapDB: configuration file, line " << line_num[0] << ": "
                  << (ftr==""? "": "feature : \"" + ftr + "\": ")
                  << e.str();
    }

  } // end of configuration file

}

/**********************************************************/

#include "geom_tools/line_utils.h"
// change object coordinates according to features
void
GObjMapDB::DrawingStep::convert_coords(MapDBObj & O){

  if (cnv) cnv->bck(O);

  if (features.count(FEATURE_MOVETO)){
    auto ftr = (FeatureMoveTo *)features.find(FEATURE_MOVETO)->second.get();
    for (auto & l:O){ // segments
      for (auto & p:l){ // points
        dRect r(p,p);
        r.expand(ftr->dist);
        if (cnv) r = cnv->frw_acc(r);

        dMultiLine lines;
        for (auto & t: ftr->targets){
          auto ids = map->find(t, r);
            for (int i:ids){
            auto O1 = map->get(i);
            if (cnv) cnv->bck(O1);
            for (auto const & l:O1)
              lines.push_back(l);
          }
        }

        dPoint t(1,0);
        double dd = nearest_pt(lines, t, p, ftr->dist);
        if (ftr->rotate) O.angle = -atan2(t.y, t.x);
      }
    }
  }

}

int
GObjMapDB::DrawingStep::draw(const CairoWrapper & cr, const dRect & range){

  std::set<uint32_t> ids;

  // calculate range for object selecting
  dRect sel_range(range);
  double exp_dist = 0;

  if (action == STEP_DRAW_TEXT){
    exp_dist = 1024; // TODO - max_text_size setting!
  }
  else {
    // expand by line width
    if (features.count(FEATURE_STROKE)){
      auto ftr = (FeatureStroke *)features.find(FEATURE_STROKE)->second.get();
      exp_dist = std::max(exp_dist, ftr->th);
    }
    // expand by image size (image may be rotated and arbitrarly centered, use hypot(w,h) as size)
    if (features.count(FEATURE_IMG)){
      auto ftr = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
      exp_dist = std::max(exp_dist, hypot(ftr->img.width(), ftr->img.height()));
    }
    // expand by move_to distance
    if (features.count(FEATURE_MOVETO)){
      auto ftr = (FeatureMoveTo *)features.find(FEATURE_MOVETO)->second.get();
      exp_dist = std::max(exp_dist, ftr->dist);
    }
    // expand by lines bbox (again, can be rotated, at least for points)
    if (features.count(FEATURE_LINES)){
      auto ftr = (FeatureLines *)features.find(FEATURE_LINES)->second.get();
      exp_dist = std::max(exp_dist, hypot(ftr->bbox.w, ftr->bbox.h));
    }
    // expand by circles bbox
    if (features.count(FEATURE_LINES)){
      auto ftr = (FeatureCircles *)features.find(FEATURE_CIRCLES)->second.get();
      exp_dist = std::max(exp_dist, hypot(ftr->bbox.w, ftr->bbox.h));
    }
  }
  sel_range.expand(exp_dist);

  // convert to wgs84
  if (cnv) sel_range = cnv->frw_acc(sel_range);

  // Select objects in the range.
  if (action == STEP_DRAW_POINT ||
      action == STEP_DRAW_LINE ||
      action == STEP_DRAW_AREA ||
      action == STEP_DRAW_TEXT){

    ids = map->find(etype, sel_range);
    if (ids.size()==0) return GObj::FILL_NONE;
  }

  // if SEL_RANGE feature exists, draw the rectangle
  if (features.count(FEATURE_SEL_RANGE)) {
    auto data = (FeatureSelRange *)features.find(FEATURE_SEL_RANGE)->second.get();
    cr->begin_new_path();
    cr->set_color_a(data->col);
    cr->set_line_width(data->th);
    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      dRect box = cnv->bck_acc(O.bbox()); //to points
      box.expand(exp_dist);
      cr->mkpath(rect_to_line(box), true);
    }
    cr->stroke();
  }

  // Make drawing path for points, lines, areas
  cr->begin_new_path();
  if ((action == STEP_DRAW_POINT ||
       action == STEP_DRAW_LINE ||
       action == STEP_DRAW_AREA) &&
      (features.count(FEATURE_STROKE) ||
       features.count(FEATURE_FILL) ||
       features.count(FEATURE_PATT)) ){
    double sm = 0;
    if (features.count(FEATURE_SMOOTH)){
      auto ftr = (FeatureSmooth *)features.find(FEATURE_SMOOTH)->second.get();
      sm = ftr->dist;
    }
    // for each object
    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      convert_coords(O);
      bool close = (action == STEP_DRAW_AREA);

      // make path for Lines or Circles
      if (features.count(FEATURE_LINES) ||
          features.count(FEATURE_CIRCLES)) {

        dMultiLine lines;
        if (features.count(FEATURE_LINES)){
          auto ftr = (FeatureLines *)features.find(FEATURE_LINES)->second.get();
          lines = ftr->lines;
        }

        dLine circles;
        if (features.count(FEATURE_CIRCLES)){
          auto ftr = (FeatureCircles *)features.find(FEATURE_CIRCLES)->second.get();
          circles = ftr->circles;
        }

        FeatureDrawPos::pos_t pos = FeatureDrawPos::POINT;
        if (features.count(FEATURE_DRAW_POS)){
          auto ftr = (FeatureDrawPos *)features.find(FEATURE_DRAW_POS)->second.get();
          pos = ftr->pos;
        }

        double dist = 0, dist0 = 0;
        if (features.count(FEATURE_DRAW_DIST)){
          auto ftr = (FeatureDrawDist *)features.find(FEATURE_DRAW_DIST)->second.get();
          dist = ftr->dist; dist0 = ftr->dist0;
        }

        dLine ref_points;

        for (auto const & l:O){
          LineWalker lw(l, close);
          switch(pos){
            case FeatureDrawPos::POINT:
              lw.move_begin();
              while (!lw.is_end()){
                ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
                lw.move_frw_to_node();
              }
              if (!close)
                ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
              break;
            case FeatureDrawPos::BEGIN:
              lw.move_begin();
              ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
              break;
            case FeatureDrawPos::END:
              lw.move_end();
              ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
              break;
            case FeatureDrawPos::DIST:
            case FeatureDrawPos::EDIST:
              lw.move_begin();
              lw.move_frw(dist0);
              if (pos == FeatureDrawPos::EDIST) { // adjust distance
                double n = floor((lw.length()-dist0*2)/dist);
                dist = (lw.length()-dist0*2)/n;
                dist *= (1-1e-10); // to be sure that last element will be drawn
              }
              if (dist==0) break;
              while (lw.dist() <= lw.length()-dist0){
                ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
                lw.move_frw(dist);
                if (lw.is_end()) break;
              }
              break;
          }
        }
        for (auto const &p:ref_points){
          cr->mkpath_smline(p + rotate2d(lines, dPoint(),p.z), sm);
          for (auto const &c:circles){
            cr->move_to(p.x+c.x+c.z, p.y+c.y);
            cr->arc(p.x+c.x, p.y+c.y, c.z, 0, 2*M_PI);
          }
        }

      }
      // make path from original object
      else {
        cr->mkpath_smline(O, close, sm);
      }
    }
  }

  // Operator feature
  if (features.count(FEATURE_OP)){
    auto data = (FeatureOp *)features.find(FEATURE_OP)->second.get();
    cr->set_operator(data->op);
  }


  // Font feature (set font + font size)
  if (features.count(FEATURE_FONT)){
    auto data = (FeatureFont *)features.find(FEATURE_FONT)->second.get();
    cr->set_font_size(data->size);
    // For work with patterns see:
    // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x103.html#AEN242
    // For font properties see:
    // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html
    // https://www.freedesktop.org/software/fontconfig/fontconfig-user.html
    // https://wiki.archlinux.org/index.php/Font_configuration
    FcPattern *patt = FcNameParse((const FcChar8 *)data->font.c_str());
    cr->set_font_face( Cairo::FtFontFace::create(patt));
    FcPatternDestroy(patt);
  }

  // make path for TEXT objects
  if (action == STEP_DRAW_TEXT &&
      (features.count(FEATURE_STROKE) ||
       features.count(FEATURE_FILL) ||
       features.count(FEATURE_PATT)) ){
    for (auto const i: ids){
      auto O = map->get(i);
      if (O.size()==0 || O[0].size()==0) continue;
      dPoint pt = O[0][0];
      if (cnv) cnv->bck(pt);
      dRect rng = cr->get_text_extents(O.name.c_str()) + pt;
      if (!intersect(rng, range)) continue;
      cr->move_to(pt);
      cr->text_path(O.name);
    }
  }

  // Pattern feature
  if (features.count(FEATURE_PATT)){
    auto data = (FeaturePatt *)features.find(FEATURE_PATT)->second.get();
    if (features.count(FEATURE_IMG_FILTER)){
      auto data_f = (FeatureImgFilter *)features.find(FEATURE_IMG_FILTER)->second.get();
      data->patt->set_filter(data_f->flt);
    }
    data->patt->set_extend(Cairo::EXTEND_REPEAT);
    cr->set_source(data->patt);
    if (action == STEP_DRAW_MAP)
      cr->paint();
    else
      cr->fill_preserve();
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
    if (features.count(FEATURE_IMG_FILTER)){
      auto data_f = (FeatureImgFilter *)features.find(FEATURE_IMG_FILTER)->second.get();
      data->patt->set_filter(data_f->flt);
    }
    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      O.angle *= M_PI/180;
      convert_coords(O);
      for (auto const & l:O){
        for (dPoint p:l){
          cr->save();
          cr->translate(p.x, p.y);
          cr->rotate(-O.angle);
          cr->set_source(data->patt);
          cr->paint();
          cr->restore();
        }
      }
    }
  }

  // Image feature (areas)
  if (features.count(FEATURE_IMG) && action == STEP_DRAW_AREA){
    auto data = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
    if (features.count(FEATURE_IMG_FILTER)){
      auto data_f = (FeatureImgFilter *)features.find(FEATURE_IMG_FILTER)->second.get();
      data->patt->set_filter(data_f->flt);
    }
    for (auto const i: ids){
      auto O = map->get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      convert_coords(O);
      // each segment
      for (auto const & l:O){
        dPoint p = l.bbox().cnt();
        cr->translate(p.x, p.y);
        cr->set_source(data->patt);
        cr->paint();
        cr->translate(-p.x, -p.y);
      }
    }
  }

  if (features.count(FEATURE_WRITE)){
    auto data = (FeatureWrite *)features.find(FEATURE_WRITE)->second.get();
    cr->begin_new_path(); // this is needed if we have WRITE+STROKE/FILL feateres
    cr->set_color(data->color);
    for (auto const i: ids){
      auto O = map->get(i);
      if (O.size()==0 || O[0].size()==0) continue;
      dPoint pt = O[0][0];
      if (cnv) cnv->bck(pt);
      dRect rng = cr->get_text_extents(O.name.c_str()) + pt;
      if (!intersect(rng, range)) continue;
      cr->text(O.name.c_str(), pt, 0, 0, 0); // TODO: scale, angle, halign, valign
    }
  }

  return GObj::FILL_PART;
}

/**********************************************************/
