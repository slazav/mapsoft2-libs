#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <string>
#include <deque>

#include "vmap2gobj.h"

#include "geom/line_walker.h"
#include "geom/poly_tools.h"
#include "read_words/read_words.h"
#include "geo_data/geo_io.h"
#include "geo_mkref/geo_mkref.h"
#include "filename/filename.h"
#include "geo_render/draw_grid.h"
//#include "conv_label.h"

using namespace std;

/**********************************************************/
void
ms2opt_add_vmap2_render(GetOptSet & opts){
  const char *g = "VMAP2_RENDER";
  opts.add("config", 1,'c',g, "Configuration file for vector map rendering. Default: /usr/share/mapsoft2/render.cfg");
  opts.add("define",      1,0,g, "Definitions for vector map rendering (json object)");
  opts.add("obj_scale",   1,0,g, "Rescaling factor for all objects, default 1.0.");
  opts.add("vmap_minsc", 1,0,g, "Minimum map scale (calculated from the 'natural' "
           "reference). Below it the map is drawn by with color "
           "(see --vmap2_minsc_color option). Default is 0.01");
  opts.add("fit_patt_size", 0,0,g, "Adjust pattern size to fit image size. "
           "This option is useful for generating tiled images. Default: false.");
  opts.add("vmap_minsc_color", 1,0,g, "Color to draw maps below minimum scale (see --vmap2_minsc). "
           "Default is 0xFFDB5A00).");
}
/**********************************************************/

// For conversion points->wgs get mean point size in m.
#include "geo_data/geo_utils.h"
double
get_ptsize(ConvBase & cnv, const dRect & r) {
  // calculate point size for this range
  dPoint p1 = r.tlc(), p1w = cnv.frw_pts(p1);
  dPoint p2 = r.trc(), p2w = cnv.frw_pts(p2);
  dPoint p3 = r.blc(), p3w = cnv.frw_pts(p3);

  return (geo_dist_2d(p1w,p2w)/dist(p1,p2) +
          geo_dist_2d(p1w,p3w)/dist(p1,p3))/2;
}

// set reference
void
GObjVMap2::set_ref(const GeoMap & r, bool set_ptsize) {
  ref = r;
  if (ref.empty()){
    if (set_ptsize) ptsize0 = 0;
    border.clear();
  }
  else {
    ConvMap cnv(ref);
    border = cnv.frw_acc(ref.border);
    if (set_ptsize) ptsize0 = get_ptsize(cnv, r.bbox());
  }
}

// set WGS border
void
GObjVMap2::set_brd(const dMultiLine & brd) {
  border = brd;
  if (ref.ref.size()==0) return;
  ConvMap cnv(ref);
  ref.border = cnv.bck_acc(border);
}


/**********************************************************/

GObjVMap2::GObjVMap2(VMap2 & map, const Opt &o): GObjMulti(false), map(map) {

  ptsize0 = 0;
  sc = 1.0;
  minsc       = o.get<double>("vmap_minsc", 0.01);
  minsc_color = o.get<uint32_t>("vmap_minsc_color", 0xFFDB5A00);
  obj_scale   = o.get<double>("obj_scale", 1.0);
  max_text_size = 1024;
  fit_patt_size = o.get<bool>("fit_patt_size", false);
  nsaved=0;

  opt = o;

  // Read configuration file.
  read_words_defs defs(o.get("define", Opt()));

  int depth = 0;
  std::string cfg = opt.get<string>("config", "/usr/share/mapsoft2/render.cfg");
  load_conf(cfg, defs, depth);
}

