#include "cairo_wrapper.h"
#include "geom/line.h"
#include "err/err.h"

#include <librsvg/rsvg.h>


Cairo::RefPtr<Cairo::ImageSurface>
image_to_surface(const Image & img) {
  // convert image to cairo surface
  Cairo::Format format = Cairo::FORMAT_ARGB32;
  // check if surface raw data compatable with Image
  if (img.type() != IMAGE_32ARGB)
    throw Err() << "Cairo::image_to_surface: only 32-bpp images are supported";
  if (Cairo::ImageSurface::format_stride_for_width(format, img.width()) != img.width()*4)
    throw Err() << "Cairo::image_to_surface: non-compatable data";
  return Cairo::ImageSurface::create(img.data(),
      format, img.width(), img.height(), img.width()*4);
}

Cairo::RefPtr<Cairo::SurfacePattern>
image_to_pattern(const Image & img, double scx, double scy, double dx, double dy){
  try{
    auto surf = image_to_surface(img);
    Cairo::RefPtr<Cairo::SurfacePattern> patt =
      Cairo::SurfacePattern::create(surf);
    Cairo::Matrix M=Cairo::identity_matrix();
    M.translate(surf->get_width()*(0.5+dx), surf->get_height()*(0.5+dy));
    M.scale(1/scx,1/scy);
    patt->set_matrix(M);
    return patt;
  }
  catch (Cairo::logic_error err){
    throw Err() << err.what();
  }
}

Cairo::RefPtr<Cairo::SurfacePattern>
svg_to_pattern(const std::string & fname, double scx, double scy, double dx, double dy){
  try{
    GError *err;
    auto svg = rsvg_handle_new_from_file(fname.c_str(), &err);
    if (!svg) throw Err() << fname << ": " << err->message;

    RsvgDimensionData dim;
    rsvg_handle_get_dimensions(svg, &dim);
    int w = dim.width;
    int h = dim.height;

    auto surf = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, w, h);
    auto cr = Cairo::Context::create(surf);
    if (!rsvg_handle_render_cairo(svg,cr->cobj()))
      throw Err() << fname << ": can't render SVG image";
    g_object_unref(svg);

    auto patt = Cairo::SurfacePattern::create(surf);
    Cairo::Matrix M=Cairo::identity_matrix();
    M.translate(w*(0.5+dx), h*(0.5+dy));
    M.scale(1/scx,1/scy);
    patt->set_matrix(M);
    return patt;
  }
  catch (Cairo::logic_error err){
    throw Err() << err.what();
  }
}


void
CairoExtra::mkpath_smline(const dMultiLine & o, bool close, double curve_l){
  for (dMultiLine::const_iterator l=o.begin(); l!=o.end(); l++)
    mkpath_smline(*l, close, curve_l);
}

void
CairoExtra::mkpath_smline(const dLine & o, bool close, double curve_l){
  if (o.size()<1) return;
  if (o.size()==1){
    move_to(*o.begin());
    line_to(*o.begin());
    return;
  }

  // simple lines, no smoothing
  if (curve_l==0){
    for (int i=0; i<o.size(); i++) {
      dPoint p = o[i];
      if (i==0) move_to(p);
      else line_to(p);
    }
    if (close) close_path();
    return;
  }

  // lines with smoothing
  for (int i=0; i<o.size(); i++) {
    dPoint p = o[i];
    dPoint pp = o[i>0? i-1: o.size()-1];
    dPoint pn = o[i<o.size()-1? i+1: 0];
    if (!close && i==0) pp=p;
    if (!close && i==o.size()-1) pn=p;

    dPoint pp2, pn1, pn2;
    if (len2d(p-pp) > 2*curve_l){
      pp2 = p - norm2d(p - pp)*curve_l;
    }
    else {
      pp2=(p+pp)/2.0;
    }
    if (len2d(p-pn) > 2*curve_l){
      pn1 = p + norm2d(pn - p)*curve_l;
      pn2 = pn - norm2d(pn - p)*curve_l;
    }
    else {
      pn1=pn2=(p+pn)/2.0;
    }

    if (i==0) move_to(pp2);

    curve_to(p, p, pn1);
    line_to(pn2);
  }
}


