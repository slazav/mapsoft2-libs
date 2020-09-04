#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <deque>

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

  max_text_size = 1024;
  obj_scale = o.get("obj_scale", 1.0) * o.get("map_scale", 1.0);

  opt = o;
  map = std::shared_ptr<MapDB>(new MapDB(mapdir));

  // Read configuration file.
  Opt defs = o.get("define", Opt());
  int depth = 0;
  load_conf(opt.get<string>("config", mapdir + "/render.cfg"), defs, depth);
}

void
GObjMapDB::load_conf(const std::string & cfgfile, Opt & defs, int & depth){

  std::string cfgdir = file_get_prefix(cfgfile); // for including images and other files

  ifstream ff(cfgfile);
  if (!ff) throw Err()
    << "GObjMapDB: can't open configuration file: " << cfgfile;

  int line_num[2] = {0,0};
  std::shared_ptr<DrawingStep> st(NULL); // current step
  std::string ftr; // current feature
  std::deque<bool> ifs;  // for if/endif commands

  while (1){
    vector<string> vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;

    // apply definitions
    for (auto & s:vs){ if (defs.exists(s)) s = defs.get(s, ""); }

    ftr = "";
    try{

      // include command
      if (vs[0] == "include"){
        if (vs.size()!=2) throw Err() << "include: filename expected";
        auto fn = vs[1];
        // should not happend, but lets check before accessing fn[0]:
        if (fn.size() == 0) throw Err() << "include: empty filename";

        if (fn[0] != '/') fn = cfgdir + fn;
        load_conf(fn, defs, depth);
        continue;
      }

      // endif command
      if (vs[0] == "endif"){
        if (ifs.size()<1) throw Err() << "unexpected endif command";
        ifs.pop_back();
        continue;
      }
      // else command
      if (vs[0] == "else"){
        if (ifs.size()<1) throw Err() << "unexpected else command";
        ifs.back() = !ifs.back();
        continue;
      }
      // if command
      if (vs[0] == "if"){
        if (vs.size() == 4 && vs[2] == "=="){
          ifs.push_back(vs[1] == vs[3]);
        }
        else if (vs.size() == 4 && vs[2] == "!="){
          ifs.push_back(vs[1] != vs[3]);
        }
        else
          throw Err() << "wrong if syntax";
        continue;
      }

      // check if conditions
      bool skip = false;
      for (auto const & c:ifs)
        if (c == false) {skip = true; break;}
      if (skip) continue;

      // setref command
      if (vs.size() > 1 && vs[0] == "set_ref") {
        st.reset(); // "+" should not work after the command
        if (vs[1] == "file") {
          if (vs.size()!=3) throw Err()
            << "wrong number of arguments: setref file <filename>";
          GeoData d;
          read_geo(vs[2], d);
          if (d.maps.size()<1 || d.maps.begin()->size()<1) throw Err()
            << "setref: can't read any reference from file: " << vs[2];
          set_ref( (*d.maps.begin())[0] );
        }
        else if (vs[1] == "nom") {
          if (vs.size()!=4) throw Err()
            << "wrong number of arguments: setref nom <name> <dpi>";
          Opt o;
          o.put("mkref", "nom");
          o.put("name", vs[2]);
          o.put("dpi", vs[3]);
          set_ref( geo_mkref(o) );
        }
        else throw Err() << "setref command: 'file' or 'nom' word is expected";
        continue;
      }

      // max_text_size command
      if (vs[0] == "max_text_size") {
        st.reset(); // "+" should not work after the command
        if (vs.size()!=2) throw Err()
            << "wrong number of arguments: max_text_size <number>";
        max_text_size = str_to_type<double>(vs[1]);
        continue;
      }

      // define command
      if (vs[0] == "define") {
        st.reset(); // "+" should not work after the command
        if (vs.size()!=3) throw Err()
            << "wrong number of arguments: define <name> <definition>";
        defs.put(vs[1],vs[2]);
        continue;
      }


      /**********************************************************/
      /// Commands with features

      // draw an object (point, line, area)
      if (vs.size() > 2 && vs[0].find(':')!=std::string::npos) {
        st.reset(new DrawingStep(this));
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
        st.reset(new DrawingStep(this));
        st->action = STEP_DRAW_MAP;
        st->step_name = vs[0];
        ftr = vs[1];
        vs.erase(vs.begin(), vs.begin()+2);
        add(depth--, st);
      }

      // draw border
      else if (vs.size() > 2 && vs[0] == "brd") {
        st.reset(new DrawingStep(this));
        st->action = STEP_DRAW_BRD;
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
      else {
        throw Err() << "Unknown command or drawing step: " << vs[0];
      }

      /**********************************************************/
      /// Parse features

      // stroke <color> <thickness>
      if (ftr == "stroke"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA |
                       STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_STROKE,
          std::shared_ptr<Feature>(new FeatureStroke(vs)));
        continue;
      }

      // fill <color>
      if (ftr == "fill"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA |
                       STEP_DRAW_MAP | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_FILL,
          std::shared_ptr<Feature>(new FeatureFill(vs)));
        continue;
      }

      // patt <file> <scale>
      if (ftr == "patt"){
        st->check_type(STEP_DRAW_LINE | STEP_DRAW_AREA | STEP_DRAW_MAP |
                       STEP_DRAW_TEXT | STEP_DRAW_BRD);
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
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_POINT |
                       STEP_DRAW_MAP | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_IMG_FILTER,
          std::shared_ptr<Feature>(new FeatureImgFilter(vs)));
        continue;
      }

      // smooth <distance>
      if (ftr == "smooth"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_SMOOTH,
          std::shared_ptr<Feature>(new FeatureSmooth(vs)));
        continue;
      }

      // dash <length1> <length2> ...
      if (ftr == "dash"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_DASH,
          std::shared_ptr<Feature>(new FeatureDash(vs)));
        continue;
      }

      // cap round|butt|square
      if (ftr == "cap"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_CAP,
          std::shared_ptr<Feature>(new FeatureCap(vs)));
        continue;
      }

      // join round|miter
      if (ftr == "join"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_JOIN,
          std::shared_ptr<Feature>(new FeatureJoin(vs)));
        continue;
      }

      // operator <op>
      if (ftr == "operator"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE |
                       STEP_DRAW_POINT | STEP_DRAW_TEXT |
                       STEP_DRAW_MAP | STEP_DRAW_BRD);
        st->features.emplace(FEATURE_OP,
          std::shared_ptr<Feature>(new FeatureOp(vs)));
        continue;
      }

      // pix_al <val>
      if (ftr == "pix_align"){
        st->check_type( STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_PIXAL,
          std::shared_ptr<Feature>(new FeaturePixAl(vs)));
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

      // draw_pos (point|begin|end)
      // draw_pos dist <dist> [<dist_begin>]
      // draw_pos edist <dist> [<dist_begin>] [<dist_end>]
      if (ftr == "draw_pos"){
        st->check_type(STEP_DRAW_LINE | STEP_DRAW_AREA);
        st->features.emplace(FEATURE_DRAW_POS,
          std::shared_ptr<Feature>(new FeatureDrawPos(vs)));
        continue;
      }

      // sel_range
      if (ftr == "sel_range"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE |
                       STEP_DRAW_AREA | STEP_DRAW_TEXT);
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

      // rotate <angle,deg>
      if (ftr == "rotate"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE |
                       STEP_DRAW_AREA | STEP_DRAW_TEXT);
        st->features.emplace(FEATURE_ROTATE,
          std::shared_ptr<Feature>(new FeatureRotate(vs)));
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
    catch (Err & e) {
      throw Err() << cfgfile << ":" << line_num[0] << ": "
                  << (ftr==""? "": "feature : \"" + ftr + "\": ")
                  << e.str();
    }

  } // end of configuration file
  if (ifs.size()>0)
    throw Err() << cfgfile << ":" << "if command is not closed";

}

