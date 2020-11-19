#ifndef AM_TRKS_H
#define AM_TRKS_H

/* Action modes for Tracks menu */
#include "am.h"
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

/**********************************************************/
// add track

#include "dlg_trk.h"

class AMTrkAdd : public ActionMode {
    GeoTrk trk;
    DlgTrk dlg;
    bool start;

    void on_result(int r){
      // new segment
      if (r == Gtk::RESPONSE_APPLY){
        start = true;
        if (mapview->rubber.size()>0) mapview->rubber.pop();
        return;
      }

      // OK button
      if (r == Gtk::RESPONSE_OK){
        dlg.dlg2trk(&trk);
        std::shared_ptr<GeoTrk> track(new GeoTrk(trk));
        mapview->panel_trks->add(track);
      }
      abort();
    }
  public:

    AMTrkAdd (Mapview * mapview) : ActionMode(mapview), start(true) {
      dlg.set_transient_for(*mapview);
      dlg.signal_response().connect(
        sigc::mem_fun (this, &AMTrkAdd::on_result));
      dlg.set_title(get_name());
    }

    std::string get_name() { return "Add Track"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::ADD; }

    void activate() { abort(); }

    void abort() {
      trk.clear();
      trk.comm="";
      mapview->rubber.clear();
      dlg.hide();
    }

    void handle_click(const iPoint p, const int button,
                      const Gdk::ModifierType & state) {

         if (button == 3) {
           abort();
           return;
         }

         if (trk.size() == 0){
           dlg.trk2dlg(&trk);
           dlg.set_hint("<b>Use mouse buttons to draw track:</b>\n"
                        "* <b>1.</b> Add point.\n"
                        "* <b>Ctrl-1.</b> Remove last point.\n"
                        "* <b>Shift-1.</b> Start new segment.\n"
                        "* <b>2.</b> Scroll map.\n"
                        "* <b>3.</b> Abort drawing.");
           dlg.show_all();
         }

        // remove point
        if (state&Gdk::CONTROL_MASK){
          if (trk.size()>0) trk.pop_back();
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
        else{
          GeoTpt pt;
          pt.dPoint::operator=(p);
          mapview->viewer.get_cnv().frw(pt);
          pt.start = (state&Gdk::SHIFT_MASK) || start;
//          pt.z = mapview->srtm.get_val_int4(pt);
          trk.push_back(pt);

          if (mapview->rubber.size()>0 && !start){
            RubberSegment s = mapview->rubber.pop();
            s.flags &= ~RUBBFL_MOUSE;
            s.p2 = pt.start ? s.p1 : dPoint(p);
            mapview->rubber.add(s);
          }
          mapview->rubber.add_line(p);
          start=false;
        }

        dlg.set_info(&trk);
    }
};


#endif

