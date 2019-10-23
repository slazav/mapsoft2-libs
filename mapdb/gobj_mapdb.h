#ifndef GOBJ_MAPDB_H
#define GOBJ_MAPDB_H

#include <list>
#include <vector>
#include <string>
#include <map>

#include "cairo/cairo_wrapper.h"
#include "image/image_colors.h"
#include "viewer/gobj_multi.h"

#include "mapdb.h"
#include "image/image.h"
#include "geo_data/geo_data.h"
#include "opt/opt.h"

/*

Drawing of a vector map is a sequence of "drawing steps". Each step
contains drawing of a single object type, of a grid, or setting
some global parameter (like map reference).
The configuration file contains description of each step.

Drawing of a point/line/area object can be described
in following way:

    (point|line|area) <type> <feature> <options> ...
    + <feature> <options> ...
    + <feature> <options> ...
    ...
    map <feature> <options> ...
    + <feature> <options> ...
    ...

where possible features are:

    stroke <width> <color> -- Draw a point, a line or an area contour.
    fill <color> -- Fill an area or the map.
    patt <image file> <scale> -- Fill an area with the pattern.
    img  <image file> <scale> -- Draw an image (point or area)
    smooth <distance> -- Use smoothed paths.
    dash <len1> ...  -- Setup dashed line. Valid for lines and areas together with stroke feature.
    cap  round|butt|square -- Set line cup (default round). Use with stroke feature.
    join round|miter       -- Set line join (default round). Use with stroke feature.
    operator <op>          -- Set drawing operator (clear|source|over|in|out|atop|dest|
                              dest_over|dest_in|dest_out|dest_atop|xor|add|saturate)
    group <name>           -- set group name for a drawing step
    name  <name>           -- set name for a drawing step

If `stroke`, `fill` and `patt` `img` features exists together then the drawing
order is following: pattern, then fill, then stroke, then img.

*/

/********************************************************************/
#include "getopt/getopt.h"

// add MS2OPT_DRAWMAPDB options
void ms2opt_add_drawmapdb(GetOptSet & opts);

/********************************************************************/

/* Render MapDB map */
class GObjMapDB : public GObjMulti{
private:

  std::string mapdir;
  std::shared_ptr<MapDB> map;
  std::vector<std::string> groups; // ordered list of all groups

public:

  /*******************************************/
  // known drawing step
  enum StepAction {
    STEP_UNKNOWN,
    STEP_DRAW_POINT,
    STEP_DRAW_LINE,
    STEP_DRAW_AREA,
    STEP_DRAW_MAP
  };

  // known drawing features for each step
  enum StepFeature {
    FEATURE_STROKE, // draw the contour with some thickness and color
    FEATURE_FILL,    // fill the area with some color
    FEATURE_PATT,    // fill the area with a pattern
    FEATURE_IMG,     // draw an image
    FEATURE_SMOOTH,  // set line smoothing
    FEATURE_DASH,    // set dashed line
    FEATURE_CAP,     // set line cap
    FEATURE_JOIN,    // set line cap
    FEATURE_OP,      // set drawing operator
    FEATURE_GROUP,   // set drawing step group
    FEATURE_NAME,    // set drawing step name
 };


  /*******************************************/
  // base class for drawing features
  struct Feature {
    // check number of arguments
    void check_args(const std::vector<std::string> & vs,
                    const std::vector<std::string> & args){
      if (vs.size()!=args.size()){
        std::string list;
        for (auto const & a:args){
          if (list.size()) list += ", ";
          list += "<" + a + ">";
        }
        throw Err()
          << args.size() << " argument" << (args.size()>1 ? "s":"")
          << " expected: " << list;
      }
    }
  };

  struct FeatureStroke : Feature {
    uint32_t col; double th;
    FeatureStroke(const std::vector<std::string> & vs){
      check_args(vs, {"color", "line width"});
      col = str_to_type<uint32_t>(vs[0]);
      th  = str_to_type<double>(vs[1]);
    }
  };

  struct FeatureFill : Feature {
    uint32_t col;
    FeatureFill(const std::vector<std::string> & vs){
      check_args(vs, {"fill color"});
      col = str_to_type<uint32_t>(vs[0]);
    }
  };

  // for both FEATURE_PATT and FEATURE_IMG
  struct FeaturePatt : Feature {
    Image img; // actual data
    Cairo::RefPtr<Cairo::SurfacePattern> patt;
    FeaturePatt(const std::string & mapdir,
                const std::vector<std::string> & vs){
      check_args(vs, {"file", "scale"});
      double k = str_to_type<double>(vs[1]);
      img = image_load(mapdir + "/" + vs[0]);
      if (img.is_empty()) throw Err() << "empty image: " << vs[0];
      if (img.type() != IMAGE_32ARGB)
        img = image_to_argb(img);
      patt = image_to_pattern(img, k);
    }
  };

  struct FeatureSmooth : Feature {
    double dist;
    FeatureSmooth(const std::vector<std::string> & vs){
      check_args(vs, {"distance"});
      dist = str_to_type<double>(vs[0]);
    }
  };

  struct FeatureDash : Feature {
    std::vector<double> vd;
    FeatureDash(const std::vector<std::string> & vs){
      if (vs.size()<1)
        throw Err()
          << "at least one argument expected: <dist1> <dist2> ...";
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
      check_args(vs, {"<miter|round>"});
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

  struct FeatureGroup : Feature {
    std::string name;
    FeatureGroup(const std::vector<std::string> & vs){
      check_args(vs, {"name"});
      name = vs[0];
    }
  };

  struct FeatureName : Feature {
    std::string name;
    FeatureName(const std::vector<std::string> & vs){
      check_args(vs, {"name"});
      name = vs[0];
    }
  };

  /*******************************************/
  // drawing step class
  struct DrawingStep : public GObj {
    StepAction action;  // what to do
    uint32_t etype;     // object extended type (type + (cl<<16) )
    std::string step_name;  // step name
    std::string group_name; // group name
    std::map<StepFeature, std::shared_ptr<Feature> > features;
    MapDB * map;

    DrawingStep(MapDB * map): map(map), action(STEP_UNKNOWN), etype(0) {}
    std::string get_name() const {return step_name;}
    std::string get_group() const {return group_name;}
    int draw(const CairoWrapper & cr, const dRect & draw_range) override;
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

  // constructor -- open new map
  GObjMapDB(const std::string & mapdir);

};

#endif