void
GObjVMap2::load_conf(const std::string & cfgfile, read_words_defs & defs, int & depth){

  std::string cfgdir = file_get_prefix(cfgfile); // for including images and other files

  ifstream ff(cfgfile);
  if (!ff) throw Err()
    << "GObjVMap2: can't open configuration file: " << cfgfile;

  int line_num[2] = {0,0}; // line counter for read_words
  std::shared_ptr<DrawingStep> st(NULL); // current step
  std::string ftr; // current feature
  std::deque<bool> ifs;  // for if/endif commands

  while (1){
    vector<string> vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;


    ftr = "";
    try{
      if (read_words_stdcmds(vs, defs, ifs)) continue;

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

      // set_ref, set_brd commands
      if (vs.size() > 1 &&
         (vs[0] == "set_ref" || vs[0] == "set_brd")) {
        st.reset(); // "+" should not work after the command
        GeoMap r;

        if (vs[1] == "file") {
          if (vs.size()!=3) throw Err()
            << "wrong number of arguments: " << vs[0] << " file <filename>";
          GeoData d;
          std::string fname = cfgdir + vs[2];
          read_geo(fname, d);
          if (vs[0] == "set_ref"){
            if (d.maps.size()<1 || d.maps.begin()->size()<1) throw Err()
              << "set_ref: can't read any reference from file: " << fname;
             r = (*d.maps.begin())[0];
          }
          if (vs[0] == "set_brd"){
            if (d.trks.size()<1) throw Err()
              << "set_brd: can't read any track from file: " << fname;
            border = *d.trks.begin();
            continue; // no need to convert border and modify ref
          }
        }
        else if (vs[1] == "nom") {
          if (vs.size()!=4) throw Err()
            << "wrong number of arguments: " << vs[0] << " nom <name> <dpi>";
          Opt o;
          o.put("mkref", "nom");
          o.put("name", vs[2]);
          o.put("dpi", vs[3]);
          r = geo_mkref_opts(o);
        }
        else if (vs[1] == "nom_fi") {
          if (vs.size()!=4) throw Err()
            << "wrong number of arguments: " << vs[0] << " nom <name> <dpi>";
          Opt o;
          o.put("mkref", "nom_fi");
          o.put("name", vs[2]);
          o.put("dpi", vs[3]);
          r = geo_mkref_opts(o);
        }
        else if (vs[1] == "none") {
          if (vs.size()!=2) throw Err()
            << "wrong number of arguments: " << vs[0] << " none";
          // do nothing, r is empty
        }
        else throw Err() << vs[0] << "command: 'file', 'nom', or 'none' word expected";

        if (vs[0] == "set_ref") ref = r;
        if (vs[0] == "set_brd") ref.border = r.border;
        set_ref(ref, true); // convert border, update ptsize0

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

      // fit_patt_size command
      if (vs[0] == "fit_patt_size") {
        st.reset(); // "+" should not work after the command
        if (vs.size()!=2) throw Err()
            << "wrong number of arguments: fit_patt_size 0|1";
        fit_patt_size = str_to_type<bool>(vs[1]);
        continue;
      }

      // minsc command
      if (vs[0] == "minsc") {
        st.reset(); // "+" should not work after the command
        if (vs.size()!=2) throw Err()
            << "wrong number of arguments: minsc <number>";
        minsc = str_to_type<double>(vs[1]);
        continue;
      }

      // minsc_color command
      if (vs[0] == "minsc_color") {
        st.reset(); // "+" should not work after the command
        if (vs.size()!=2) throw Err()
            << "wrong number of arguments: minsc_color <color>";
        minsc_color = str_to_type<uint32_t>(vs[1]);
        continue;
      }

      // define command
      if (vs[0] == "define") {
        st.reset(); // "+" should not work after the command
        if (vs.size()!=3) throw Err()
            << "wrong number of arguments: define <name> <definition>";
        defs.define(vs[1],vs[2]);
        continue;
      }

      // obj_scale
      if (vs[0] == "obj_scale") {
        st.reset(); // "+" should not work after the command
        if (vs.size()!=2) throw Err()
            << "wrong number of arguments: obj_scale <value>";
        obj_scale = str_to_type<double>(vs[1]);
        continue;
      }



      /**********************************************************/
      /// Commands with features

      // draw an object (point, line, area)
      if (vs.size() > 1 && vs[0].find(':')!=std::string::npos) {
        st.reset(new DrawingStep(this));
        st->etype = VMap2obj::make_type(vs[0]);
        switch (st->etype >> 24) {
          case VMAP2_POINT:   st->action = STEP_DRAW_POINT; break;
          case VMAP2_LINE:    st->action = STEP_DRAW_LINE; break;
          case VMAP2_POLYGON: st->action = STEP_DRAW_AREA; break;
          case VMAP2_TEXT:    st->action = STEP_DRAW_TEXT; break;
          default: throw Err() << "Bad map object type: " << hex << st->etype;
        }
        st->step_name = vs[0];
        ftr = vs[1];
        vs.erase(vs.begin(), vs.begin()+2);
        add(depth--, st);
      }

      // draw on the whole map
      else if (vs.size() > 1 && vs[0] == "map") {
        st.reset(new DrawingStep(this));
        st->action = STEP_DRAW_MAP;
        st->step_name = vs[0];
        ftr = vs[1];
        vs.erase(vs.begin(), vs.begin()+2);
        add(depth--, st);
      }

      // draw border
      else if (vs.size() > 1 && vs[0] == "brd") {
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
        st->check_args(vs, {"<color>", "<line width>"});
        st->do_stroke = true;
        st->stroke_color = str_to_type<uint32_t>(vs[0]);
        st->thickness    = str_to_type<double>(vs[1]);
        continue;
      }

      // fill <color>
      if (ftr == "fill"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA |
                       STEP_DRAW_MAP | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->check_args(vs, {"<fill color>"});
        st->do_fill = true;
        st->fill_color = str_to_type<uint32_t>(vs[0]);
        continue;
      }

      // clip
      if (ftr == "clip"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_MAP |
                       STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->check_args(vs, {});
        st->do_clip = true;
        continue;
      }

      // patt <file> <scale>
      if (ftr == "patt"){
        st->check_type(STEP_DRAW_LINE | STEP_DRAW_AREA | STEP_DRAW_MAP |
                       STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->check_args(vs, {"<file>", "<scale>"});
        auto fname = vs[0];
        double sc = str_to_type<double>(vs[1]);
        if (fname.size()==0) throw Err() << "empty image filename";
        if (fname[0]!='/') fname = cfgdir + fname;
        st->do_patt = true;
        st->patt = ImageRenderer(fname,sc,0,0);
        continue;
      }

      // img <file> <scale>
      if (ftr == "img"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_POINT);
        st->check_args(vs, {"<file>", "<scale>", "?<dx>", "?<dy>"});
        auto fname = vs[0];
        double sc = str_to_type<double>(vs[1]);
        double dx = vs.size()>2 ? str_to_type<double>(vs[2]):0;
        double dy = vs.size()>3 ? str_to_type<double>(vs[3]):0;
        if (fname.size()==0) throw Err() << "empty image filename";
        if (fname[0]!='/') fname = cfgdir + fname;
        st->do_img = true;
        st->img = ImageRenderer(fname, sc, dx,dy);
        continue;
      }

      // img_filter fast|good|best|nearest|bilinear
      if (ftr == "img_filter"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_POINT |
                       STEP_DRAW_MAP | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->check_args(vs, {"fast|good|best|nearest|bilinear"});
        st->img_filter = str_to_type<Cairo::Filter>(vs[0]);
        continue;
      }

      // smooth <distance>
      if (ftr == "smooth"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_BRD);
        st->check_args(vs, {"<distance>"});
        st->smooth = str_to_type<double>(vs[0]);
        continue;
      }

      // dash <length1> <length2> ...
      if (ftr == "dash"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->check_args(vs, {"<dist>", "..."});
        st->dash.clear();
        for (auto const & s:vs)
          st->dash.push_back(str_to_type<double>(s));
        continue;
      }

      // cap round|butt|square
      if (ftr == "cap"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->check_args(vs, {"round|butt|square"});
        st->line_cap = str_to_type<Cairo::LineCap>(vs[0]);
        continue;
      }

      // join round|miter
      if (ftr == "join"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE | STEP_DRAW_TEXT | STEP_DRAW_BRD);
        st->check_args(vs, {"miter|round"});
        st->line_join = str_to_type<Cairo::LineJoin>(vs[0]);
        continue;
      }

      // operator <op>
      if (ftr == "operator"){
        st->check_type(STEP_DRAW_AREA | STEP_DRAW_LINE |
                       STEP_DRAW_POINT | STEP_DRAW_TEXT |
                       STEP_DRAW_MAP | STEP_DRAW_BRD);
        st->check_args(vs, {"clear|source|over|in|out|atop|dest|"
            "dest_over|dest_in|dest_out|dest_atop|xor|add|saturate"});
        st->op = str_to_type<Cairo::Operator>(vs[0]);
        continue;
      }

      // pix_align <val>
      if (ftr == "pix_align"){
        st->check_type( STEP_DRAW_TEXT);
        st->check_args(vs, {"<val>"});
        st->pix_align = str_to_type<bool>(vs[0]);
        continue;
      }

      // outer
      if (ftr == "outer"){
        st->check_type( STEP_DRAW_BRD);
        st->check_args(vs, {});
        st->outer = true;
        continue;
      }

      // lines <lines> ...
      if (ftr == "lines"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA);
        st->check_args(vs, {"<lines>", "..."});
        for (auto const & s:vs){
          dMultiLine ml(s);
          st->add_lines.insert(st->add_lines.end(), ml.begin(), ml.end());
          st->add_bbox.expand(ml.bbox());
        }
        continue;
      }

      // circles <circle> ...
      if (ftr == "circles"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA);
        st->check_args(vs, {"<circle>", "..."});
        for (auto const & s:vs){
          dPoint p(s);
          if (p.z<=0) throw Err() << "positive radius expected";
          st->add_circles.push_back(p);
          st->add_bbox.expand(p+dPoint(p.z,p.z));
          st->add_bbox.expand(p-dPoint(p.z,p.z));
        }
        continue;
      }

      // draw_pos (point|begin|end)
      // draw_pos dist <dist> [<dist_begin>]
      // draw_pos edist <dist> [<dist_begin>] [<dist_end>]
      // draw_pos fill <w> <h>
      if (ftr == "draw_pos"){
        st->check_type(STEP_DRAW_LINE | STEP_DRAW_AREA);
        if (vs.size()<1) throw Err() << "argument expected";
        if (vs[0] == "point"){
           st->check_args(vs, {"point"});
           st->draw_pos = DrawingStep::DRAW_POS_POINT;
        }
        else if (vs[0] == "begin"){
          st->check_args(vs, {"begin"});
          st->draw_pos = DrawingStep::DRAW_POS_BEGIN;
        }
        else if (vs[0] == "end"){
          st->check_args(vs, {"end"});
          st->draw_pos = DrawingStep::DRAW_POS_END;
        }
        else if (vs[0] == "dist" || vs[0] == "edist") {
          st->check_args(vs, {"(dist|edist)", "dist", "?dist_b", "?dist_e"});
          st->draw_pos = (vs[0] == "dist") ? DrawingStep::DRAW_POS_DIST : DrawingStep::DRAW_POS_EDIST;
          st->draw_pos_dist = str_to_type<double>(vs[1]);
          if (vs.size()>2) st->draw_pos_b = str_to_type<double>(vs[2]);
          else st->draw_pos_b = st->draw_pos_dist/2;
          if (vs.size()>3) st->draw_pos_e = str_to_type<double>(vs[3]);
          else st->draw_pos_e = st->draw_pos_dist/2;
        }
        else if (vs[0] == "fill") {
          st->check_args(vs, {"fill", "w", "h"});
          st->draw_pos = DrawingStep::DRAW_POS_FILL;
          st->draw_pos_w = str_to_type<size_t>(vs[1]);
          st->draw_pos_h = str_to_type<size_t>(vs[2]);
        }
        else throw Err() << "Wrong position. Possible values: "
          "point, begin, end, dist, edist.";
        continue;
      }

      // sel_range
      if (ftr == "sel_range"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE |
                       STEP_DRAW_AREA | STEP_DRAW_TEXT);
        st->check_args(vs, {"<color>", "<line width>"});
        st->do_sel_range = true;
        st->sel_range_color = str_to_type<uint32_t>(vs[0]);
        st->sel_range_thickness = str_to_type<double>(vs[1]);
        continue;
      }

      // move_to <max distance> <type>
      if (ftr == "move_to"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA);
        st->check_args(vs, {"<dist>", "(area|line|point):<type>", "..."});
        st->move_to_dist = str_to_type<double>(vs[0]);
        for (size_t i=1; i<vs.size(); ++i)
          st->move_to_targets.insert(VMap2obj::make_type(vs[i]));
        st->move_to_rot = false;
        continue;
      }

      // rotate_to <max distance> <type>
      if (ftr == "rotate_to"){
        st->check_type(STEP_DRAW_POINT);
        st->check_args(vs, {"<dist>", "(area|line):<type>", "..."});
        st->move_to_dist = str_to_type<double>(vs[0]);
        for (size_t i=1; i<vs.size(); ++i)
          st->move_to_targets.insert(VMap2obj::make_type(vs[i]));
        st->move_to_rot = true;
        continue;
      }

      // move_from <max distance> <type>
      if (ftr == "move_from"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE | STEP_DRAW_AREA);
        st->check_args(vs, {"<dist>", "(area|line|point):<type>", "..."});
        st->move_from_dist = str_to_type<double>(vs[0]);
        for (size_t i=1; i<vs.size(); ++i)
          st->move_from_targets.insert(VMap2obj::make_type(vs[i]));
        continue;
      }

      // rotate <angle,deg>
      if (ftr == "rotate"){
        st->check_type(STEP_DRAW_POINT | STEP_DRAW_LINE |
                       STEP_DRAW_AREA | STEP_DRAW_TEXT);
        st->check_args(vs, {"<val>"});
        st->rotate = str_to_type<double>(vs[0]) * M_PI/180.0;
        continue;
      }

      // font <size> <font>
      if (ftr == "font"){
        st->check_type(STEP_DRAW_TEXT);
        st->check_args(vs, {"<font size>", "<font>"});
        st->font_size = str_to_type<double>(vs[0]);
        st->font = vs[1];
        continue;
      }

      // write <color>
      if (ftr == "write"){
        st->check_type(STEP_DRAW_TEXT);
        st->check_args(vs, {"<color>"});
        st->do_write = true;
        st->write_color = str_to_type<uint32_t>(vs[0]);
        continue;
      }

      // pulk_grid <color> <line width>
      if (ftr == "pulk_grid"){
        st->check_type(STEP_DRAW_MAP);
        st->check_args(vs, {"<step>", "<color>", "<line width>"});
        st->do_pulk_grid = true;
        st->grid_opts["grid_step"]  = vs[0];
        st->grid_opts["grid_color"] = vs[1];
        st->grid_opts["grid_thick"] = vs[2];
        continue;
      }

      // fi_grid <color> <line width>
      if (ftr == "fi_grid"){
        st->check_type(STEP_DRAW_MAP);
        st->check_args(vs, {"<step>", "<color>", "<line width>"});
        st->do_fi_grid = true;
        st->grid_opts["grid_step"]  = vs[0];
        st->grid_opts["grid_color"] = vs[1];
        st->grid_opts["grid_thick"] = vs[2];
        continue;
      }

      // grid_labels <size> <font> <color>
      if (ftr == "grid_labels"){
        st->check_type(STEP_DRAW_MAP);
        st->check_args(vs, {"<size>", "<font>", "<color>"});
        st->grid_opts["grid_text_size"] = vs[0];
        st->grid_opts["grid_text_font"]  = vs[1];
        st->grid_opts["grid_text_color"] = vs[2];
        continue;
      }

      // group <name>
      if (ftr == "group"){
        st->check_args(vs, {"<group>"});
        st->group_name = vs[0];

        // if the group is not in the group list, add it there
        bool known_group = false;
        for (auto const & gr:groups)
          if (gr == st->group_name){ known_group = true; break;}
        if (!known_group) groups.push_back(st->group_name);
        continue;
      }

      // name <name>
      if (ftr == "name"){
        st->check_args(vs, {"<name>"});
        st->step_name = vs[0];
        continue;
      }

      // short_expand <length>
      if (ftr == "short_expand"){
        st->check_type(STEP_DRAW_LINE);
        st->check_args(vs, {"<length>"});
        st->short_expand = str_to_type<double>(vs[0]);
        continue;
      }

      // short_skip <length>
      if (ftr == "short_skip"){
        st->check_type(STEP_DRAW_LINE);
        st->check_args(vs, {"<length>"});
        st->short_expand = str_to_type<double>(vs[0]);
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

#include "geom/poly_tools.h"
// change object coordinates according to features
// change object angle to radians
void
GObjVMap2::DrawingStep::convert_coords(VMap2obj & O){

  ConvBase *cnv = gobj->cnv.get();

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

  if (cnv) cnv->bck(O);

  // move_to / rotate_to
  if (move_to_targets.size()>0){
    for (auto & l:O){ // segments
      for (auto & p:l){ // points
        dRect r(p,p);
        r.expand(move_to_dist);
        if (cnv) r = cnv->frw_acc(r);

        dMultiLine lines;
        for (auto & t: move_to_targets){
          auto ids = gobj->map.find(t, r);
            for (int i:ids){
            auto O1 = gobj->map.get(i);
            if (cnv) cnv->bck(O1);
            for (auto const & l:O1)
              lines.push_back(l);
          }
        }
        dPoint t(1,0);
        nearest_pt(lines, t, p, move_to_dist);
        if (move_to_rot) O.angle = atan2(t.y, t.x);
      }
    }
  }

  // move_from
  if (move_from_targets.size()>0){
    for (auto & l:O){ // segments
      for (auto & p:l){ // points
        dRect r(p,p);
        r.expand(move_from_dist);
        if (cnv) r = cnv->frw_acc(r);

        dMultiLine lines;
        for (auto & t: move_from_targets){
          auto ids = gobj->map.find(t, r);
            for (int i:ids){
            auto O1 = gobj->map.get(i);
            if (cnv) cnv->bck(O1);
            for (auto const & l:O1)
              lines.push_back(l);
          }
        }
        dPoint t(1,0);
        dPoint p1(p);
        nearest_pt(lines, t, p1, move_from_dist);
        if (p1!=p) p = p1 + (p-p1)*move_from_dist/dist2d(p1,p);
      }
    }
  }

  // additional rotation
  O.angle += rotate;

  // Should be always finite for Cairo:
  if (std::isnan(O.angle)) O.angle = 0;

  // skip short lines
  if (short_skip > 0){
    for (auto l = O.begin(); l!=O.end();){
      if (l->length2d() < short_skip) l=O.erase(l);
      else ++l;
    }
  }

  // expand short lines
  if (short_expand>0){
    for (auto l = O.begin(); l!=O.end(); ++l){
      if (l->size()<2) continue;
      if (l->length2d() >= short_expand) continue;
      auto n = l->size()-1;
      auto p0 = (*l)[0], p1=(*l)[1]; // saving points (line could have 1 segment)
      auto p2 = (*l)[n-1], p3=(*l)[n];
      auto l1 = dist2d(p0,p1);   // first segment
      auto l2 = dist2d(p2,p3); // last segment
      if (l1+l2 > 0){
        auto dl = short_expand - l->length2d();
        (*l)[0] = p0 + (p0-p1) * dl/(l1+l2);
        (*l)[n] = p3 + (p3-p2) * dl/(l1+l2);
      }
      else {
        (*l)[0] = p0 + dPoint(1,0) * short_expand;
        (*l)[n] = p3 - dPoint(1,0) * short_expand;
      }
    }
  }

}


// Draw a text object. Used in two places: in `write` feature
// (with parameter path=false) and for making text path in
// `fill`/`stroke`/`patt` features (with path=true).
// `range` parameter is used for check if we should draw the object.
void
GObjVMap2::DrawingStep::draw_text(VMap2obj & O, const CairoWrapper & cr, const dRect & range, bool path){
  if (O.size()==0 || O[0].size()==0) return; // no coordinates
  dPoint pt = O[0][0];
//  auto txt = conv_label(O.name);
  auto txt = O.name;
  dRect ext = cr->get_text_extents(txt.c_str());
  // To allow any rotated/align text do be in the range use diagonal
  dRect rng = expand(dRect(pt,pt), hypot(ext.w, ext.h)*(double)O.scale);
  if (!intersect(rng, range)) return;

  dPoint sh;
  switch (O.align){
    case VMAP2_ALIGN_SW: break;
    case VMAP2_ALIGN_W:  sh.y=ext.h/2; break;
    case VMAP2_ALIGN_NW: sh.y=ext.h;   break;
    case VMAP2_ALIGN_N:  sh.y=ext.h;   sh.x=-ext.w/2; break;
    case VMAP2_ALIGN_NE: sh.y=ext.h;   sh.x=-ext.w;   break;
    case VMAP2_ALIGN_E:  sh.y=ext.h/2; sh.x=-ext.w;   break;
    case VMAP2_ALIGN_SE:               sh.x=-ext.w;   break;
    case VMAP2_ALIGN_S:                sh.x=-ext.w/2; break;
    case VMAP2_ALIGN_C:  sh.y=ext.h/2; sh.x=-ext.w/2; break;
  }
  if (pix_align) {
    pt = rint(pt);
    sh = rint(sh);
  }
  cr->save();
  cr->translate(pt.x, pt.y);
  cr->rotate(O.angle);
  cr->scale(O.scale, O.scale);
  cr->move_to(sh);
  if (path) cr->text_path(txt);
  else      cr->show_text(txt);
  cr->restore();
}

void
GObjVMap2::DrawingStep::setup_ctx(const CairoWrapper & cr, const double osc){
  // Operator feature
  cr->set_operator(op);

  // Font feature (set font + font size)
  if (font!=""){
    cr->set_font_size(osc*font_size);
    // For work with patterns see:
    // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x103.html#AEN242
    // For font properties see:
    // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html
    // https://www.freedesktop.org/software/fontconfig/fontconfig-user.html
    // https://wiki.archlinux.org/index.php/Font_configuration
    FcPattern *patt = FcNameParse((const FcChar8 *)font.c_str());
    cr->set_font_face(Cairo::FtFontFace::create(patt));
    FcPatternDestroy(patt);
  }

  // Set up pattern feature
  if (do_patt){
    patt.set_filter(img_filter);
  }

  // Set up image feature (points and areas)
  if (do_img){
    img.set_filter(img_filter);
  }

  // Always using this rule (for fill, pattern, clip)
  cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);

  // Set up stroke
  if (do_stroke){
    // Setup dashed line
    if (dash.size()){
      auto vd = dash;
      for (auto & d:vd) d*=osc;
      cr->set_dash(vd, 0);
    }
    else
      cr->unset_dash();

    // Setup line cap, line join, line thickness
    cr->set_color_a(stroke_color);
    cr->set_line_cap(line_cap);
    cr->set_line_join(line_join);
    cr->set_line_width(osc*thickness);
  }
}

