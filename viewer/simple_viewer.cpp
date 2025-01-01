#include "simple_viewer.h"

#include <cassert>
#include <climits>
#include "image/image_r.h"
#include "geom/rect.h"

// Do not do rescaling if bbox exists and
// its size exceeds limits:
#define MIN_BBOX_LIMIT  (1<<4)
#define MAX_BBOX_LIMIT  (1<<30)

SimpleViewer::SimpleViewer(GObj * o) :
    cnv(std::shared_ptr<ConvBase>(new ConvBase)),
    on_drag(false),
    bgcolor(0xFF000000),
    sc(1.0),
    xloop(false), yloop(false), obj(o) {

  // which events we want to recieve
  set_events (
    Gdk::BUTTON_PRESS_MASK |
    Gdk::BUTTON_RELEASE_MASK |
    Gdk::SCROLL_MASK |
    Gdk::POINTER_MOTION_MASK |
    Gdk::POINTER_MOTION_HINT_MASK );

  // connect signal from the object
  obj->signal_redraw_me().connect(
    sigc::mem_fun(this, &SimpleViewer::redraw));

  // suppress default themed drawing of the widget's background
  set_app_paintable();
  reset_bbox();
}

/***********************************************************/

void
SimpleViewer::set_origin (iPoint p) {
  if (!obj) return;

  int w=get_width();
  int h=get_height();

  if (bbox) {
    if (xloop){ p.x = p.x % (int)rint(bbox.w); }
    else {
      if (p.x + w >= bbox.x + bbox.w) p.x = bbox.x + bbox.w - w - 1;
      if (p.x < bbox.x) p.x=bbox.x;
    }

    if (p.y + h >= bbox.y + bbox.h) p.y = bbox.y+ bbox.h - h - 1;
    if (p.y < bbox.y) p.y=bbox.y;
  }

  // now win->scroll invalidates the whole window,
  // see Scrolling section in
  //   https://developer.gnome.org/gtk3/stable/chap-drawing-model.html
  auto win = get_window();
  if (win) win->scroll(origin.x-p.x, origin.y-p.y);

  origin = p;
  signal_ch_origin_.emit(p);
}

dPoint
SimpleViewer::get_center (bool obj_crd) const {
  dPoint p = origin + iPoint(get_width(), get_height())/2;
  if (obj_crd && cnv) cnv->frw(p);
  return p;
}

void
SimpleViewer::set_center(dPoint new_center, bool obj_crd){
  if (obj_crd && cnv) cnv->bck(new_center);
  set_origin(iPoint(rint(new_center)) - iPoint(get_width(), get_height())/2);
}

dRect
SimpleViewer::get_range (bool obj_crd) const {
  dRect r = iRect(origin.x, origin.y, get_width(), get_height());
  if (obj_crd && cnv) return cnv->frw_acc(r);
  return r;
}

void
SimpleViewer::set_range(dRect dst, bool obj_crd){

  dRect src = get_range(false);

  // It is possible that coordinates can not be converted,
  // then just skip conversion
  if (obj_crd && cnv) try {
    dst = cnv->bck_acc(dst);
  } catch(Err & e) {}

  // calculate scaling factor (power of 2)
  double k = 1;
  while (src.w > 2.1*k*dst.w || src.h > 2.1*k*dst.h) k*=2;
  while (src.w < 0.9*k*dst.w || src.h < 0.9*k*dst.h) k/=2;

  // avoid rounding errors: first scaling, then moving
  rescale(k);
  set_center(dst.cnt()*k, false);
}


/***********************************************************/

void
SimpleViewer::redraw (const iRect & range){
  if (range) queue_draw_area(range.x, range.y, range.w, range.h);
  else queue_draw();
}

void
SimpleViewer::set_cnv(std::shared_ptr<ConvBase> c, bool fix_range){
  dRect r = get_range(true);
  cnv = c;
  reset_bbox();
  obj->set_cnv(cnv);
  if (fix_range) set_range(r, true);
  else set_origin(iPoint(0,0));
}

void
SimpleViewer::set_opt(const Opt & o){
  opt = o;
  obj->set_opt(o);
}

