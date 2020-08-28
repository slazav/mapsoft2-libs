///\cond HIDDEN (do not show this in Doxyden)

#include <gtkmm.h>
#include "err/err.h"
#include "mapview/css.h"

#include "mapview/dlg_err.h"
#include "mapview/dlg_confirm.h"
#include "mapview/dlg_show_pt.h"
#include "mapview/dlg_trk_opt.h"
#include "mapview/dlg_srtm_opts.h"
#include "mapview/dlg_pano.h"

class MyWindow : public Gtk::ApplicationWindow {

  // Error dialog
  DlgErr err;
  void on_err() {
    err.call(Err() << "This is a dialog for displaying arbitrary error messages."
       "There is no formatting here, just a line of text.");
  }

  // Confirmation dialog
  DlgConfirm conf;
  void on_conf(bool ok) {
    if (!ok)
      conf.call(
        sigc::bind(sigc::mem_fun(this, &MyWindow::on_conf), true));
    else
      std::cerr << "Confirmed!\n";
  }

  // Show point dialog
  DlgShowPt showpt;
  void on_showpt() {
    showpt.call(dPoint(70.56072,39.43434), 0);
  }
  void on_crds_jump(const dPoint & p){
    std::cerr << "Jump to: " << p << "\n";
  }

  // Track drawing options dialog
  DlgTrkOpt trkopt;
  void on_trkopt_ch() {
    std::cerr << trkopt.get_opt() << "\n";
  }
  void on_trkopt_res(int r) {
    trkopt.hide();
  }

  // SRTM drawing options dialog
  DlgSrtmOpt srtmopt;
  void on_srtmopt_ch() {
    std::cerr << srtmopt.get_opt() << "\n";
  }
  void on_srtmopt_res(int r) {
    srtmopt.hide();
  }

  // SRTM panoramic view
  DlgPano pano;
  void on_pano_point(const dPoint & p) {
    std::cerr << "mark point: " << p << "\n";
  }
  void on_pano_go(const dPoint & p) {
    std::cerr << "jump to: " << p << "\n";
  }

  public:

  /***********************************/
  MyWindow(){

    set_default_size(200,200);

    // Error dialog
    auto b_err = manage(new Gtk::Button("DlgErr"));
    b_err->signal_clicked().connect( sigc::mem_fun(this, &MyWindow::on_err));
    err.set_transient_for(*this);

    // Confirmation dialog
    auto b_conf = manage(new Gtk::Button("DlgConfirm"));
    b_conf->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(this, &MyWindow::on_conf), false));
    conf.set_transient_for(*this);

    // Show point dialog
    auto b_showpt = manage(new Gtk::Button("DlgShowPt"));
    b_showpt->signal_clicked().connect(
      sigc::mem_fun(this, &MyWindow::on_showpt));
    showpt.signal_jump().connect(
      sigc::mem_fun(this, &MyWindow::on_crds_jump));
    showpt.set_transient_for(*this);

    // Track drawing options dialog
    auto b_trkopt = manage(new Gtk::Button("DlgTrkOpt"));
    b_trkopt->signal_clicked().connect(
      sigc::mem_fun(trkopt, &DlgTrkOpt::show_all));
    trkopt.signal_changed().connect(
      sigc::mem_fun(this, &MyWindow::on_trkopt_ch));
    trkopt.signal_response().connect(
      sigc::mem_fun(this, &MyWindow::on_trkopt_res));
    trkopt.set_transient_for(*this);

    // SRTM drawing options dialog
    auto b_srtmopt = manage(new Gtk::Button("DlgSrtmOpt"));
    b_srtmopt->signal_clicked().connect(
      sigc::mem_fun(trkopt, &DlgSrtmOpt::show_all));
    srtmopt.signal_changed().connect(
      sigc::mem_fun(this, &MyWindow::on_srtmopt_ch));
    srtmopt.signal_response().connect(
      sigc::mem_fun(this, &MyWindow::on_srtmopt_res));
    srtmopt.set_transient_for(*this);

    // SRTM pano
    Opt pano_opts;
//    pano_opts.put("pano_pt", dPoint(29.5, 78.5));
    pano_opts.put("pano_pt", dPoint(78.5, 41.5));
    pano_opts.put("srtm_dir", "../../srtm/test_srtm");
    pano.set_opt(pano_opts);
    auto b_pano = manage(new Gtk::Button("DlgPano"));
    b_pano->signal_clicked().connect(
      sigc::mem_fun(pano, &DlgPano::show_all));
    pano.signal_point().connect(
      sigc::mem_fun(this, &MyWindow::on_pano_point));
    pano.signal_go().connect(
      sigc::mem_fun(this, &MyWindow::on_pano_go));
    pano.set_transient_for(*this);

    /***********************************/
    // Main vbox
    Gtk::VBox * vbox = manage(new Gtk::VBox);
    vbox->pack_start(*b_err,     false, true, 5);
    vbox->pack_start(*b_conf,    false, true, 5);
    vbox->pack_start(*b_showpt,  false, true, 5);
    vbox->pack_start(*b_trkopt,  false, true, 5);
    vbox->pack_start(*b_srtmopt, false, true, 5);
    vbox->pack_start(*b_pano,    false, true, 5);

    add (*vbox);
    load_css("./widgets.css", *this);
    show_all();
  }




};


int
main(int argc, char **argv){
  try {

    auto app = Gtk::Application::create();

    MyWindow w;
    app->run(w, argc, argv);

    return 0;

  }
  catch (Err & e) {
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond

