#ifndef GOBJ_MAPDB_H
#define GOBJ_MAPDB_H

#include <list>
#include <vector>
#include <string>
#include <map>

#include "cairo/cairo_wrapper.h"
#include "image/image_colors.h"
#include "viewer/gobj_multi.h"
#include "filename/filename.h"
#include "image/image_r.h"
#include "geo_data/geo_data.h"
#include "geo_data/conv_geo.h"
#include "opt/opt.h"

#include "mapdb.h"

/*

Drawing of a vector map is a sequence of "drawing steps". Each step
contains drawing of a single object type, of a grid, or setting
some global parameter (like map reference).
The configuration file contains description of each step.

    (point|line|area) <type> <feature> <options> ...
    + <feature> <options> ...
    + <feature> <options> ...
    ...
    (map|brd) <feature> <options> ...
    + <feature> <options> ...
    ...

where possible features are:

    stroke <color> <width> -- Draw a point, a line or an area contour.
    fill <color> -- Fill an area or the map.
    clip         -- After drawing set clipping region for the path (area, text, map, border).
    patt <image file> <scale> [<dx>] [<dy>] -- Fill an area with the pattern.
    img  <image file> <scale> [<dx>] [<dy>] -- Draw an image (point or area)
    img_filter <flt>  -- Set image filter fast|good|best|nearest|bilinear
    smooth <distance> -- Use smoothed paths.
    dash <len1> ...   -- Setup dashed line. Valid for lines and areas together with stroke feature.
    cap  round|butt|square -- Set line cup (default round). Use with stroke feature.
    join round|miter       -- Set line join (default round). Use with stroke feature.
    operator <op>          -- Set drawing operator (clear|source|over|in|out|atop|dest|
                              dest_over|dest_in|dest_out|dest_atop|xor|add|saturate)
    text_align (0|1)       -- align text positions to integer pixels (default: 0)
    lines <lines> ...      -- draw lines
    circles <circle> ...   -- draw circles
    draw_pos (point|begin|end) -- reference positions for lines/circles features:
                              at each point, at the beginning, at the end
    draw_pos (dist|edist) <dist> [<dist_begin>] [<dist_end>] -- reference positions for lines/circles features:
                              at dist period, starting at dist_begin (defaults: dist/2) from the first point
                              and ending not further then dist_end from the last point.
                              For `edist` distanse will be adjusted to have exact begin and end distances.
    draw_dist <dist> [<dist0>] -- distances for dist/edist position
    sel_range <color> <width>  -- draw selection range for each object
    move_to (line|area) <type> <max distance> -- move point to the nearest line or area object
    rotate_to (line|area) <type> <max distance> -- move and rotate point to the nearest line or area object
    rotate <angle>         -- rotate object pictures (img, lines, circles), degrees, cw
    font <size> <font>     -- set font (fontconfig pattern)
    write <color>          -- render the text (a bit different from `fill`)
    group <name>           -- set group name for a drawing step
    name  <name>           -- set name for a drawing step

If `stroke`, `fill` and `patt` `img` features exists together then the drawing
order is following: pattern, then fill, then stroke, then img.

other commands in the configuration file

    set_ref file <filename> -- set map reference including border from a file
    set_ref nom <name> <dpi> -- set map reference and border as a Soviet nomenclature name
    set_brd file <filename> -- set map border from track file
    max_text_size <value> -- set max_text_size value for selecting text objects in the database
    define <name> <definition> -- redefine a word (substitution works on full words and done once)
    if <word1> (==|!=) <word2>, else, endif --  if statement. If condition is true/false
       text between if and endif is is processed/ignored. One can use nested if-endif commands.
       Command else just inverts condition of the last if command.
    include <file> -- Read another configuration file.
       Image paths are calculated with respect to the current file.

*/

/********************************************************************/
#include "getopt/getopt.h"

// add MS2OPT_DRAWMAPDB options
void ms2opt_add_mapdb_render(GetOptSet & opts);

/********************************************************************/

/* Render MapDB map */
class GObjMapDB : public GObjMulti{
private:

