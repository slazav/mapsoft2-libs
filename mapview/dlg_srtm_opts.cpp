#include "dlg_srtm_opts.h"
#include "geo_render/gobj_srtm.h" // get_def_opt()

using namespace std;

Opt
DlgSrtmOpt::get_opt() const{
  Opt o;

  if (cnt->get_active()){
    o.put<double>("srtm_cnt", 1);
    o.put<double>("srtm_cnt_step", cnt_val->get_value());
  }
  else {
    o.put<double>("srtm_cnt", 0);
  }

  o.put("srtm_holes", holes->get_active());
  o.put("srtm_peaks", peaks->get_active());
  o.put("srtm_interp_holes", interp->get_active());


  if (m_none->get_active()){
    o.put<string>("srtm_draw_mode", "none");
  }
  if (m_heights->get_active()){
    bool sh = shades->get_active();
    o.put<string>("srtm_draw_mode", sh? "shades":"heights");
    o.put<int>("srtm_hmin", rh->get_v1());
    o.put<int>("srtm_hmax", rh->get_v2());
  }
  if (m_slopes->get_active()){
    o.put<string>("srtm_draw_mode", "slopes");
    o.put<int>("srtm_smin", rs->get_v1());
    o.put<int>("srtm_smax", rs->get_v2());
  }


  o.put<string>("srtm_dir", dir->get_text());
  return o;
}

void
DlgSrtmOpt::set_opt(const Opt & o){

  // set only options which are explicetely set,
  // keep others untouched:
  string mode = o.get<string>("srtm_draw_mode");

  if (mode == "none")
    m_none->set_active();
  if (mode == "heights")
    m_heights->set_active();
  if (mode == "shades"){
    m_heights->set_active();
    shades->set_active();
  }
  if (mode == "slopes")
    m_slopes->set_active();

  if (o.exists("srtm_hmin") || o.exists("srtm_hmax"))
    rh->set(
      o.get<int>("srtm_hmin", rh->get_v1()),
      o.get<int>("srtm_hmax", rh->get_v2())
    );
  if (o.exists("srtm_smin") || o.exists("srtm_smax"))
    rs->set(
      o.get<int>("srtm_smin", rs->get_v1()),
      o.get<int>("srtm_smax", rs->get_v2())
    );

  if (o.exists("srtm_dir"))
    dir->set_text(o.get("srtm_dir", ""));

  if (o.exists("srtm_cnt"))
    cnt->set_active(o.get<bool>("srtm_cnt", true));

  if (o.exists("srtm_cnt_step"))
    cnt_val->set_value(o.get<double>("srtm_cnt_step", 0));

  if (o.exists("srtm_peaks"))
    peaks->set_active(o.get<bool>("srtm_peaks", true));

  if (o.exists("srtm_interp_holes"))
    interp->set_active(o.get<bool>("srtm_interp_holes", true));

  if (o.exists("srtm_holes"))
    holes->set_active(o.get<bool>("srtm_holes", true));

}

void
DlgSrtmOpt::on_ch(int mode, Gtk::RadioButton *b){
  // No need to emit signal if changes does not
  // affect the current mode.
  // We get mode for which signal must be emitted:
  // 0 - all modes, 1 - cnt, 2 - heights/shades, 3 - slopes
  if (b && !b->get_active()) return; // signal from switching a button off
  if (!is_visible()) return;
  if ( ((mode == 1) && !cnt->get_active()) ||
       ((mode == 2) && !m_heights->get_active()) ||
       ((mode == 3) && !m_slopes->get_active()) ) return;
  signal_changed_.emit();
}

DlgSrtmOpt::DlgSrtmOpt():
    fdlg("SRTM data folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER){
  add_button (Gtk::Stock::OK,     Gtk::RESPONSE_OK);
  add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  auto cnt_adj = Gtk::Adjustment::create(50,0,9999);
  cnt     = manage(new Gtk::CheckButton("Draw contours"));
  cnt_val = manage(new Gtk::SpinButton(cnt_adj, 10, 0));

  peaks   = manage(new Gtk::CheckButton("Draw summits"));
  holes   = manage(new Gtk::CheckButton("Draw holes"));
  interp  = manage(new Gtk::CheckButton("Interpolate holes"));

  m_none =  manage(new Gtk::RadioButton("None"));
  Gtk::RadioButtonGroup gr = m_none->get_group();
  m_heights =  manage(new Gtk::RadioButton(gr, "Height"));
  m_slopes =  manage(new Gtk::RadioButton(gr, "Slopes"));
  shades = manage(new Gtk::CheckButton("Shades"));

  rh =  manage(
    new RainbowWidget(256,8, -999, 9999, 10, 0, RAINBOW_NORMAL));
  rs =  manage(
    new RainbowWidget(256,8, 0, 90, 1, 1, RAINBOW_BURNING));

  dirbtn = manage(new Gtk::Button("Change data folder..."));
  dir  = manage(new Gtk::Label("", Gtk::ALIGN_START));

  Gtk::Table *t = manage(new Gtk::Table(2,11));
  t->attach(*cnt,      0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*cnt_val,  1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*peaks,    0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*interp,   0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*holes,    0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*m_none,   0, 1, 4, 5, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*m_heights,0, 1, 5, 6, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*shades,   1, 2, 5, 6, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*rh,       0, 2, 6, 7, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*m_slopes, 0, 2, 7, 8, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*rs,       0, 2, 8, 9, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*dir,      0, 2, 9,10, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*dirbtn,   0, 1,10,11, Gtk::FILL, Gtk::SHRINK, 3, 3);

  get_vbox()->add(*t);

  m_none->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0, m_none));
  m_heights->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0, m_heights));
  m_slopes->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0, m_slopes));

  Gtk::RadioButton* b0(NULL);
  cnt->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0, b0));
  cnt_val->signal_value_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 1, b0));
  peaks->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0, b0));
  interp->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0, b0));
  holes->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0, b0));

  shades->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 2, b0));
  rh->signal_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 2, b0));
  rs->signal_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 3, b0));
  rh->set(0,5000);
  rs->set(35,50);

  dirbtn->signal_clicked().connect(
      sigc::mem_fun(this, &DlgSrtmOpt::on_dirbtn));

  /* file selection dialog */
  fdlg.signal_response().connect(
    sigc::mem_fun (this, &DlgSrtmOpt::on_fresult));
  fdlg.add_button("_Cancel", GTK_RESPONSE_CANCEL);
  fdlg.add_button("_Open",   GTK_RESPONSE_ACCEPT);
  fdlg.set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
  fdlg.set_transient_for(*this);

  set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
  set_opt(GObjSRTM::get_def_opt()); //set default options
}

void
DlgSrtmOpt::on_dirbtn(){
  fdlg.set_current_folder(dir->get_text());
  fdlg.show_all();
  return;
}

void
DlgSrtmOpt::on_fresult(int r){
  fdlg.hide();
  if (r == Gtk::RESPONSE_CANCEL) return;
  dir->set_text(fdlg.get_current_folder());
  signal_changed_.emit();
}
