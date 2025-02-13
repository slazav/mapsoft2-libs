#ifndef CAIRO_WRAPPER_H
#define CAIRO_WRAPPER_H

#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <cairomm/script.h>

#include <string>

#include "image/image_r.h"
#include "geom/point.h"
#include "geom/line.h"
#include "geom/multiline.h"
#include "geom/rect.h"

// Convert image to a Cairo::ImageSurface.
// Data is kept in the Image, it should be alive while
// the ImageSurface is used.
Cairo::RefPtr<Cairo::ImageSurface> image_to_surface(const ImageR & img);

// Convert image to a Cairo::SurfacePattern.
// Data is kept in the Image, it should be alive while
// the ImageSurface is used.
Cairo::RefPtr<Cairo::SurfacePattern> image_to_pattern(
  const ImageR & img, double scx, double scy, double dx=0, double dy=0);

// Load svg file to a pattern
// Return scaled image dimensions in wret,href (if non-null)
Cairo::RefPtr<Cairo::SurfacePattern> svg_to_pattern(
  const std::string & fname, double scx, double scy, double dx, double dy,
  double *wret = NULL, double *hret = NULL);

//////////////////////////////////////////////////////////////////
// converting strings to cairo types

template<>
Cairo::Operator str_to_type<Cairo::Operator>(const std::string & s);

template<>
Cairo::LineJoin str_to_type<Cairo::LineJoin>(const std::string & s);

template<>
Cairo::LineCap str_to_type<Cairo::LineCap>(const std::string & s);

template<>
Cairo::Filter str_to_type<Cairo::Filter>(const std::string & s);


//////////////////////////////////////////////////////////////////
/// This class contains functions
/// we want to add to the Cairo::Context
struct CairoExtra : public Cairo::Context {
  void save_png(const char *fname){
    Cairo::Context::get_target()->write_to_png(fname); }

  void set_color_a(const int c){
    Cairo::Context::set_source_rgba(
      ((c&0xFF0000)>>16)/255.0,
      ((c&0xFF00)>>8)/255.0,
       (c&0xFF)/255.0,
      ((c&0xFF000000)>>24)/255.0
    );
  }
  void set_color(const int c){
    Cairo::Context::set_source_rgb(
      ((c&0xFF0000)>>16)/255.0,
      ((c&0xFF00)>>8)/255.0,
       (c&0xFF)/255.0
    );
  }

  // move_to/line_to functions for dPoint arguments

  using Cairo::Context::translate;
  void translate(const dPoint & p){ translate(p.x, p.y); }

  using Cairo::Context::move_to;
  void move_to(const dPoint & p){ move_to(p.x, p.y); }

  using Cairo::Context::line_to;
  void line_to(const dPoint & p){ line_to(p.x, p.y); }

  using Cairo::Context::rel_move_to;
  void rel_move_to(const dPoint & p){ rel_move_to(p.x, p.y); }

  using Cairo::Context::rel_line_to;
  void rel_line_to(const dPoint & p){ rel_line_to(p.x, p.y); }

  using Cairo::Context::curve_to;
  void curve_to(const dPoint & p1, const dPoint & p2, const dPoint & p3){
    curve_to(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y); }

  using Cairo::Context::rel_curve_to;
  void rel_curve_to(const dPoint & p1, const dPoint & p2, const dPoint & p3){
    rel_curve_to(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y); }

  using Cairo::Context::rectangle;
  void rectangle(const dRect &r){ rectangle(r.x, r.y, r.w, r.h); }

  void circle(const dPoint &p, const double r){
    Cairo::Context::begin_new_sub_path();
    Cairo::Context::arc(p.x, p.y, r, 0, 2*M_PI);
  }

  using Cairo::Context::get_current_point;
  dPoint get_current_point(){
    dPoint ret;
    get_current_point(ret.x, ret.y);
    return ret;
  }

  using Cairo::Context::get_text_extents;
  dRect get_text_extents(const std::string & utf8){
    // dPoint p;
    // get_current_point(p.x, p.y);
    Cairo::TextExtents extents;
    get_text_extents (utf8, extents);
    return dRect(extents.x_bearing, extents.y_bearing,
                 extents.width, extents.height);
  }

  // Make paths from dLine/dMultiline argument