GObj::ret_t
GObjVMap2::DrawingStep::draw(const CairoWrapper & cr, const dRect & range){

  ConvBase *cnv = gobj->cnv.get();
  double osc = gobj->obj_scale;
  if (gobj->sc!=0) osc *= gobj->sc;

  std::set<uint32_t> ids;

  // calculate range for object selecting
  dRect sel_range(range);
  double exp_dist = 0;

  if (action == STEP_DRAW_TEXT){
    exp_dist = gobj->max_text_size*osc;
  }
  else {
    // expand by line expand length
    if (short_expand>0){
      exp_dist = std::max(exp_dist, short_expand/2);
    }
    // expand by line width
    if (stroke_color!=0){
      exp_dist = std::max(exp_dist, thickness*osc);
    }
    // expand by image size (image may be rotated and arbitrarly
    // centered, use hypot(w,h) as size)
    if (do_img){
      exp_dist = std::max(exp_dist, osc*hypot(img.width(), img.height()));
    }
    // expand by move_to distance
    exp_dist = std::max(exp_dist, move_to_dist);

    // expand by lines/circles bbox (again, can be rotated, at least for points)
    exp_dist = std::max(exp_dist, osc*hypot(add_bbox.w, add_bbox.h));
  }
  sel_range.expand(exp_dist);

  // convert to wgs84
  if (cnv) sel_range = cnv->frw_acc(sel_range);

  // Select objects in the range.
  if (action == STEP_DRAW_POINT ||
      action == STEP_DRAW_LINE ||
      action == STEP_DRAW_AREA ||
      action == STEP_DRAW_TEXT){

    ids = gobj->map.find(etype, sel_range);
    if (ids.size()==0){
      // set empty clipping range if needed
      if (do_clip && action == STEP_DRAW_AREA) {
        if (gobj->nsaved>0) { cr->restore(); gobj->nsaved--;}
        cr->save(); gobj->nsaved++;
        cr->begin_new_path();
        cr->clip();
      }
      return GObj::FILL_NONE;
    }
  }

  // if SEL_RANGE feature exists, draw rectangles
  if (do_sel_range) {
    cr->begin_new_path();
    cr->set_color_a(sel_range_color);
    cr->set_line_width(sel_range_thickness);
    for (auto const i: ids){
      auto O = gobj->map.get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      dRect box = cnv->bck_acc(O.bbox()); //to points
      box.expand(exp_dist);
      cr->mkpath(rect_to_line(box), true);
    }
    cr->stroke();
  }

  // set up smooth feature
  double sm = osc*smooth;

  // for building paths
  bool close = (action == STEP_DRAW_AREA);

  // Set up drawing context
  setup_ctx(cr, osc);

  // Fill with line/circle objects
  if (draw_pos == DRAW_POS_FILL){
    // make new cairo surface
    CairoWrapper cr2;
    ImageR img(draw_pos_w,draw_pos_h, IMAGE_32ARGB);
    img.fill32(0);
    cr2.set_surface_img(img);

    // setup context
    setup_ctx(cr2, osc);

    //draw lines/circles
    cr2->mkpath_smline(osc*add_lines, sm);
    for (auto const &c:osc*add_circles){
      cr2->move_to(c.x+c.z, c.y);
      cr2->arc(c.x, c.y, c.z, 0, 2*M_PI);
    }
    if (do_fill){
      cr2->set_color_a(fill_color);
      cr2->fill_preserve();
    }
    if (do_stroke){
      cr2->set_color_a(stroke_color);
      cr2->stroke_preserve();
      }
    patt = ImageRenderer(img);
  }

  // Draw each object
  for (auto const i: ids){
    auto O = gobj->map.get(i);
    if (!intersect(O.bbox(), sel_range)) continue;
    convert_coords(O);
    cr->begin_new_path();

    // Make drawing path for points, lines, areas
    if ((action == STEP_DRAW_POINT ||
         action == STEP_DRAW_LINE ||
         action == STEP_DRAW_AREA) &&
        (do_stroke || do_fill || do_patt )){

      // make path for original object
      if ((add_lines.size()==0 && add_circles.size()==0) ||
          draw_pos == DRAW_POS_FILL) {
        cr->mkpath_smline(O, close, sm);
      }
      // make path for Lines or Circles
      else {
        // make reference points (x,y,angle)
        dLine ref_points;
        for (auto const & l:O){
          LineWalker lw(l, close);
          switch(draw_pos){
            case DRAW_POS_POINT:
              lw.move_begin();
              while (!lw.is_end()){
                ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
                lw.move_frw_to_node();
              }
              if (!close)
                ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
              break;
            case DRAW_POS_BEGIN:
              lw.move_begin();
              ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
              break;
            case DRAW_POS_END:
              lw.move_end();
              ref_points.push_back(dPoint(lw.pt().x, lw.pt().y, lw.ang()));
              break;
            case DRAW_POS_DIST:
            case DRAW_POS_EDIST:
            {
              double dist = osc*draw_pos_dist;
              double dist_b = osc*draw_pos_b;
              double dist_e = osc*draw_pos_e;

              lw.move_begin();
              lw.move_frw(dist_b);
              if (draw_pos == DRAW_POS_EDIST) { // adjust distance
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
        }

        // make paths
        dMultiLine lines = osc*add_lines;
        dLine circles = osc*add_circles;
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
    }

    // make path for TEXT objects
    if (action == STEP_DRAW_TEXT &&
        (do_stroke || do_fill || do_clip || do_patt) ){
      draw_text(O, cr, range, true);
    }

    // Pattern feature
    if (do_patt || draw_pos == DRAW_POS_FILL){
      double scx = osc, scy = osc;
      if (gobj->fit_patt_size) {
        double nx = range.w/patt.w/patt.sc0;
        double ny = range.h/patt.h/patt.sc0;
        scx = nx/rint(nx/osc);
        scy = ny/rint(ny/osc);
      }
      patt.draw_patt(cr,scx,scy,true);
    }

    // Fill feature
    // We want to set color for each object, because it can contain
    // fill+stroke+draw features with different colors
    if (do_fill && draw_pos != DRAW_POS_FILL){
      cr->set_color_a(fill_color);
      cr->fill_preserve();
    }


    // Stroke feature
    if (do_stroke && draw_pos != DRAW_POS_FILL){
      cr->set_color_a(stroke_color);
      cr->stroke_preserve();
    }

    // Image feature (points and areas)
    if (do_img){
      for (auto const & l:O){
        if (action == STEP_DRAW_POINT) {
          for (dPoint p:l) img.draw_img(cr,p,O.angle,osc);
        }
        else {
          dPoint p = l.bbox().cnt();
          img.draw_img(cr,p,O.angle,osc);
        }
      }
    }

    // Write feature
    if (do_write){
      cr->begin_new_path(); // this is needed if we have WRITE+STROKE/FILL feateres
      cr->set_color(write_color);
      draw_text(O, cr, range, false);
    }
  }

  // clip to areas: make a single path for all objects:
  if (do_clip && action == STEP_DRAW_AREA) {
    if (gobj->nsaved>0) { cr->restore(); gobj->nsaved--;}
    cr->save(); gobj->nsaved++;
    cr->begin_new_path();
    for (auto const i: ids){
      auto O = gobj->map.get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      convert_coords(O);
      cr->mkpath_smline(O, close, sm);
    }
    cr->clip();
  }

  // clip to text: make a single path for all objects:
  if (do_clip && action == STEP_DRAW_TEXT) {
    if (gobj->nsaved>0) { cr->restore(); gobj->nsaved--;}
    cr->save(); gobj->nsaved++;
    cr->begin_new_path();
    for (auto const i: ids){
      auto O = gobj->map.get(i);
      if (!intersect(O.bbox(), sel_range)) continue;
      convert_coords(O);
      draw_text(O, cr, range, true);
    }
    cr->clip();
  }

  // MAP drawing step:
  if (action == STEP_DRAW_MAP) {

    // Pattern
    if (do_patt){
      double scx = osc, scy = osc;
      if (gobj->fit_patt_size) { // move to patt.draw_patt
        double nx = range.w/patt.w/patt.sc0;
        double ny = range.h/patt.h/patt.sc0;
        scx = nx/rint(nx/osc);
        scy = ny/rint(ny/osc);
      }
      patt.draw_patt(cr,scx,scy,false);
    }
    // Fill feature
    if (do_fill){
      cr->set_color_a(fill_color);
      cr->paint();
    }

    // PulkGrid feature
    if (do_pulk_grid){
      draw_pulk_grid(cr, dPoint(), *cnv, grid_opts);
    }

    // FiGrid feature
    if (do_fi_grid){
      draw_fi_grid(cr, dPoint(), *cnv, grid_opts);
    }

    // Clip feature - just restore previous clip
    if (do_clip) if (gobj->nsaved>0) { cr->restore(); gobj->nsaved--;}
  }

  // BRD drawing step:
  if (action == STEP_DRAW_BRD && gobj->border.size()>0) {

    if (do_clip){
      if (gobj->nsaved>0) { cr->restore(); gobj->nsaved--;}
      cr->save(); gobj->nsaved++;
    }

    dMultiLine brd(gobj->border);
    if (cnv) brd = cnv->bck_acc(brd);
    cr->begin_new_path();
    cr->mkpath_smline(brd, true, sm);

    // outer path (FILL_RULE_EVEN_ODD is set)
    if (outer)
      cr->mkpath(rect_to_line(expand(range,1.0)));

    // Pattern
    if (do_patt){
      double scx = osc, scy = osc;
      if (gobj->fit_patt_size) { // move to patt.draw_patt
        double nx = range.w/patt.w/patt.sc0;
        double ny = range.h/patt.h/patt.sc0;
        scx = nx/rint(nx/osc);
        scy = ny/rint(ny/osc);
      }
      patt.draw_patt(cr,scx,scy,true);
    }

    // Fill
    if (do_fill){
      cr->set_color_a(fill_color);
      cr->fill_preserve();
    }

    // Stroke
    if (do_stroke){
      cr->set_color_a(stroke_color);
      cr->stroke_preserve();
    }

    // Clip
    if (do_clip) cr->clip();
  }

  return GObj::FILL_PART;
}

/**********************************************************/


GObj::ret_t
GObjVMap2::draw(const CairoWrapper & cr, const dRect & draw_range) {

  // calculate scaling for this range
  sc = ptsize0/get_ptsize(*cnv, draw_range);
  if (sc!=0 && sc < minsc){
    cr->set_color_a(minsc_color);
    cr->paint();
    return GObj::FILL_PART;
  }

  // we want to track save/restore pairs here
  nsaved=0;
  auto ret = GObjMulti::draw(cr, draw_range);
  while (nsaved>0){ cr->restore(); nsaved--; }
  return ret;
}
