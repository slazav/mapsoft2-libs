#ifndef AM_SRTM_H
#define AM_SRTM_H

/* Action modes for DEM/SRTM menu */
#include "am.h"

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

    void activate(const std::string & menu) {
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
      o.put_missing(mapview->obj_srtm->get_def_opt());
      set_opt(o);
    }

    std::string get_name() { return "SRTM drawing options"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::PROPERTIES; }

    bool is_radio() { return false; }

    void activate(const std::string & menu) { show_all(); }

    void on_response(int r){
      // Unlike trk_opt dialog we need to emit signal_redraw_me explicitely.
      // This is because tracks are located inside gobj_multi, which emits
      // the signal. Something should be changed here...

      // Cancel button: set old options. DlgSrtmOpt will emit
      // signal_changed and this function will be called again with r=1
      if (r==Gtk::RESPONSE_CANCEL){
        set_opt(o);
        hide();
        return;
      }
      // OK button: just save options, do not redraw anything
      if (r==Gtk::RESPONSE_OK){
        o = get_opt();
        hide();
        return;
      }

      // Any changes: update options in srtm layer
      mapview->obj_srtm->set_opt(get_opt());
      mapview->signal_srtm_conf().emit();
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

    std::string get_name() override { return "Panoramic view"; }
//    Gtk::StockID get_stockid() { return Gtk::Stock::INFO; }

    void abort() override {
      state=0;
      dlg.hide();
      mapview->rubber.clear();
    }

    void handle_click(const iPoint p, const int button,
                      const Gdk::ModifierType & mod) override {

      if (button == 3) {
        abort();
        return;
      }

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

private:
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

    DlgPano dlg;
    int state; // first/next click;
    dPoint p0;
};


#endif