  std::shared_ptr<MapDB> map;
  std::vector<std::string> groups; // ordered list of all groups
  GeoMap ref;            // default map reference
  double max_text_size;  // for selecting text objects
  double obj_scale;      // object scale
  dMultiLine border;     // border (WGS84) from set_ref
  bool clip_border;      // Clip objects to the border (default: true unless brd drawing step is used)
  bool fit_patt_size;    // Adjust pattern size to fit image size (useful for tiles). Default: false.
  double ptsize0;        // 1pt size in meters for linewidths, font size etc.
                         // Set when a "natural" reference is set with set_ref configuration command.
                         // In the beginning it is 0, this means that objects are not
                         // rescaled when map scale changes.
  double sc;             // Scale factor for objects: ptsize/ptsize0.
                         // Recalculated for each drawing process.
                         // Could be 0 (no rescaling).
  double minsc;          // Min scale (ptsize/ptsize0), below this map is just filled with minsc_color.
  uint32_t minsc_color;  // Color for drawing too small scales.

public:

  /*******************************************/
  // known drawing step
  enum StepAction {
    STEP_UNKNOWN = 0,
    STEP_DRAW_POINT = 1<<0,
    STEP_DRAW_LINE  = 1<<1,
    STEP_DRAW_AREA  = 1<<2,
    STEP_DRAW_TEXT  = 1<<3,
    STEP_DRAW_MAP   = 1<<4,
    STEP_DRAW_BRD   = 1<<5
  };

  // known drawing features for each step
  enum StepFeature {
    FEATURE_STROKE,     // draw the contour with some thickness and color
    FEATURE_FILL,       // fill the area with some color
    FEATURE_PATT,       // fill the area with a pattern
    FEATURE_CLIP,       // set clipping path
    FEATURE_IMG,        // draw an image
    FEATURE_IMG_FILTER, // set image filter
    FEATURE_SMOOTH,     // set line smoothing
    FEATURE_DASH,       // set dashed line
    FEATURE_CAP,        // set line cap
    FEATURE_JOIN,       // set line join
    FEATURE_OP,         // set drawing operator
    FEATURE_PIXAL,      // set pix_al
    FEATURE_LINES,      // lines to be drawn instead of the object
    FEATURE_CIRCLES,    // circles to be drawn instead of the object
    FEATURE_DRAW_POS,   // position for lines/circles
    FEATURE_DRAW_DIST,  // distances for lines/circles (if pos = dist or edist)
    FEATURE_SEL_RANGE,  // draw selection range instead of the object
    FEATURE_MOVETO,     // move point to a nearest object
    FEATURE_ROTATE,     // rotate object pictures/test
    FEATURE_FONT,       // set font for text objects
    FEATURE_WRITE,      // write a text object
    FEATURE_GROUP,      // set drawing step group
    FEATURE_NAME,       // set drawing step name
 };


  /*******************************************/
  // Base class for drawing features.
  // Each drawing step may contain a number of features
  // which describe various operations. Feature objects
  // are used to keep data for these operations. They
  // also fave constructors for extractind data from 
  // string arguments.

  struct Feature {
    // check number of arguments
    // args -- given arguments
    // templ -- argument template: name, ?name, ...
    void check_args(const std::vector<std::string> & args,
                    const std::vector<std::string> & templ){

      std::string list;
      for (auto const & a:templ){
        if (list.size()) list += " ";
        if (a.substr(0,1) == "?")
          list += "[" + a.substr(1,0) + "]";
        else
          list += a;
      }

      if (args.size()>templ.size()){
        if (templ.size() == 0) throw Err()
          << "Too many arguments. No arguments needed";
        if (*templ.rbegin() != "...") throw Err()
            << "Too many arguments. Expected arguments: " << list;
      }

      for (size_t i=0; i<templ.size(); i++){
        if (i<args.size()) continue;
        if (templ[i].substr(0,1) == "?") continue;
        if (templ[i] == "...") break;
        throw Err()
          << "Too few arguments. Expected arguments: " << list;
      }
    }
  };

  struct FeatureStroke : Feature {
    uint32_t col; double th;
    FeatureStroke(const std::vector<std::string> & vs){
      check_args(vs, {"<color>", "<line width>"});
      col = str_to_type<uint32_t>(vs[0]);
      th  = str_to_type<double>(vs[1]);
    }
  };

  struct FeatureFill : Feature {
    uint32_t col;
    FeatureFill(const std::vector<std::string> & vs){
      check_args(vs, {"<fill color>"});
      col = str_to_type<uint32_t>(vs[0]);
    }
  };

  struct FeatureClip : Feature {
    FeatureClip(const std::vector<std::string> & vs){
      check_args(vs, {});
    }
  };

