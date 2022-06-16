#include "w_coord_box.h"
#include <sstream>
#include <iomanip>

CoordBox::CoordBox(): cnv("WGS"){
  init();
}

/*
CoordBox::CoordBox(BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& builder) :
      Gtk::Frame(cobject){
  init();
}
*/

void
CoordBox::init(){
  set_label("Coordinates:");

  // Entry
  coords = manage(new Gtk::Entry);
  coords->set_width_chars(20);
  coords->signal_changed().connect(
    sigc::mem_fun(this, &CoordBox::on_change));

  // Projection
  proj_cb  = manage(new CBProj());
  cnv = ConvGeo(proj_cb->get_active_id());
  proj_cb->signal_changed().connect(
    sigc::mem_fun(this, &CoordBox::on_conv));

  // Jump button
  Gtk::Button *jb = manage(new Gtk::Button);
  Gtk::IconSize isize=Gtk::ICON_SIZE_MENU;
  jb->set_relief(Gtk::RELIEF_NONE);
  jb->set_image(*manage(new Gtk::Image(Gtk::Stock::JUMP_TO, isize)));
  jb->signal_clicked().connect( sigc::mem_fun(this, &CoordBox::on_jump));
  jb->set_tooltip_text("Jump to coordinates");

  // Labels
  Gtk::Label * lproj = manage(new Gtk::Label);
  lproj->set_text("Proj:");
  lproj->set_alignment(Gtk::ALIGN_END);

  // Main table
  Gtk::Table * table = manage(new Gtk::Table(4,2));
            //  widget    l  r  t  b  x       y
  table->attach(*coords,  0, 3, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*lproj,   0, 1, 1, 2, Gtk::EXPAND|Gtk::SHRINK, Gtk::SHRINK, 3, 3);
  table->attach(*jb,      3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
  table->attach(*proj_cb, 1, 3, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  add(*table);

  // set widget style (for use in css)
  get_style_context()->add_class("coord_box_widget");
}

void
CoordBox::set_ll(const dPoint & p){
  dPoint pc(p);
  cnv.bck(pc);
  int prec = cnv.is_src_deg()? 7:0;
  std::ostringstream s;
  s << std::fixed << std::setprecision(prec) << pc.x << " " << pc.y;
  coords->set_text(s.str());
  old_pt = pc;
}

dPoint
CoordBox::get_xy(){
  std::istringstream s(coords->get_text());
  double x,y;
  char c;
  s >> x;
  if (!s.eof()) s >> std::ws;
  if (!s) return old_pt;
  do {s >> c;} while (s && (c==',' || c==';'));
  s.putback(c);
  if (!s) return old_pt;
  s >> y;
  return dPoint(x,y);
}

dPoint
CoordBox::get_ll(){
  dPoint ret = get_xy();
  cnv.frw(ret);
  return ret;
}

void
CoordBox::on_conv(){
  dPoint pt = get_ll();
  cnv = ConvGeo(proj_cb->get_active_id());
  set_ll(pt);
}

void
CoordBox::on_change(){
  signal_changed_.emit();
}

void
CoordBox::on_jump(){
  signal_jump_.emit(get_ll());
}

sigc::signal<void> &
CoordBox::signal_changed(){
  return signal_changed_;
}

sigc::signal<void, dPoint> &
CoordBox::signal_jump(){
  return signal_jump_;
}
