#ifndef AM_SRTM_H
#define AM_SRTM_H

/* Action modes for DEM/SRTM menu */
#include "action_mode.h"
//#include "dlg_srtm_opts.h"

/**********************************************************/
// Show/Hide SRTM layer
class ShowSRTM : public ActionMode{
  int state;
  public:
    ShowSRTM (Mapview * mapview): ActionMode(mapview), state(0) { }

    std::string get_name() { return "Show/Hide SRTM layer"; }
    std::string get_icon() { return "view"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate() {
      state = (state+1)%2;
      if (state==1) mapview->open_srtm();
      else mapview->close_srtm();
    }
};

/*
// Edit SRTM options
class DEMOpts : public ActionMode {
public:
    DEMOpts (Mapview * mapview) : ActionMode(mapview) {
      dlg.set_transient_for(*mapview);
      dlg.signal_response().connect(
        sigc::mem_fun (this, &SrtmOpt::on_response));
      dlg.signal_changed().connect(
        sigc::bind(sigc::mem_fun (this, &SrtmOpt::on_response),1));

      dlg.set_title(get_name());
    }

    std::string get_name() { return "configure"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::PROPERTIES; }

    bool is_radio() { return false; }

    void activate() {
      dlg.set_opt(o);
      dlg.show_all();
    }

    void on_response(int r){
      if (r==Gtk::RESPONSE_CANCEL) mapview->srtm.set_opt(o);
      if (r>0) mapview->panel_misc.set_opt(dlg.get_opt());
      else dlg.hide_all();
    }

private:
    DlgSrtmOpt dlg;
    Options o;
};
*/

#endif