  // for both FEATURE_PATT and FEATURE_IMG
  struct FeaturePatt : Feature {
    ImageR img; // actual data for raster images
    Cairo::RefPtr<Cairo::SurfacePattern> patt;
    double sc0,w,h;
    FeaturePatt(const std::string & imgdir,
                const std::vector<std::string> & vs){
      check_args(vs, {"<file>", "<scale>", "?<dx>", "?<dy>"});
      sc0 = str_to_type<double>(vs[1]);
      double dx = vs.size()>2 ? str_to_type<double>(vs[2]):0;
      double dy = vs.size()>3 ? str_to_type<double>(vs[3]):0;

      auto fn = vs[0];
      if (fn.size()==0) throw Err() << "empty image filename";
      if (fn[0]!='/') fn = imgdir + fn;

      if (file_ext_check(vs[0], ".svg")){
        patt = svg_to_pattern(fn, 1.0, 1.0, dx, dy, &w, &h);
      }
      else {
        img = image_load(fn);
        if (img.is_empty()) throw Err() << "empty image: " << vs[0];
        if (img.type() != IMAGE_32ARGB) img = image_to_argb(img);
        w = img.width(); h = img.height();
        patt = image_to_pattern(img, 1.0, 1.0, dx, dy);
      }
    }
    void draw_patt(const CairoWrapper & cr, const double scx, const double scy , bool fill=true){
      cr->save();
      double sx = scx*sc0, sy = scy*sc0;
      if (sx*w<1.0) sx = 1.0/w;
      if (sy*h<1.0) sy = 1.0/h;
      cr->scale(sx, sy);
      patt->set_extend(Cairo::EXTEND_REPEAT);
      cr->set_source(patt);
      if (fill) cr->fill_preserve();
      else cr->paint();
      cr->restore();
    }
    void draw_img(const CairoWrapper & cr, const dPoint & p,
                  const double angle, const double sc){
      cr->save();
      cr->translate(p.x, p.y);
      cr->rotate(angle);
      double s = sc*sc0;
      if (s*w<1.0 || s*h<1.0) s = 1.0/std::min(w,h);
      cr->scale(s, s);
      cr->set_source(patt);
      cr->paint();
      cr->restore();
    }

  };

  struct FeatureImgFilter : Feature {
    Cairo::Filter flt;
    FeatureImgFilter(const std::vector<std::string> & vs){
      check_args(vs, {"fast|good|best|nearest|bilinear"});
      if      (vs[0] == "fast") flt = Cairo::FILTER_FAST;
      else if (vs[0] == "good") flt = Cairo::FILTER_GOOD;
      else if (vs[0] == "best") flt = Cairo::FILTER_BEST;
      else if (vs[0] == "nearest")  flt = Cairo::FILTER_NEAREST;
      else if (vs[0] == "bilinear") flt = Cairo::FILTER_BILINEAR;
      else throw Err() << "wrong value: fast, good, best, nearest, or bilinear expected";
    }
  };

  struct FeatureSmooth : Feature {
    double dist;
    FeatureSmooth(const std::vector<std::string> & vs){
      check_args(vs, {"<distance>"});
      dist = str_to_type<double>(vs[0]);
    }
  };

  struct FeatureDash : Feature {
    std::vector<double> vd;
    FeatureDash(const std::vector<std::string> & vs){
      check_args(vs, {"<dist>", "..."});
      for (auto const & s:vs)
        vd.push_back(str_to_type<double>(s));
    }
  };

  struct FeatureCap : Feature {
    Cairo::LineCap cap;
    FeatureCap(const std::vector<std::string> & vs){
      check_args(vs, {"round|butt|square"});
      if      (vs[0] == "round")  cap = Cairo::LINE_CAP_ROUND;
      else if (vs[0] == "butt")   cap = Cairo::LINE_CAP_BUTT;
      else if (vs[0] == "square") cap = Cairo::LINE_CAP_SQUARE;
      else throw Err() << "wrong value: round, butt, or square expected";
    }
  };

  struct FeatureJoin : Feature {
    Cairo::LineJoin join;
    FeatureJoin(const std::vector<std::string> & vs){
      check_args(vs, {"miter|round"});
      if      (vs[0] == "miter")  join = Cairo::LINE_JOIN_MITER;
      else if (vs[0] == "round")  join = Cairo::LINE_JOIN_ROUND;
      else throw Err() << "wrong value: round or miter expected";
    }
  };

  struct FeatureOp : Feature {
    Cairo::Operator op;
    FeatureOp(const std::vector<std::string> & vs){
      check_args(vs, {"clear|source|over|in|out|atop|dest|"
          "dest_over|dest_in|dest_out|dest_atop|xor|add|saturate"});
      if      (vs[0] == "clear")     op = Cairo::OPERATOR_CLEAR;
      else if (vs[0] == "source")    op = Cairo::OPERATOR_SOURCE;
      else if (vs[0] == "over")      op = Cairo::OPERATOR_OVER;
      else if (vs[0] == "in")        op = Cairo::OPERATOR_IN;
      else if (vs[0] == "out")       op = Cairo::OPERATOR_OUT;
      else if (vs[0] == "atop")      op = Cairo::OPERATOR_ATOP;
      else if (vs[0] == "dest")      op = Cairo::OPERATOR_DEST;
      else if (vs[0] == "dest_over") op = Cairo::OPERATOR_DEST_OVER;
      else if (vs[0] == "dest_in")   op = Cairo::OPERATOR_DEST_IN;
      else if (vs[0] == "dest_out")  op = Cairo::OPERATOR_DEST_OUT;
      else if (vs[0] == "dest_atop") op = Cairo::OPERATOR_DEST_ATOP;
      else if (vs[0] == "xor")       op = Cairo::OPERATOR_XOR;
      else if (vs[0] == "add")       op = Cairo::OPERATOR_ADD;
      else if (vs[0] == "saturate")  op = Cairo::OPERATOR_SATURATE;
      else throw Err() << "Wrong drawing operator. Possible values: "
       "clear, source, over, in, out, atop, dest, "
       "dest_over, dest_in, dest_out, dest_atop, xor, add, saturate.";
    }
  };


  struct FeatureLines : Feature {
    dMultiLine lines;
    dRect bbox;
    FeatureLines(const std::vector<std::string> & vs){
      check_args(vs, {"<lines>", "..."});
      for (auto const & s:vs){
        dMultiLine ml(s);
        lines.insert(lines.end(), ml.begin(), ml.end());
      }
      bbox = lines.bbox();
    }
  };

  struct FeatureCircles : Feature {
    dLine circles; // z-coordinate = radius
    dRect bbox;
    FeatureCircles(const std::vector<std::string> & vs){
      check_args(vs, {"<circle>", "..."});
      for (auto const & s:vs){
        dPoint p(s);
        if (p.z<=0) throw Err() << "positive radius expected";
        circles.push_back(p);
        bbox.expand(p+dPoint(p.z,p.z));
        bbox.expand(p-dPoint(p.z,p.z));
      }
    }
  };

  struct FeatureDrawPos : Feature {
    enum pos_t { POINT, BEGIN, END, DIST, EDIST} pos;
    double dist, dist_b, dist_e;
    FeatureDrawPos(const std::vector<std::string> & vs):
            pos(POINT), dist(0), dist_b(0), dist_e(0){
      if (vs.size()<1) throw Err() << "argument expected";
      if      (vs[0] == "point"){ check_args(vs, {"point"}); pos = POINT; }
      else if (vs[0] == "begin"){ check_args(vs, {"begin"}); pos = BEGIN; }
      else if (vs[0] == "end")  { check_args(vs, {"end"}) ;  pos = END; }
      else if (vs[0] == "dist" || vs[0] == "edist") {
        check_args(vs, {"(dist|edist)", "dist", "?dist_b", "?dist_e"});
        pos = (vs[0] == "dist") ? DIST : EDIST;
        dist = str_to_type<double>(vs[1]);
        if (vs.size()>2) dist_b = str_to_type<double>(vs[2]);
        else dist_b = dist/2;
        if (vs.size()>3) dist_e = str_to_type<double>(vs[3]);
        else dist_e = dist/2;
      }
      else throw Err() << "Wrong position. Possible values: "
        "point, begin, end, dist, edist.";
    }
  };

  struct FeatureSelRange: Feature {
    uint32_t col; double th;
    FeatureSelRange(const std::vector<std::string> & vs){
      check_args(vs, {"<color>", "<line width>"});
      col = str_to_type<uint32_t>(vs[0]);
      th  = str_to_type<double>(vs[1]);
    }
  };

