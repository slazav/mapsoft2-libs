#ifndef VMAP2_GOBJ_H
#define VMAP2_GOBJ_H

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
#include "read_words/read_words.h"

#include "vmap2.h"

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
    pix_align (0|1)        -- align text positions to integer pixels (default: 0)
    text_vspace <value>    -- adjust multi-line objects vertical spacing. 1.0 is
                              a normal font spacing, default is 0.7 (dense lines).
    lines <lines> ...      -- draw lines
    circles <circle> ...   -- draw circles
    draw_pos (point|begin|end) -- reference positions for lines/circles features:
                              at each point, at the beginning, at the end
    draw_pos (dist|edist) <dist> [<dist_begin>] [<dist_end>] -- reference positions for lines/circles features:
                              at dist period, starting at dist_begin (defaults: dist/2) from the first point
                              and ending not further then dist_end from the last point.
                              For `edist` distanse will be adjusted to have exact begin and end distances.
    draw_pos fill <w> <h>     use lines/circles features as a pattern for filling area
    sel_range <color> <width>  -- draw selection range for each object
    move_to <max distance> <type> ...  -- move point to the nearest object
    rotate_to <max distance> <type> ... -- move and rotate point to the nearest object
    move_from <min distance> <type> ... -- move object points away from other type of object, keeping minimum distance
    rotate <angle>         -- rotate object pictures (img, lines, circles), degrees, cw
    short_expand <len>     -- expand short lines to the length by extending end segments (TODO: polygons?)
    short_skip <len>       -- skip short lines (TODO: polygons?)
    font <size> <font>     -- set font (fontconfig pattern)
    write <color>          -- render the text (a bit different from `fill`)
    group <name>           -- set group name for a drawing step
    name  <name>           -- set name for a drawing step
    pulk_grid <step> <color> <line width> -- draw Pulkovo-1942 grid
    fi_grid <step> <color> <line width> -- draw ETRS-TM35FIN grid (Finland)
    grid_labels <size> <font> <color> -- draw grid labels

If `stroke`, `fill` and `patt` `img` features exists together then the drawing
order is following: pattern, then fill, then stroke, then img.

other commands in the configuration file

    set_ref file <filename> -- set map reference including border from a file
    set_ref nom <name> <dpi> -- set map reference and border as a Soviet nomenclature name
    set_brd file <filename> -- set map border from track file
    max_text_size <value> -- set max_text_size value for selecting text objects in the database
    define <name> <definition> -- define a variable which can be used as ${name} later
    define_if_undef <name> <definition> -- define a variable if it is not defined
    if <word1> (==|!=) <word2>, ifdef <word>, ifndef <word>, else, endif --  if statement. If condition is true/false
       text between if and endif is is processed/ignored. One can use nested if-endif commands.
       Command else just inverts condition of the last if command.
    include <file> -- Read another configuration file.
       Image paths are calculated with respect to the current file.

*/

/********************************************************************/
#include "getopt/getopt.h"

// add VMAP2_RENDER options
void ms2opt_add_vmap2_render(GetOptSet & opts);

/********************************************************************/

/* Render VMap2 map */
class GObjVMap2 : public GObjMulti{
private:

  VMap2 & map;
  std::vector<std::string> groups; // ordered list of all groups
  GeoMap ref;            // default map reference
  double max_text_size;  // for selecting text objects
  double obj_scale;      // object scale
  dMultiLine border;     // border (WGS84) from set_ref
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
  size_t nsaved;         // how many times cairo context has been saved (fr clipping)

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

  // load image from file and render it with cairo
  // for both FEATURE_PATT and FEATURE_IMG
  struct ImageRenderer {
    ImageR img; // actual data for raster images
    Cairo::RefPtr<Cairo::SurfacePattern> patt;
    double sc0,w,h;
    bool empty;
    ImageRenderer(){ sc0 = w = h = 0; empty = true;}
    ImageRenderer(const std::string & fname, double sc, double dx, double dy){
      sc0 = sc;
      if (file_ext_check(fname, ".svg")){
        patt = svg_to_pattern(fname, 1.0, 1.0, dx, dy, &w, &h);
      }
      else {
        img = image_load(fname);
        if (img.is_empty()) throw Err() << "empty image: " << fname;
        if (img.type() != IMAGE_32ARGB) img = image_to_argb(img);
        w = img.width(); h = img.height();
        patt = image_to_pattern(img, 1.0, 1.0, dx, dy);
      }
      empty = false;
    }
    ImageRenderer(const ImageR & image){
      img = image; sc0=1;
      if (img.type() != IMAGE_32ARGB) img = image_to_argb(img);
      w = img.width(); h = img.height();
      patt = image_to_pattern(img, 1.0, 1.0, 0.0, 0.0);
    }
    void draw_patt(const CairoWrapper & cr,
         const double scx, const double scy,
         bool fill=true){
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
    void set_filter(const Cairo::Filter f){
      if (!empty) patt->set_filter(f);
    }

    bool is_empty() const {return empty;}
    double width() const {return w;}
    double height() const {return h;}
  };

