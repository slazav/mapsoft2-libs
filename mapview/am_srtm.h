#ifndef AM_SRTM_H
#define AM_SRTM_H

/* Action modes for DEM/SRTM menu */
#include "action_mode.h"

/**********************************************************/
// Show/Hide SRTM layer
class AMShowSRTM : public ActionMode{
  int state;
  public:
    AMShowSRTM (Mapview * mapview): ActionMode(mapview), state(0) { }

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


// Edit SRTM options
#include "dlg_srtm_opts.h"
class AMSrtmOpts : public ActionMode, public DlgSrtmOpt {
public:
    AMSrtmOpts (Mapview * mapview) : ActionMode(mapview) {
      set_transient_for(*mapview);
      signal_response().connect(
        sigc::mem_fun (this, &AMSrtmOpts::on_response));
      signal_changed().connect(
        sigc::bind(sigc::mem_fun (this, &AMSrtmOpts::on_response),1));
      set_title(get_name());
      o = mapview->opts;
    }

    std::string get_name() { return "SRTM drawing options"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::PROPERTIES; }

    bool is_radio() { return false; }

    void activate() {
      set_opt(o);
      show_all();
    }

    void on_response(int r){
      // Unlike trk_opt dialog we need to emit signal_redraw_me explicitely.
      // This is because tracks are located inside gobj_multi, which emits
      // the signal. Something should be changed here...
      if (r==Gtk::RESPONSE_CANCEL){
        mapview->srtm->set_opt(o);
        mapview->srtm->signal_redraw_me().emit(iRect());
      }
      if (r>0) {
        mapview->srtm->set_opt(get_opt());
        mapview->srtm->signal_redraw_me().emit(iRect());
      }
      else hide();
    }

private:
    Opt o;
};

#endif

