#ifndef AM_VIEW_H
#define AM_VIEW_H

/* Action modes for View menu */
#include "am.h"

/**********************************************************/
// Toggle fullscreen mode

class FullScreen : public ActionMode{
    int state;
  public:
    FullScreen (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Fullscreen mode"; }
    std::string get_icon() { return "view-fullscreen"; }
    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      state = (state+1)%2;
      if (state==0) mapview->unfullscreen();
      else mapview->fullscreen();
    }
};

/**********************************************************/
// Hide panels

class HidePanels : public ActionMode{
    int state;
  public:
    HidePanels (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Hide/Show Panels"; }
    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>h"); }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      state = (state+1)%2;
      if (state==0){
        mapview->spanel.show();
        mapview->panels->show();
      }
      else {
        mapview->spanel.hide();
        mapview->panels->hide();
      }
    }
};

/**********************************************************/
// Show point information

#include "dlg_show_pt.h"
class ShowPt : public ActionMode, public DlgShowPt {
public:
    ShowPt (Mapview * mapview) : ActionMode(mapview) {
      set_transient_for(*mapview);
      signal_jump().connect(
          sigc::mem_fun (this, &ShowPt::on_jump));
      signal_response().connect(
        sigc::hide(sigc::mem_fun (this, &ShowPt::abort)));
      set_title(get_name());
    }

    std::string get_name() { return "Show point information"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::INFO; }

    void abort() {
      mapview->rubber.clear();
      hide();
    }

    void handle_click(const iPoint p, const int button,
                      const Gdk::ModifierType & state) {
      if (button == 3) {
        abort();
        return;
      }
      mapview->rubber.clear();
      mapview->rubber.add_cr_mark(p, false);
      dPoint pt(p);
      mapview->viewer.get_cnv().frw(pt);
      call(pt, 0 /* mapview->panel_misc.srtm.geth4(pt) */);
    }

    void on_jump(dPoint p){
      mapview->rubber.clear();
      mapview->viewer.get_cnv().bck(p);
      mapview->viewer.set_center(p,false);
      mapview->rubber.add_cr_mark(p, false);
      mapview->rubber.add_cr_mark(p, true);
    }
};


#endif
