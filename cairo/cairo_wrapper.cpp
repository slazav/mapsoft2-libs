#include "cairo_wrapper.h"
#include "geom/line.h"
#include "err/err.h"

#include <librsvg/rsvg.h>


Cairo::RefPtr<Cairo::ImageSurface>
image_to_surface(const ImageR & img) {
  // convert image to cairo surface
  Cairo::Format format = Cairo::FORMAT_ARGB32;
  // check if surface raw data compatable with Image
  if (img.type() != IMAGE_32ARGB)
    throw Err() << "Cairo::image_to_surface: only 32-bpp images are supported";
  if ((size_t)Cairo::ImageSurface::format_stride_for_width(format, img.width()) != img.width()*4)
    throw Err() << "Cairo::image_to_surface: non-compatable data";
  return Cairo::ImageSurface::create(img.data(),
      format, img.width(), img.height(), img.width()*4);
}

Cairo::RefPtr<Cairo::SurfacePattern>
image_to_pattern(const ImageR & img, double scx, double scy, double dx, double dy){
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
  catch (Cairo::logic_error & err){
    throw Err() << err.what();
  }
}

Cairo::RefPtr<Cairo::SurfacePattern>
svg_to_pattern(const std::string & fname, double scx, double scy, double dx, double dy, double *wret, double *hret){
  try{
    GError *err = NULL;
    auto svg = rsvg_handle_new_from_file(fname.c_str(), &err);
    if (!svg && err) throw Err() << fname << ": " << err->message;
    if (!svg) throw Err() << fname << ": can't load SVG file";

    gdouble dw, dh;
    if (!rsvg_handle_get_intrinsic_size_in_pixels(svg, &dw, &dh))
      throw Err() << fname << ": can't convert dimensions to pixels";

    int w = ceil(dw);
    int h = ceil(dh);
    if (wret) *wret=w*scx;
    if (hret) *hret=w*scy;

    auto surf = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, w, h);
    auto cr = Cairo::Context::create(surf);
    RsvgRectangle rect = {0.0,0.0,dw,dh};
    if (!rsvg_handle_render_document(svg,cr->cobj(), &rect, &err))
      throw Err() << fname << ": can't render SVG image: " << err->message;
    g_object_unref(svg);

    auto patt = Cairo::SurfacePattern::create(surf);
    Cairo::Matrix M=Cairo::identity_matrix();
    M.translate(w*(0.5+dx), h*(0.5+dy));
    M.scale(1/scx,1/scy);
    patt->set_matrix(M);
    return patt;
  }
  catch (Cairo::logic_error & err){
    throw Err() << err.what();
  }
}

template<>
Cairo::Operator
str_to_type<Cairo::Operator>(const std::string & s){
  if (s == "clear")     return Cairo::OPERATOR_CLEAR;
  if (s == "source")    return Cairo::OPERATOR_SOURCE;
  if (s == "over")      return Cairo::OPERATOR_OVER;
  if (s == "in")        return Cairo::OPERATOR_IN;
  if (s == "out")       return Cairo::OPERATOR_OUT;
  if (s == "atop")      return Cairo::OPERATOR_ATOP;
  if (s == "dest")      return Cairo::OPERATOR_DEST;
  if (s == "dest_over") return Cairo::OPERATOR_DEST_OVER;
  if (s == "dest_in")   return Cairo::OPERATOR_DEST_IN;
  if (s == "dest_out")  return Cairo::OPERATOR_DEST_OUT;
  if (s == "dest_atop") return Cairo::OPERATOR_DEST_ATOP;
  if (s == "xor")       return Cairo::OPERATOR_XOR;
  if (s == "add")       return Cairo::OPERATOR_ADD;
  if (s == "saturate")  return Cairo::OPERATOR_SATURATE;
  throw Err() << "Wrong operator value: " << s << ": "
              << "expected one of clear, source, over, in, out, atop, dest, "
              << "dest_over, dest_in, dest_out, dest_atop, xor, add, saturate.";
}

