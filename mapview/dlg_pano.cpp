#include "dlg_pano.h"
#include <cfloat> // NAN

DlgPano::DlgPano(SRTM * s): gobj_pano(s, Opt()),
                    viewer(&gobj_pano),
                    rubber(&viewer){

  signal_response().connect(
      sigc::hide(sigc::mem_fun(this, &DlgPano::hide)));

  auto dh_adj = Gtk::Adjustment::create(20,0,9999,10);
  auto az_adj = Gtk::Adjustment::create(0,0,360,10);
  auto mr_adj = Gtk::Adjustment::create(60,0,999,10);

  az = manage(new Gtk::SpinButton(az_adj));
  dh = manage(new Gtk::SpinButton(dh_adj));
  mr = manage(new Gtk::SpinButton(mr_adj));
  az->set_wrap();

  auto azl = manage(new Gtk::Label("Azimuth, deg:", Gtk::ALIGN_END));
  auto dhl = manage(new Gtk::Label("Altitude, m:", Gtk::ALIGN_END));
  auto mrl = manage(new Gtk::Label("Max.Dist., km:", Gtk::ALIGN_END));

  auto t =   manage(new Gtk::Table(6,1));

      //  widget    l  r  t  b  x       y
  t->attach(*azl,   0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*az,    1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*dhl,   2, 3, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*dh,    3, 4, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*mrl,   4, 5, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*mr,    5, 6, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);

  get_vbox()->pack_start (viewer, true, true);
  get_vbox()->pack_start (*t, false, true);
  add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  dh->signal_value_changed().connect(
      sigc::mem_fun(this, &DlgPano::on_set_dh));
  az->signal_value_changed().connect(
      sigc::mem_fun(this, &DlgPano::on_set_az));
  mr->signal_value_changed().connect(
      sigc::mem_fun(this, &DlgPano::on_set_mr));

  viewer.signal_ch_origin().connect(
      sigc::mem_fun(this, &DlgPano::get_az));

  signal_key_press_event().connect (
    sigc::mem_fun (this, &DlgPano::on_key_press));

  // connect viewer click
  viewer.signal_click().connect(
    sigc::mem_fun (this, &DlgPano::click));

  // Viewer is not realized yet, we don't know its size,
  // we can't use set_center
  set_default_size(640,480);
  int w = gobj_pano.get_width();
  viewer.set_origin(iPoint(w/2-320, w/4 - 240));
  az->set_value(180);
  dh->set_value(gobj_pano.get_dh());
  mr->set_value(gobj_pano.get_mr()/1000.0);

  viewer.set_can_focus();
  viewer.grab_focus();
  viewer.set_xloop();
  viewer.set_bbox(iRect(0,0,w,w/2));

  set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
  target = dPoint(NAN,NAN);
}

void
DlgPano::set_origin(const dPoint & p){
  Gtk::Dialog::show_all();
  gobj_pano.set_p0(p);
  if (std::isnan(target.x)) target = p; // initial call
  set_target(target); // keep same target
}

void
DlgPano::set_target(const dPoint & pt, bool scroll){
  target = pt;
  dPoint pt0 = gobj_pano.get_p0();
  double width = gobj_pano.get_width();
  double angle = atan2((pt.x-pt0.x)*cos(pt0.y*M_PI/180), pt.y-pt0.y);
  rubber.clear();
  if (scroll)
    viewer.set_center(iPoint( width*angle/2.0/M_PI, viewer.get_center(false).y), false);
  iPoint p = gobj_pano.geo2xy(pt);
  if (p.y>0) rubber.add_cr_mark(p, false);
  signal_point_.emit(pt);
}

void
DlgPano::on_set_az(){
  if (viewer.is_on_drag()) return;
  viewer.set_center(iPoint(
    gobj_pano.get_width()/360.0*az->get_value(),
    viewer.get_center(false).y), false);
}

void
DlgPano::get_az(const iPoint & p){
  double a = viewer.get_center(false).x * 360.0/gobj_pano.get_width();
  while (a>360) a-=360;
  while (a<0)   a+=360;
  az->set_value(a);
}

void
DlgPano::on_set_mr(){
  if (viewer.is_on_drag()) return;
  gobj_pano.set_mr(mr->get_value() * 1000); // km -> m
}

void
DlgPano::on_set_dh(){
  if (viewer.is_on_drag()) return;
  gobj_pano.set_dh(dh->get_value());
}


bool
DlgPano::on_key_press(GdkEventKey * event) {
  switch (event->keyval) {
    case 43:
    case 61:
    case 65451: // + =
      viewer.rescale(2);
      return true;
    case 45:
    case 95:
    case 65453: // _ -
      viewer.rescale(0.5);
      return true;
  }
  return false;
}

void
DlgPano::click (iPoint p, int button, const Gdk::ModifierType & state) {
  dPoint pg=gobj_pano.xy2geo(p);
  if (pg.y>90) return;
  if (state&Gdk::CONTROL_MASK) set_origin(pg);
  else set_target(pg, false);
}

sigc::signal<void, dPoint>
DlgPano::signal_go(){
  return signal_go_;
}
sigc::signal<void, dPoint>
DlgPano::signal_point(){
  return signal_point_;
}
