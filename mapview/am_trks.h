#ifndef AM_TRKS_H
#define AM_TRKS_H

/* Action modes for Tracks menu */
#include "action_mode.h"
#include "dlg_trk_opt.h"

/**********************************************************/
// set track drawing options
class AMTrkOpt : public ActionMode{

public:
    AMTrkOpt (Mapview * mapview) : ActionMode(mapview) {
      dlg.set_transient_for(*mapview);
      dlg.signal_response().connect(
        sigc::mem_fun (this, &AMTrkOpt::on_response));
      dlg.signal_changed().connect(
        sigc::bind(sigc::mem_fun (this, &AMTrkOpt::on_response),1));
      dlg.set_title(get_name());
      o = mapview->opts;
    }

    std::string get_name() { return "Track drawing Opt"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::PROPERTIES; }

    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      dlg.set_opt(o);
      dlg.show_all();
    }

    void on_response(int r){
      if (r==Gtk::RESPONSE_CANCEL)
        mapview->panel_trks->set_opt(o);

      if (r>0)
        mapview->panel_trks->set_opt(dlg.get_opt());
      else
        dlg.hide();
    }

private:
    DlgTrkOpt dlg;
    Opt o;
};

#endif

