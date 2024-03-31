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


/**********************************************************/
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

/**********************************************************/
/// Panoramic view
#include "dlg_pano.h"
class AMSrtmPano : public ActionMode {
public:
    AMSrtmPano (Mapview * mapview) :
      ActionMode(mapview),
      dlg(&mapview->srtm),
      state(0)
    {
      dlg.signal_response().connect(
        sigc::hide(sigc::mem_fun (this, &AMSrtmPano::abort)));
      dlg.signal_go().connect(
        sigc::mem_fun (this, &AMSrtmPano::on_go));
      dlg.signal_point().connect(
        sigc::mem_fun (this, &AMSrtmPano::on_point));
      dlg.set_title(get_name());

      mapview->signal_srtm_conf().connect(
        sigc::mem_fun (this, &AMSrtmPano::on_reconf));

    }

    std::string get_name() override { return "Panoramic view"; }
    std::string get_desc() override {
      return "left click - set view point / set view direction; ctrl-left - change view point; right - close"; }

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
        dlg.set_target(p1);
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

/**********************************************************/

// ActionMode for selecting polygon/rectangular region
class ActionModeRegion : public ActionMode {
    dLine line;
    dRect rect;
public:
    ActionModeRegion (Mapview * mapview) : ActionMode(mapview) { }

    virtual void on_select(const dLine & l) = 0;

    std::string get_desc() override {
      return "left click - start line, add points; "
             "ctrl-left - start rectangle, remove line point; "
             "middle - finish; right - abort"; }

    void activate(const std::string & menu) override { abort(); }

    void abort() override {
      line.clear();
      rect = dRect();
      mapview->rubber.clear();
    }

    void handle_click(const iPoint p, const int button,
                      const Gdk::ModifierType & state) override {

      // start drawing
      if (line.size()==0 && rect.is_empty() && button == 1){
        // start rect
        if (state&Gdk::CONTROL_MASK) {
          rect.expand(p);
          mapview->rubber.clear();
          mapview->rubber.add_rect(p);
        }
        // start line
        else {
          line.push_back(p);
          mapview->rubber.add_line(p);
        }
        return;
      }

      // finish rect
      if (!rect.is_empty() && button == 1){
        rect.expand(p);
        on_select(rect_to_line(rect));
        mapview->rubber.clear();
        rect = dRect();
        return;
      }

      // continue line
      if (line.size()!=0 && button == 1){
        // remove point
        if (state&Gdk::CONTROL_MASK){
          if (line.size()>0){
            line.resize(line.size()-1);
          }
          if (mapview->rubber.size()>0){
            mapview->rubber.pop();
          }
          if (mapview->rubber.size()>0){
            RubberSegment s = mapview->rubber.pop();
            s.flags |= RUBBFL_MOUSE_P2;
            s.p2=iPoint(0,0);
            mapview->rubber.add(s);
          }
        }
        // add point
        else {
          line.push_back(p);
          // fix the last rubber segment
          if (mapview->rubber.size()>0){
            RubberSegment s = mapview->rubber.pop();
            s.flags &= ~RUBBFL_MOUSE;
            s.p2 = dPoint(p);
            mapview->rubber.add(s);
          }
          mapview->rubber.add_line(p);
        }
        return;
      }

      // abort line/rect
      if (button == 3) {
        abort();
        return;
      }

      // finish line
      if (line.size()!=0 && button == 2) {
        line.push_back(p);
        on_select(line);
        abort();
        return;
      }
    }

};

/**********************************************************/
// Cut overlay hole
class AMSrtmCut : public ActionModeRegion {
  dLine line;
  dRect rect;

  public:

    AMSrtmCut (Mapview * mapview) : ActionModeRegion(mapview) { }

    std::string get_name() override { return "Make hole in SRTM overlay"; }
    std::string get_icon() { return "cut"; }

    void on_select(const dLine & l) override{
      dLine l1(l);
      mapview->viewer.get_cnv().frw(l1);
      mapview->srtm.overlay_cut(l1);
      mapview->obj_srtm->redraw();
    }
};

/**********************************************************/
// Clear overlay
class AMSrtmClear : public ActionModeRegion {
  dLine line;
  dRect rect;

  public:

    AMSrtmClear (Mapview * mapview) : ActionModeRegion(mapview) { }

    std::string get_name() override { return "Clear SRTM overlay"; }
    std::string get_icon() { return "Clear"; }

    void on_select(const dLine & l) override{
      dLine l1(l);
      mapview->viewer.get_cnv().frw(l1);
      mapview->srtm.overlay_clear(l1);
      mapview->obj_srtm->redraw();
    }
};

#endif

