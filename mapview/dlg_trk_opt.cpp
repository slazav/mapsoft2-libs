#include "dlg_trk_opt.h"
#include "geo_render/gobj_trk.h" // get_def_opt()

using namespace std;

Opt
DlgTrkOpt::get_opt() const{
  Opt o;
  if (m_normal->get_active())
    o.put<string>("trk_draw_mode", "normal");
  o.put<bool>("trk_draw_dots", dots->get_active());
  if (m_speed->get_active()) {
    o.put<string>("trk_draw_mode", "speed");
    o.put<int>("trk_draw_smin", rv->get_v1());
    o.put<int>("trk_draw_smax", rv->get_v2());
  }
  if (m_height->get_active()) {
    o.put<string>("trk_draw_mode", "height");
    o.put<int>("trk_draw_hmin", rh->get_v1());
    o.put<int>("trk_draw_hmax", rh->get_v2());
  }
  return o;
}

void
DlgTrkOpt::set_opt(const Opt & o){
  // set only options which are explicetely set,
  // keep others untouched:
  string mode = o.get<string>("trk_draw_mode", "");
  if (mode == "speed")  m_speed->set_active();
  if (mode == "height") m_height->set_active();
  if (mode == "normal") m_normal->set_active();

  if (o.exists("trk_draw_dots"))
    dots->set_active(o.get("trk_draw_dots", true));

  if (o.exists("trk_draw_smin") || o.exists("trk_draw_smax"))
    rv->set(
      o.get<int>("trk_draw_smin", rv->get_v1()),
      o.get<int>("trk_draw_smax", rv->get_v2())
    );

  if (o.exists("trk_draw_hmin") || o.exists("trk_draw_hmax"))
    rh->set(
      o.get<int>("trk_draw_hmin", rh->get_v1()),
      o.get<int>("trk_draw_hmax", rh->get_v2())
    );
}

void
DlgTrkOpt::on_ch(int mode, Gtk::RadioButton *b){
  // No need to emit signal if changes does not
  // affect the current mode.
  // We get mode for which signal must be emitted:
  // 0 - all modes, 1 - normal, 2 - speed, 3 - height
  if (b && !b->get_active()) return; // signal from switching a button off
  if (!is_visible()) return;
  if ( ((mode == 1) && !m_normal->get_active()) ||
       ((mode == 2) && !m_speed->get_active()) ||
       ((mode == 3) && !m_height->get_active()) ) return;
  signal_changed_.emit();
}

sigc::signal<void> &
DlgTrkOpt::signal_changed(){
  return signal_changed_;
}


DlgTrkOpt::DlgTrkOpt(){
  add_button (Gtk::Stock::OK,     Gtk::RESPONSE_OK);
  add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  Gtk::Frame *trk_frame = manage(new Gtk::Frame("Track drawing options"));
  m_normal =  manage(new Gtk::RadioButton("Normal"));
  Gtk::RadioButtonGroup gr = m_normal->get_group();
  m_speed  =  manage(new Gtk::RadioButton(gr, "Speed"));
  m_height =  manage(new Gtk::RadioButton(gr, "Height"));
  m_normal->set_active();

  dots   = manage(new Gtk::CheckButton("draw dots"));

  rv =  manage(
    new RainbowWidget(256,8, 0, 999, 1, 1, "BCGYRM"));
  rh =  manage(
    new RainbowWidget(256,8, -999, 9999, 10, 0, "BCGYRM"));
  rv->set(0,10);
  rh->set(0,2000);

  Gtk::Table *t = manage(new Gtk::Table(2,3));
  t->attach(*m_normal, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*dots,     1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*m_speed,  0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*rv,       1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*m_height, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*rh,       1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);
  trk_frame->add(*t);
  get_vbox()->add(*trk_frame);

  m_normal->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgTrkOpt::on_ch), 0, m_normal));
  m_speed->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgTrkOpt::on_ch), 0, m_speed));
  m_height->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgTrkOpt::on_ch), 0, m_height));

  Gtk::RadioButton* b0(NULL);
  dots->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgTrkOpt::on_ch), 1, b0));
  rv->signal_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgTrkOpt::on_ch), 2, b0));
  rh->signal_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgTrkOpt::on_ch), 3, b0));

  set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
  set_opt(GObjTrk::get_def_opt()); //set default options
}