  /*******************************************/
  // drawing step class
  struct DrawingStep : public GObj {
    GObjVMap2 * gobj; // back reference to the mapdb gobj
    StepAction action;  // what to do
    uint32_t etype;     // object extended type (type + (cl<<16) )

    std::string step_name;  // step name
    std::string group_name; // group name
    bool do_clip, do_stroke, do_fill, do_write,
         do_patt, do_img, do_pulk_grid, do_fi_grid, do_sel_range;
    uint32_t stroke_color;
    uint32_t fill_color;
    uint32_t write_color;
    double thickness;
    Cairo::Filter img_filter;
    double smooth;
    std::vector<double> dash;
    Cairo::LineCap line_cap;
    Cairo::LineJoin line_join;
    Cairo::Operator op;
    double short_expand;
    double short_skip;
    bool pix_align;
    enum pos_t { DRAW_POS_POINT, DRAW_POS_BEGIN,
                 DRAW_POS_END, DRAW_POS_DIST, DRAW_POS_EDIST, DRAW_POS_FILL} draw_pos;
    double draw_pos_dist, draw_pos_b, draw_pos_e;
    size_t draw_pos_w, draw_pos_h;
    Opt grid_opts;
    std::string font;
    double font_size;
    double text_vspace;
    double rotate;
    bool outer;
    std::set<uint32_t> move_to_targets, move_from_targets;
    bool move_to_rot;
    double move_to_dist, move_from_dist;
    double sel_range_thickness;
    uint32_t sel_range_color;
    dMultiLine add_lines;
    dLine add_circles; // (x,y,r) points
    dRect add_bbox;
    ImageRenderer patt, img;

    DrawingStep(GObjVMap2 * gobj): gobj(gobj), action(STEP_UNKNOWN), etype(0) {
      do_clip = do_stroke = do_fill = do_write = false;
      do_patt = do_img = do_pulk_grid = do_fi_grid = do_sel_range = false;
      stroke_color = 0;
      fill_color = 0;
      write_color = 0;
      thickness = 1.0;
      img_filter = Cairo::FILTER_GOOD;
      smooth = 0;
      line_cap = Cairo::LINE_CAP_ROUND;
      line_join = Cairo::LINE_JOIN_ROUND;
      op = Cairo::OPERATOR_OVER;
      short_expand = 0;
      short_skip = 0;
      pix_align = false;
      draw_pos = DRAW_POS_POINT;
      draw_pos_dist = draw_pos_b = draw_pos_e = 0;
      font_size = 10;
      text_vspace = 0.7;
      rotate = 0;
      outer = false;
      move_to_rot = false;
      move_to_dist = 0;
      move_from_dist = 0;
      sel_range_thickness = 0;
      sel_range_color = 0;
    }
    std::string get_name() const {return step_name;}
    std::string get_group() const {return group_name;}

    // helpers used in draw() method, see .cpp files for description
    void convert_coords(VMap2obj & O);
    void draw_text(VMap2obj & O, const CairoWrapper & cr, const dRect & range, bool path);
    void setup_ctx(const CairoWrapper & cr, const double osc);

    // main drawing function
    ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

    // check valid step types
    void check_type(const int step_mask){
      if ((action & step_mask) == 0) throw Err()
        << "can not be used for this drawing step";
    }

    // check number of arguments
    // args -- given arguments
    // templ -- argument template: name, ?name, ...
    static void check_args(const std::vector<std::string> & args,
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

  // update range (used in set_brd, set_ref, set_cnv)
  void update_bbox();

  // constructor -- open new map
  GObjVMap2(VMap2 & map, const Opt & o);

  // load configuration file
  void load_conf(const std::string & cfgfile, read_words_defs & defs, int & depth);

  /*******************************************/

  std::shared_ptr<ConvBase> cnv;
  Opt opt;
  dRect range; // bbox in viewer coords (based on non-empty border or geohash)

  // set coordinate transformation
  void set_cnv(const std::shared_ptr<ConvBase> c) override;

  // set drawing options
  void set_opt(const Opt & o) override {opt = o;}

  // bbox in viewer coords
  dRect bbox() const override { return range; }

  // Check drawing range
  ret_t check(const dRect & draw_range) const override;

  // Draw all objects
  ret_t draw(const CairoWrapper & cr, const dRect & draw_range) override;

};

#endif