/**********************************************************/

#include "geom_tools/line_utils.h"
// change object coordinates according to features
// change object angle to radians
void
GObjMapDB::DrawingStep::convert_coords(MapDBObj & O){

  ConvBase *cnv = mapdb_gobj->cnv.get();
  MapDB *map = mapdb_gobj->map.get();

  // deg -> rad
  // Note:
  // - bck_ang calculates angle from x axis, but O.angle is from vertical
  // - on the picture we have inversed y axis
  // - lonlat projection has different scales in x any, we can not use bck_ang/frw_ang
  // - object angle is in deg, ccw
  if (!std::isnan(O.angle) && O.size()>0 && O[0].size()>0 && cnv) {
    dPoint pt = O[0][0]; // point where we calculate direction to geographic north:
    cnv->bck(pt); // lonlat -> px
    dPoint pt1 = pt + dPoint(0,-1); // up direction
    cnv->frw(pt); cnv->frw(pt1);  pt1-=pt;
    O.angle = O.angle*M_PI/180 + atan2(pt1.y, pt1.x) - M_PI/2; // from north, cw, rad
  }
  else {
    O.angle = 0;
  }

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
          auto ids = mapdb_gobj->map->find(t, r);
            for (int i:ids){
            auto O1 = map->get(i);
            if (cnv) cnv->bck(O1);
            for (auto const & l:O1)
              lines.push_back(l);
          }
        }

        dPoint t(1,0);
        double dd = nearest_pt(lines, t, p, ftr->dist);
        if (ftr->rotate) O.angle = atan2(t.y, t.x);
      }
    }
  }

  if (features.count(FEATURE_ROTATE)){
    auto ftr = (FeatureRotate *)features.find(FEATURE_ROTATE)->second.get();
    O.angle += ftr->val;
  }
}


// Draw a text object. Used in two places: in `write` feature
// (with parameter path=false) and for making text path in
// `fill`/`stroke`/`patt` features (with path=true).
// `range` parameter is used for check if we should draw the object.
void
GObjMapDB::DrawingStep::draw_text(MapDBObj & O, const CairoWrapper & cr, const dRect & range, bool path, bool pix_align){
  if (O.size()==0 || O[0].size()==0) return; // no coordinates
  dPoint pt = O[0][0];
  dRect ext = cr->get_text_extents(O.name.c_str());
  // To allow any rotated/align text do be in the range use diagonal
  dRect rng = expand(dRect(pt,pt), hypot(ext.w, ext.h));
  if (!intersect(rng, range)) return;

  dPoint sh;
  switch (O.align){
    case MAPDB_ALIGN_SW: break;
    case MAPDB_ALIGN_W:  sh.y=ext.h/2; break;
    case MAPDB_ALIGN_NW: sh.y=ext.h;   break;
    case MAPDB_ALIGN_N:  sh.y=ext.h;   sh.x=-ext.w/2; break;
    case MAPDB_ALIGN_NE: sh.y=ext.h;   sh.x=-ext.w;   break;
    case MAPDB_ALIGN_E:  sh.y=ext.h/2; sh.x=-ext.w;   break;
    case MAPDB_ALIGN_SE:               sh.x=-ext.w;   break;
    case MAPDB_ALIGN_S:                sh.x=-ext.w/2; break;
    case MAPDB_ALIGN_C:  sh.y=ext.h/2; sh.x=-ext.w/2; break;
  }
  if (pix_align) {
    pt = rint(pt);
    sh = rint(sh);
  }
  cr->save();
  cr->translate(pt.x, pt.y);
  cr->rotate(O.angle);
  cr->move_to(sh);
  if (path) cr->text_path(O.name);
  else      cr->show_text(O.name);
  cr->restore();
}

