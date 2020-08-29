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
        mapview->obj_srtm->set_opt(o);
        mapview->signal_srtm_conf().emit();
      }
      if (r>0) {
        mapview->obj_srtm->set_opt(get_opt());
        mapview->signal_srtm_conf().emit();
      }
      else hide();
    }

private:
    Opt o;
};

/// Panoramic view
#include "dlg_pano.h"
class AMPano : public ActionMode {
public:
    AMPano (Mapview * mapview) :
      ActionMode(mapview),
      dlg(&mapview->srtm),
      state(0)
    {
      dlg.signal_response().connect(
        sigc::hide(sigc::mem_fun (this, &AMPano::abort)));
      dlg.signal_go().connect(
        sigc::mem_fun (this, &AMPano::on_go));
      dlg.signal_point().connect(
        sigc::mem_fun (this, &AMPano::on_point));
      dlg.set_title(get_name());

      mapview->signal_srtm_conf().connect(
        sigc::mem_fun (this, &AMPano::on_reconf));

    }

    std::string get_name() { return "Panoramic view"; }
//    Gtk::StockID get_stockid() { return Gtk::Stock::INFO; }

    void abort() {
      state=0;
      dlg.hide();
      mapview->rubber.clear();
    }

    void handle_click(iPoint p, const Gdk::ModifierType & mod) {
      if (state==0 || mod&Gdk::CONTROL_MASK){ // first click
        state=1; p0=p;
        mapview->viewer.get_cnv().frw(p0);
        mapview->rubber.clear();
        mapview->rubber.add_sq_mark(p, false);
        dlg.show_all();
        dlg.set_origin(p0);
      }
      else{ // next click
        dPoint p0i(p0), p1(p);
        mapview->viewer.get_cnv().bck(p0i);
        mapview->viewer.get_cnv().frw(p1);
        mapview->rubber.clear();
        mapview->rubber.add_cr_mark(p0i, false);
        mapview->rubber.add_cr_mark(p, false);
        mapview->rubber.add_line(p,p0i);
        dlg.set_dir(p1);
      }
    }

    void on_point(dPoint p){
      dPoint p0i(p0);
      mapview->viewer.get_cnv().bck(p);
      mapview->viewer.get_cnv().bck(p0i);
      mapview->rubber.clear();
      mapview->rubber.add_cr_mark(p0i, false);
      mapview->rubber.add_cr_mark(p, false);
      mapview->rubber.add_line(p,p0i);
    }
    void on_go(dPoint p){
      dlg.set_origin(p);
      p0=p; mapview->viewer.get_cnv().bck(p); state=1;
      mapview->rubber.clear();
      mapview->rubber.add_cr_mark(p, false);
      mapview->viewer.set_center(p, false);
    }

    void on_reconf(){ dlg.redraw(); }

private:
    DlgPano dlg;
    int state; // first/next click;
    dPoint p0;
};


#endif