void
SimpleViewer::rescale(const double k, const iPoint & cnt){
  // limit scaling according to the bbox size:
  if (bbox){
    if (k < 1 && (bbox.w < MIN_BBOX_LIMIT || bbox.h < MIN_BBOX_LIMIT)) return;
    if (k > 1 && (bbox.w > MAX_BBOX_LIMIT || bbox.h > MAX_BBOX_LIMIT)) return;
  }

  signal_on_rescale_.emit(k);
  iPoint wsize(get_width(), get_height());
  iPoint wcenter = get_origin() + cnt;
  wcenter=iPoint(wcenter.x * k, wcenter.y * k);
  if (bbox) bbox *= k;
  if (cnv){
    cnv->rescale_src(1.0/k);
    obj->set_cnv(cnv);
  }
  // set_origin() should be at the end, because signal_ch_origin
  // is emitted here, cnv should be already set.
  set_origin(wcenter - cnt);
}

/***********************************************************/

void
SimpleViewer::draw(const CairoWrapper & crw, const iRect & r){
  if (!r) {redraw(); return;}
  signal_busy_.emit();

  // some objects want to draw on an image, without using
  // Cairo::Context. We create an image, make new CairoWrapper
  // and then transfer information back. Not very good-looking solution.

  // TODO: remove such objects, remove extra CairoWrapper
  // add correct clipping for original context.

  CairoWrapper crw1;
  crw1.set_surface_img(r.w, r.h);
  crw1->set_color(bgcolor);
  crw1->paint();

  // It could be that viewer.bbox does not cover the whole
  // range. It xloop is set we want to draw a few pictures.
  // Calculate shifts:
  int x1=0, x2=1;
  if (bbox) {
    x1 = floor((origin.x + r.x - bbox.x) / (double)bbox.w);
    x2 =  ceil((origin.x + r.x + r.w - bbox.x) / (double)bbox.w)+1;
  }
  for (int x = x1; x<x2; x++) {
    // if xloop = false we draw only x=0
    if (!get_xloop() && x!=0) continue;
    if (!obj) continue;

    iRect draw_range = r+origin;
    if (bbox) {
      draw_range.x -= x*bbox.x;
      draw_range.intersect(bbox);
      if (draw_range.is_zsize()) continue;
    }

    if (obj->check(draw_range)==GObj::FILL_NONE) continue;

    crw1->save();
    crw1->translate(-r.tlc()-origin-iPoint(x*bbox.x,0));
    obj->prepare_range(draw_range);
    obj->draw(crw1, draw_range);
    crw1->restore();
  }
  crw1.get_surface()->flush();
  crw->set_source(crw1.get_surface(), r.x, r.y);
  crw->paint();
  signal_idle_.emit();
}

bool
SimpleViewer::on_draw (const Cairo::RefPtr<Cairo::Context> & cr){
  std::vector<Cairo::Rectangle> rects;
  cr->copy_clip_rectangle_list(rects);
  CairoWrapper crw(cr);

  for (auto const & r:rects)
    draw(crw, dRect(r.x,r.y,r.width,r.height));

  return false;
}

bool
SimpleViewer::on_button_press_event (GdkEventButton * event) {
  drag_pos = get_origin() + iPoint((int)event->x, (int)event->y);
  drag_start = get_origin();
  if ((event->button == 1) || (event->button == 2)) on_drag=true;
  return false;
}

bool
SimpleViewer::on_button_release_event (GdkEventButton * event) {
  iPoint p;
  Gdk::ModifierType state;
  get_window()->get_pointer(p.x,p.y, state);
  if ((event->button == 1) || (event->button == 2)){
    on_drag=false;
    if (dist(drag_start, get_origin()) > 5) return true;
  }
  signal_click_.emit(drag_pos, event->button, state);
  return false;
}

bool
SimpleViewer::on_motion_notify_event (GdkEventMotion * event) {
  if (!event->is_hint) return false;
  if (on_drag){
    iPoint p((int)event->x, (int)event->y);
    set_origin(drag_pos - p);
  }
  return false;
}

// Note: grabbing focus and processing keypress events
// from the viewer itself is not so simple, and prevents
// other key processing.
// see http://www.mail-archive.com/gtk-list@gnome.org/msg03381.html
// It's better to connect this function to the main window's signal.
bool
SimpleViewer::on_key_press(GdkEventKey * event) {
  switch (event->keyval) {
    case 43:
    case 61:
    case 65451: // + =
      rescale(2.0);
      return true;
    case 45:
    case 95:
    case 65453: // _ -
      rescale(0.5);
      return true;
    case 'r':
    case 'R': // refresh
      redraw();
      return true;
  }
  return false;
}

bool
SimpleViewer::on_scroll_event(GdkEventScroll * event) {
  double scale = event->direction ? 0.5:2.0;
  rescale(scale, iPoint(event->x, event->y));
  return true;
}
