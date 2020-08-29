#include "dlg_pano.h"

Opt
DlgPano::get_opt() const{
  Opt o;
  o.put("pano_pt", pt);
  o.put("pano_alt", dh->get_value());
  o.put("pano_rmax", mr->get_value());
  return o;
}

void
DlgPano::set_opt(const Opt & o) {
  pt = o.get<dPoint>("pano_pt", pt);

  if (o.exists("pano_alt"))
    dh->set_value(o.get<double>("pano_alt"));

  if (o.exists("pano_rmax"))
    mr->set_value(o.get<double>("pano_rmax"));

  gobj_pano.set_opt(o);
}



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
  Gtk::Label * azl = manage(new Gtk::Label("Azimuth, deg:", Gtk::ALIGN_END));
  Gtk::Label * dhl = manage(new Gtk::Label("Altitude, m:", Gtk::ALIGN_END));
  Gtk::Label * mrl = manage(new Gtk::Label("Max.Dist., km:", Gtk::ALIGN_END));

  Gtk::Table * t =   manage(new Gtk::Table(6,1));

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
      sigc::mem_fun(this, &DlgPano::on_ch));
  az->signal_value_changed().connect(
      sigc::mem_fun(this, &DlgPano::set_az));
  mr->signal_value_changed().connect(
      sigc::mem_fun(this, &DlgPano::on_ch));

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

  viewer.set_can_focus();
  viewer.grab_focus();
  viewer.set_xloop();
  viewer.set_bbox(iRect(0,0,w,w/2));

  set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
  set_opt(GObjPano::get_def_opt()); //set default options
}


void
DlgPano::on_ch(){
  gobj_pano.set_opt(get_opt());
}

void
DlgPano::set_origin(const dPoint & p){
  pt = p;
  Gtk::Dialog::show_all();
  gobj_pano.set_origin(pt);
}

void
DlgPano::set_dir(const dPoint & pt){
  dPoint pt0 = gobj_pano.get_origin();
  double width = gobj_pano.get_width();
  double angle = atan2((pt.x-pt0.x)*cos(pt0.y*M_PI/180), pt.y-pt0.y);
  rubber.clear();
  viewer.set_center(iPoint( width*angle/2.0/M_PI, viewer.get_center(false).y), false);
  iPoint p = gobj_pano.geo2xy(pt);
  if (p.y>0) rubber.add_cr_mark(p, false);
}

void
DlgPano::set_az(){
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
  rubber.clear();
  if (pg.y<90){
    rubber.add_cr_mark(p, false);
    if (state&Gdk::CONTROL_MASK){
      set_origin(pg);
      signal_go_.emit(pg);
    }
    else signal_point_.emit(pg);
  }
}

sigc::signal<void, dPoint>
DlgPano::signal_go(){
  return signal_go_;
}
sigc::signal<void, dPoint>
DlgPano::signal_point(){
  return signal_point_;
}