template<>
Cairo::LineJoin
str_to_type<Cairo::LineJoin>(const std::string & s){
  if (s == "miter") return Cairo::LINE_JOIN_MITER;
  if (s == "round") return Cairo::LINE_JOIN_ROUND;
  throw Err() << "Wrong line_join value: " << s << ": "
              << "expected round or miter";
}

template<>
Cairo::LineCap str_to_type<Cairo::LineCap>(const std::string & s){
  if (s == "round")  return Cairo::LINE_CAP_ROUND;
  if (s == "butt")   return Cairo::LINE_CAP_BUTT;
  if (s == "square") return Cairo::LINE_CAP_SQUARE;
  throw Err() << "wrong line_cap value: " << s << ": "
              << "expected round, butt, or square";
}

template<>
Cairo::Filter str_to_type<Cairo::Filter>(const std::string & s){
  if (s == "fast") return Cairo::FILTER_FAST;
  if (s == "good") return Cairo::FILTER_GOOD;
  if (s == "best") return Cairo::FILTER_BEST;
  if (s == "nearest")  return Cairo::FILTER_NEAREST;
  if (s == "bilinear") return Cairo::FILTER_BILINEAR;
  throw Err() << "wrong filter value: " << s << ": "
              << "expected fast, good, best, nearest, or bilinear";
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
    for (size_t i=0; i<o.size(); i++) {
      dPoint p = o[i];
      if (i==0) move_to(p);
      else line_to(p);
    }
    if (close) close_path();
    return;
  }

  // lines with smoothing
  for (size_t i=0; i<o.size(); i++) {
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
CairoExtra::mkpath_shifted(const dMultiLine & o, bool close, double shift){
  for (const auto l: o) mkpath_shifted(l, close, shift);
}

void
CairoExtra::mkpath_shifted(const dLine & o, bool close, double shift){
  if (o.length()==0) return; // should be at least two different points
  for (auto p = o.begin(); p!= o.end(); ++p) {
    // previous/next different point
    auto pp(p), pn(p);
    bool is_start=false, is_end=false;
    while (*pp==*p){
      if (pp==o.begin()){
        pp = o.begin()+(o.size()-1);
        is_start=true;
      }
      else
        pp = p-1;
    }
    while (*pn==*p){
      if (pn+1==o.end()){
        pn = o.begin();
        is_end=true;
      }
      else
        pn = p+1;
    }
    auto v1 = norm(*p-*pp), v2 = norm(*pn-*p);
    dPoint p1 = *p + shift*dPoint(v1.y, -v1.x);
    dPoint p2 = *p + shift*dPoint(v2.y, -v2.x);

    if (!close){
      if (is_start) {move_to(p2); continue;}
      if (is_end) {line_to(p1); continue;}
    }
    bool outer = pscal(p1-*p, *pn-*p)<0;
    if (outer) {
      if (is_start) move_to(p1);
      else line_to(p1);
      line_to(p2);
    }
    else {
      auto c1 = sqrt((pscal(v1,v2) + 1.0)/2.0);
      dPoint p3;
      if (shift > c1*(dist(*p,*pn) + dist(*p,*pp))/2)
        p3 = (*pn+*pp)/2;
      else
        p3 = *p + fabs(shift)*norm(v2-v1)/c1;
      if (is_start) move_to(p3);
      else line_to(p3);
    }
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
  image=ImageR(w, h, IMAGE_32ARGB);

  surface = image_to_surface(image);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_img(const ImageR & img){
  w = img.width(); h=img.height();
  image=img; // increase refcount of image

  surface = image_to_surface(image);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_ps(const char *fname, int w_, int h_){
  w = w_; h=h_; image = ImageR();
  surface = Cairo::PsSurface::create(fname, w, h);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_pdf(const char *fname, int w_, int h_){
  w = w_; h=h_; image = ImageR();
  surface = Cairo::PdfSurface::create(fname, w, h);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}

void
CairoWrapper::set_surface_svg(const char *fname, int w_, int h_){
  w = w_; h=h_; image = ImageR();
  surface = Cairo::SvgSurface::create(fname, w, h);
  Cairo::RefPtr<CairoExtra>::operator=
    (cast_static(Cairo::Context::create(surface)));
}