  // for both move_to and rotate_to features
  struct FeatureMoveTo : Feature {
    std::set<uint32_t> targets; // target object types
    bool rotate;
    double dist;
    FeatureMoveTo(const std::vector<std::string> & vs, const bool rotate):
        rotate(rotate){
      check_args(vs, {"<dist>", "(area|line):<type>", "..."});
      dist   = str_to_type<double>(vs[0]);
      for (size_t i=1; i<vs.size(); ++i)
        targets.insert(MapDBObj::make_type(vs[i]));
    }
  };

  struct FeaturePixAl : Feature {
    double val;
    FeaturePixAl(const std::vector<std::string> & vs){
      check_args(vs, {"<val>"});
      val = str_to_type<bool>(vs[0]);
    }
  };

  struct FeatureRotate : Feature {
    double val;
    FeatureRotate(const std::vector<std::string> & vs){
      check_args(vs, {"<val>"});
      val = str_to_type<double>(vs[0]) * M_PI/180.0;
    }
  };

  struct FeatureFont : Feature {
    double   size;
    std::string font; // target object types
    FeatureFont(const std::vector<std::string> & vs){
      check_args(vs, {"<font size>", "<font>"});
      size   = str_to_type<double>(vs[0]);
      font   = vs[1];
    }
  };

  struct FeatureWrite : Feature {
    uint32_t color;
    FeatureWrite(const std::vector<std::string> & vs){
      check_args(vs, {"<color>"});
      color  = str_to_type<uint32_t>(vs[0]);
    }
  };

  struct FeatureGroup : Feature {
    std::string name;
    FeatureGroup(const std::vector<std::string> & vs){
      check_args(vs, {"<name>"});
      name = vs[0];
    }
  };

  struct FeatureName : Feature {
    std::string name;
    FeatureName(const std::vector<std::string> & vs){
      check_args(vs, {"<name>"});
      name = vs[0];
    }
  };

  /*******************************************/
  // drawing step class
  struct DrawingStep : public GObj {
    GObjMapDB * mapdb_gobj; // back reference to the mapdb gobj
    StepAction action;  // what to do
    uint32_t etype;     // object extended type (type + (cl<<16) )
    std::string step_name;  // step name
    std::string group_name; // group name
    std::map<StepFeature, std::shared_ptr<Feature> > features;

    DrawingStep(GObjMapDB * mapdb_gobj):
       mapdb_gobj(mapdb_gobj), action(STEP_UNKNOWN), etype(0) {}
    std::string get_name() const {return step_name;}
    std::string get_group() const {return group_name;}

    // helpers used in draw() method, see .cpp files for description
    void convert_coords(MapDBObj & O);
    void draw_text(MapDBObj & O, const CairoWrapper & cr, const dRect & range, bool path, bool pix_align=false);

    // main drawing function
    ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

    // check valid step types
    void check_type(const int step_mask){
      if ((action & step_mask) == 0) throw Err()
        << "can not be used for this drawing step";
    }

  };
  /*******************************************/


  /*******************************************/
  std::vector<std::string> get_groups() const {return groups;}

  // Note that group can be only partially visible, we can only
  // set the group visibility, but not read it.
  void set_group_visibility(const std::string & name, const bool vis){
    for (auto const & st:get_data()){
      if (name != ((DrawingStep*)st.get())->get_group()) continue;
      set_visibility(st, vis);
    }
  }

  // get reference
  GeoMap get_ref() const { return ref; }

  // Set reference. If set_ptsize=true, then set ptsize0
  // using the reference.
  void set_ref(const GeoMap & r, bool set_ptsize = false);

  // get WGS border
  dMultiLine get_brd() const { return border; }

  // set WGS border
  void set_brd(const dMultiLine & brd);

  // constructor -- open new map
  GObjMapDB(const std::string & mapdir, const Opt & o);

  // load configuration file
  void load_conf(const std::string & cfgfile, Opt & defs, int & depth);

  /*******************************************/

  std::shared_ptr<ConvBase> cnv;
  Opt opt;

  // set coordinate transformation
  void set_cnv(const std::shared_ptr<ConvBase> c) override {cnv = c;};

  // set drawing options
  void set_opt(const Opt & o) override {opt = o;}

  dRect bbox() const override {
    if (border.size()) return border.bbox(); // wgs
    else return map->bbox();
  }

  // Draw all objects
  ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;


};

#endif