void
CairoExtra::set_fig_font(int color, int fig_font, double font_size, double dpi){
  set_color(color);

  std::string       face;
  Cairo::FontSlant  slant;
  Cairo::FontWeight weight;
  switch(fig_font){
    case 0:
      face="times";
      slant=Cairo::FONT_SLANT_NORMAL;
      weight=Cairo::FONT_WEIGHT_NORMAL;
      break;
    case 1:
      face="times";
      slant=Cairo::FONT_SLANT_ITALIC;
      weight=Cairo::FONT_WEIGHT_NORMAL;
      break;
    case 2:
      face="times";
      slant=Cairo::FONT_SLANT_NORMAL;
      weight=Cairo::FONT_WEIGHT_BOLD;
      break;
    case 3:
      face="times";
      slant=Cairo::FONT_SLANT_ITALIC;
      weight=Cairo::FONT_WEIGHT_BOLD;
      break;
    case 16:
      face="sans";
      slant=Cairo::FONT_SLANT_NORMAL;
      weight=Cairo::FONT_WEIGHT_NORMAL;
      break;
    case 17:
      face="sans";
      slant=Cairo::FONT_SLANT_OBLIQUE;
      weight=Cairo::FONT_WEIGHT_NORMAL;
      break;
    case 18:
      face="sans";
      slant=Cairo::FONT_SLANT_NORMAL;
      weight=Cairo::FONT_WEIGHT_BOLD;
      break;
    case 19:
      face="sans";
      slant=Cairo::FONT_SLANT_OBLIQUE;
      weight=Cairo::FONT_WEIGHT_BOLD;
      break;
    default:
      std::cerr << "warning: unsupported fig font: " << fig_font << "\n";
      face="sans";
      slant=Cairo::FONT_SLANT_NORMAL;
      weight=Cairo::FONT_WEIGHT_NORMAL;
  }

  if (face=="times") font_size/=0.85;
  Cairo::Context::set_font_size(font_size*dpi/89.0);
  Cairo::Context::set_font_face(
    Cairo::ToyFontFace::create(face, slant, weight));
}


void
CairoExtra::set_fc_font(int color, const char *fc_patt, double font_size){
  set_color(color);
  Cairo::Context::set_font_size(font_size);
  // For work with patterns see:
  // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x103.html#AEN242
  // For font properties see:
  // https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html
  // https://www.freedesktop.org/software/fontconfig/fontconfig-user.html
  // https://wiki.archlinux.org/index.php/Font_configuration
  FcPattern *patt = FcNameParse((const FcChar8 *)fc_patt);
  set_font_face( Cairo::FtFontFace::create(patt));
  FcPatternDestroy(patt);
}

void
CairoExtra::text(const char *text, dPoint pos, double ang, int hdir, int vdir){
  Cairo::Context::save();
  move_to(pos);
  Cairo::Context::rotate(ang);
  if (hdir!=0 || vdir!=0) {
    dRect ext = get_text_extents(text);
    switch (hdir){
      case 1: Cairo::Context::rel_move_to(-ext.w/2, 0.0); break;
      case 2: Cairo::Context::rel_move_to(-ext.w, 0.0); break;
      case 0: break;
      default: throw Err() << "cairo_wrapper: wrong hdir parameter for text";
    }
    switch (vdir){
      case 1: Cairo::Context::rel_move_to(0.0, ext.h/2); break;
      case 2: Cairo::Context::rel_move_to(0.0, ext.h); break;
      case 0: break;
      default: throw Err() << "cairo_wrapper: wrong hdir parameter for text";
    }
  }
//  Cairo::Context::reset_clip();
  Cairo::Context::show_text(text);
  Cairo::Context::restore();
}


void
CairoExtra::render_border(const iRect & range, const dLine & brd, const int bgcolor){
  // make border path
  mkpath(brd);
  if (bgcolor!=0){
    // draw border
    set_source_rgb(0,0,0);
    set_line_width(2);
    stroke_preserve();
  }

  // erase everything outside border
  mkpath(rect_to_line(expand(range,1)));
  set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
  if (bgcolor==0)  set_operator(Cairo::OPERATOR_CLEAR);
  else  set_color(bgcolor);
  fill();
}

/// Cairo Wrapper functions
void
CairoWrapper::set_surface_img(int w_, int h_){
  w = w_; h=h_;
  image=Image(w, h, IMAGE_32ARGB);

  surface = image_to_surface(image);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_img(const Image & img){
  w = img.width(); h=img.height();
  image=img; // increase refcount of image

  surface = image_to_surface(image);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_ps(const char *fname, int w_, int h_){
  w = w_; h=h_; image = Image();
  surface = Cairo::PsSurface::create(fname, w, h);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_pdf(const char *fname, int w_, int h_){
  w = w_; h=h_; image = Image();
  surface = Cairo::PdfSurface::create(fname, w, h);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_svg(const char *fname, int w_, int h_){
  w = w_; h=h_; image = Image();
  surface = Cairo::SvgSurface::create(fname, w, h);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}