  void mkpath(const dLine & line, bool close=true){
    if (line.size()==0) return;
    move_to(*line.begin());
    for (auto const & p:line) line_to(p);
    if (close) close_path();
  }
  void mkpath(const dMultiLine & mline, bool close=true){
    for (auto const & l:mline) mkpath(l, close);
  }

  void mkpath_points(const dLine & line){
    for (auto const & p:line){
      move_to(p);
      Cairo::Context::rel_line_to(0,0);
    }
  }
  void mkpath_points(const dMultiLine & mline){
    for (auto const & l:mline) mkpath_points(l);
  }

  // make path for smoothed line
  void mkpath_smline(const dMultiLine & o, bool close=true, double curve_l=0);
  void mkpath_smline(const dLine & o, bool close=true, double curve_l=0);

  // shifted
  void mkpath_shifted(const dMultiLine & o, bool close=true, double shift = 0);
  void mkpath_shifted(const dLine & o, bool close=true, double shift = 0);

  // some short functions for cap/miter setting
  void cap_round()  { set_line_cap(Cairo::LINE_CAP_ROUND);  }
  void cap_butt()   { set_line_cap(Cairo::LINE_CAP_BUTT);   }
  void cap_square() { set_line_cap(Cairo::LINE_CAP_SQUARE); }
  void join_miter() { set_line_join(Cairo::LINE_JOIN_MITER); }
  void join_round() { set_line_join(Cairo::LINE_JOIN_ROUND); }

  // short functions for dash line settings
  using Cairo::Context::set_dash;
  void set_dash(std::vector<double> d){ set_dash(d, 0); }

  void set_dash(double d1, double d2){
    std::vector<double> d;
    d.push_back(d1);
    d.push_back(d2);
    set_dash(d, 0);
  }
  void set_dash(double d1, double d2, double d3, double d4){
    std::vector<double> d;
    d.push_back(d1);
    d.push_back(d2);
    d.push_back(d3);
    d.push_back(d4);
    set_dash(d, 0);
  }

  // set FIG font (not full support, not recommended)
  void set_fig_font(int color, int fig_font, double font_size, double dpi);

  // set FC font
  // For font properties see:
  // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html
  // https://www.freedesktop.org/software/fontconfig/fontconfig-user.html
  // https://wiki.archlinux.org/index.php/Font_configuration
  // Examples of fc_patt:
  //   "Century Schoolbook L:Italic:semicondensed:rgba=none"
  void set_fc_font(int color, const char *fc_patt, double font_size);

  // render text
  void text(const char *text, dPoint pos, double ang, int hdir=0, int vdir=0);


  void render_border(const iRect & range, const dLine & brd, const int bgcolor);

};

//////////////////////////////////////////////////////////////////
/*** CairoWrapper - we need this to create RefPtr<CairoExtra> ***/

struct CairoWrapper: Cairo::RefPtr<CairoExtra> {

private:
  ImageR image; // keeps actual data for image surfaces.
  Cairo::RefPtr<Cairo::Surface> surface;
  int w,h; // surface size in pixels

public:

  CairoWrapper(){}

  CairoWrapper(const Cairo::RefPtr<Cairo::Context> & cr):
    Cairo::RefPtr<CairoExtra>(
      Cairo::RefPtr<CairoExtra>::cast_static(cr)) {}

  // Create surface and new cairo context
  // using internal image.
  // This should be done before any drawing.
  void set_surface_img(int w, int h);

  // Create surface and new cairo context
  // using external image.
  // This should be done before any drawing.
  void set_surface_img(const ImageR & img);

  // Create surface and new cairo context
  // using Postscript file.
  // This should be done before any drawing.
  void set_surface_ps(const char *fname, int w, int h);

  // Create surface and new cairo context
  // using PDF file.
  // This should be done before any drawing.
  void set_surface_pdf(const char *fname, int w, int h);

  // Create surface and new cairo context
  // using SVG file.
  // This should be done before any drawing.
  void set_surface_svg(const char *fname, int w, int h);

  // get the surface
  Cairo::RefPtr<Cairo::Surface> get_surface() const { return surface; }

  // get the image (empty image for non-image surfaces!)
  ImageR get_image() const { return image; }

  // get surface width
  int width() const { return w; }

  // get surface height
  int height() const { return h; }

  // get surface bbox (starting at 0,0)
  iRect bbox() const { return dRect(0,0,w,h); }

};
#endif