GObj::ret_t
GObjMapDB::DrawingStep::draw(const CairoWrapper & cr, const dRect & range){

  ConvBase *cnv = mapdb_gobj->cnv.get();
  MapDB *map = mapdb_gobj->map.get();
  double osc = mapdb_gobj->obj_scale;

  std::set<uint32_t> ids;

  // calculate range for object selecting
  dRect sel_range(range);
  double exp_dist = 0;

  if (action == STEP_DRAW_TEXT){
    exp_dist = mapdb_gobj->max_text_size;
  }
  else {
    // expand by line width
    if (features.count(FEATURE_STROKE)){
      auto ftr = (FeatureStroke *)features.find(FEATURE_STROKE)->second.get();
      exp_dist = std::max(exp_dist, ftr->th*osc);
    }
    // expand by image size (image may be rotated and arbitrarly centered, use hypot(w,h) as size)
    if (features.count(FEATURE_IMG)){
      auto ftr = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
      exp_dist = std::max(exp_dist, osc*hypot(ftr->img.width(), ftr->img.height()));
    }
    // expand by move_to distance
    if (features.count(FEATURE_MOVETO)){
      auto ftr = (FeatureMoveTo *)features.find(FEATURE_MOVETO)->second.get();
      exp_dist = std::max(exp_dist, osc*ftr->dist);
    }
    // expand by lines bbox (again, can be rotated, at least for points)
    if (features.count(FEATURE_LINES)){
      auto ftr = (FeatureLines *)features.find(FEATURE_LINES)->second.get();
      exp_dist = std::max(exp_dist, osc*hypot(ftr->bbox.w, ftr->bbox.h));
    }
    // expand by circles bbox
    if (features.count(FEATURE_CIRCLES)){
      auto ftr = (FeatureCircles *)features.find(FEATURE_CIRCLES)->second.get();
      exp_dist = std::max(exp_dist, osc*hypot(ftr->bbox.w, ftr->bbox.h));
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

  // if SEL_RANGE feature exists, draw rectangles
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

  // Operator feature
  if (features.count(FEATURE_OP)){
    auto data = (FeatureOp *)features.find(FEATURE_OP)->second.get();
    cr->set_operator(data->op);
  }

  // pix_align feature
  bool pix_al = false;
  if (features.count(FEATURE_PIXAL)){
    auto data = (FeaturePixAl *)features.find(FEATURE_PIXAL)->second.get();
    pix_al = data->val;
  }

  // Font feature (set font + font size)
  if (features.count(FEATURE_FONT)){
    auto data = (FeatureFont *)features.find(FEATURE_FONT)->second.get();
    cr->set_font_size(osc*data->size);
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

  // Set up pattern feature
  if (features.count(FEATURE_PATT)){
    auto data = (FeaturePatt *)features.find(FEATURE_PATT)->second.get();
    if (features.count(FEATURE_IMG_FILTER)){
      auto data_f = (FeatureImgFilter *)features.find(FEATURE_IMG_FILTER)->second.get();
      data->patt->set_filter(data_f->flt);
    }
    data->patt->set_extend(Cairo::EXTEND_REPEAT);
    cr->set_source(data->patt);
  }

  // Set up image feature (points and areas)
  if (features.count(FEATURE_IMG)){
    auto data = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
    if (features.count(FEATURE_IMG_FILTER)){
      auto data_f = (FeatureImgFilter *)features.find(FEATURE_IMG_FILTER)->second.get();
      data->patt->set_filter(data_f->flt);
    }
    cr->set_source(data->patt);
  }

  // set up smooth feature
  double sm = 0;
  bool close = (action == STEP_DRAW_AREA);
  if (features.count(FEATURE_SMOOTH)){
    auto ftr = (FeatureSmooth *)features.find(FEATURE_SMOOTH)->second.get();
    sm = osc*ftr->dist;
  }

  // Set up fill feature
  if (features.count(FEATURE_FILL)){
    auto data = (FeatureFill *)features.find(FEATURE_FILL)->second.get();
    cr->set_color_a(data->col);
    cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
  }

  // Set up stroke feature
  if (features.count(FEATURE_STROKE)){
    // Setup dashed line
    if (features.count(FEATURE_DASH)){
      auto data = (FeatureDash *)features.find(FEATURE_DASH)->second.get();
      auto vd = data->vd;
      for (auto & d:vd) d*=osc;
      cr->set_dash(vd, 0);
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
    cr->set_line_width(osc*data->th);
  }


  // Draw each object
  for (auto const i: ids){
    auto O = map->get(i);
    if (!intersect(O.bbox(), sel_range)) continue;
    convert_coords(O);
    cr->begin_new_path();

    // Make drawing path for points, lines, areas
    if ((action == STEP_DRAW_POINT ||
         action == STEP_DRAW_LINE ||
         action == STEP_DRAW_AREA) &&
        (features.count(FEATURE_STROKE) ||
         features.count(FEATURE_FILL) ||
         features.count(FEATURE_PATT)) ){

      // make path for Lines or Circles
      if (features.count(FEATURE_LINES) ||
          features.count(FEATURE_CIRCLES)) {

        dMultiLine lines;
        if (features.count(FEATURE_LINES)){
          auto ftr = (FeatureLines *)features.find(FEATURE_LINES)->second.get();
          lines = osc*ftr->lines;
        }

        dLine circles;
        if (features.count(FEATURE_CIRCLES)){
          auto ftr = (FeatureCircles *)features.find(FEATURE_CIRCLES)->second.get();
          circles = osc*ftr->circles;
        }

        double dist = 0, dist_b = 0, dist_e = 0;
        FeatureDrawPos::pos_t pos = FeatureDrawPos::POINT;
        if (features.count(FEATURE_DRAW_POS)){
          auto ftr = (FeatureDrawPos *)features.find(FEATURE_DRAW_POS)->second.get();
          pos = ftr->pos;
          dist = osc*ftr->dist;
          dist_b = osc*ftr->dist_b;
          dist_e = osc*ftr->dist_e;
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
              lw.move_frw(dist_b);
              if (pos == FeatureDrawPos::EDIST) { // adjust distance
                double span = lw.length()- dist_b - dist_e;
                double n = floor(span/dist);
                dist = span/n;
                dist *= (1 - 1e-10); // to be sure that last element will be drawn
              }
              if (dist==0) break;
              while (lw.dist() <= lw.length()-dist_e){
                ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
                lw.move_frw(dist);
                if (lw.is_end()) break;
              }
              break;
          }
        }
        for (auto const &p:ref_points){
          cr->save();
          cr->translate(p.x, p.y);
          cr->rotate(O.angle + p.z);
          cr->mkpath_smline(lines, sm);
          for (auto const &c:circles){
            cr->move_to(c.x+c.z, c.y);
            cr->arc(c.x, c.y, c.z, 0, 2*M_PI);
          }
          cr->restore();
        }

      }
      // make path from original object
      else {
        cr->mkpath_smline(O, close, sm);
      }
    }

    // make path for TEXT objects
    if (action == STEP_DRAW_TEXT &&
        (features.count(FEATURE_STROKE) ||
         features.count(FEATURE_FILL) ||
         features.count(FEATURE_PATT)) ){
      draw_text(O, cr, range, true, pix_al);
    }

    // Pattern feature
    if (features.count(FEATURE_PATT)){
      auto data = (FeaturePatt *)features.find(FEATURE_PATT)->second.get();
      data->draw_patt(cr,osc,true);
    }

    // Fill feature
    // We want to set color for each object, because it can contain
    // fill+stroke+draw features with different colors
    if (features.count(FEATURE_FILL)){
      auto data = (FeatureFill *)features.find(FEATURE_FILL)->second.get();
      cr->set_color_a(data->col);
      cr->fill_preserve();
    }

    // Stroke feature
    if (features.count(FEATURE_STROKE)){
      auto data = (FeatureStroke *)features.find(FEATURE_STROKE)->second.get();
      cr->set_color_a(data->col);
      cr->stroke_preserve();
    }

    // Image feature (points and areas)
    if (features.count(FEATURE_IMG)){
      auto data = (FeaturePatt *)features.find(FEATURE_IMG)->second.get();
      for (auto const & l:O){
        if (action == STEP_DRAW_POINT) {
          for (dPoint p:l) data->draw_img(cr,p,O.angle,osc);
        }
        else {
          dPoint p = l.bbox().cnt();
          data->draw_img(cr,p,O.angle,osc);
        }
      }
    }

    // Write feature
    if (features.count(FEATURE_WRITE)){
      auto data = (FeatureWrite *)features.find(FEATURE_WRITE)->second.get();
      cr->begin_new_path(); // this is needed if we have WRITE+STROKE/FILL feateres
      cr->set_color(data->color);
      draw_text(O, cr, range, false, pix_al);
    }
  }

  // MAP drawing step:
  if (action == STEP_DRAW_MAP) {
    // Fill feature
    if (features.count(FEATURE_FILL)) cr->paint();

    // Pattern feature
    if (features.count(FEATURE_PATT)){
      auto data = (FeaturePatt *)features.find(FEATURE_PATT)->second.get();
      data->draw_patt(cr,osc,false);
    }
  }

  // BRD drawing step:
  if (action == STEP_DRAW_BRD && mapdb_gobj->border.size()>0) {
    dMultiLine brd(mapdb_gobj->border);
    if (cnv) brd = cnv->bck_acc(brd);
    cr->begin_new_path();
    cr->mkpath_smline(brd, true, sm);

    // Pattern feature
    if (features.count(FEATURE_PATT)){
      cr->mkpath(rect_to_line(expand(range,1.0))); // outer path
      cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
      auto data = (FeaturePatt *)features.find(FEATURE_PATT)->second.get();
      data->draw_patt(cr,osc,true);
    }

    // Fill feature
    // We want to set color for each object, because it can contain
    // fill+stroke+draw features with different colors
    if (features.count(FEATURE_FILL)){
      auto data = (FeatureFill *)features.find(FEATURE_FILL)->second.get();
      cr->mkpath(rect_to_line(expand(range,1.0))); // outer path
      cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
      cr->set_color_a(data->col);
      cr->fill_preserve();
    }

    // Stroke feature
    if (features.count(FEATURE_STROKE)){
      auto data = (FeatureStroke *)features.find(FEATURE_STROKE)->second.get();
      cr->set_color_a(data->col);
      cr->stroke_preserve();
    }
  }

  return GObj::FILL_PART;
}

/**********************************************************/
