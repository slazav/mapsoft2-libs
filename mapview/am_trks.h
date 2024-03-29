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

    std::string get_name() override { return "Track drawing Opt"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::PROPERTIES; }

    bool is_radio() override { return false; }

    void activate(const std::string & menu) override {
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

    void on_result(int r){
      // new segment
      if (r == Gtk::RESPONSE_APPLY){
        trk.add_segment();
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

    AMTrkAdd (Mapview * mapview) : ActionMode(mapview) {
      dlg.set_transient_for(*mapview);
      dlg.signal_response().connect(
        sigc::mem_fun (this, &AMTrkAdd::on_result));
      dlg.set_title(get_name());
      dlg.add_button ("New Segment", Gtk::RESPONSE_APPLY);
    }

    std::string get_name() override { return "Add Track"; }
    std::string get_desc() override {
      return "left click - add points; ctrl-left - remove last point; "
             "shift-left - new segment; right - finish; ctrl-right - abort"; }

    Gtk::StockID get_stockid() { return Gtk::Stock::ADD; }

    void activate(const std::string & menu) override { abort(); }

    void abort() override {
      trk.clear();
      trk.comm="";
      mapview->rubber.clear();
      dlg.hide();
    }

    void handle_click(const iPoint p, const int button,
                      const Gdk::ModifierType & state) override {

         if (button == 3) {
           if (!(state&Gdk::CONTROL_MASK) && trk.size()){
             dlg.dlg2trk(&trk);
             std::shared_ptr<GeoTrk> track(new GeoTrk(trk));
             mapview->panel_trks->add(track);
           }
           abort();
           return;
         }

         if (button != 1) return;

         if (trk.size() == 0){
           dlg.trk2dlg(&trk);
           dlg.show_all();
         }

        // remove point
        if (state&Gdk::CONTROL_MASK){
          trk.del_last_point();
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
          GeoTpt pt(p);
          mapview->viewer.get_cnv().frw(pt);
//          pt.z = mapview->srtm.get_val_int4(pt);
          if (state&Gdk::SHIFT_MASK){
            trk.add_segment();
            if (mapview->rubber.size()>0) mapview->rubber.pop();
          }
          trk.add_point(pt);
          // first point of a segment?
          bool start = trk.size() && trk[trk.size()-1].size()==1;

          // fix the last rubber segment
          if (mapview->rubber.size()>0 && !start){
            RubberSegment s = mapview->rubber.pop();
            s.flags &= ~RUBBFL_MOUSE;
            s.p2 = dPoint(p);
            mapview->rubber.add(s);
          }
          mapview->rubber.add_line(p);
        }

        dlg.set_info(&trk);
    }
};

/**********************************************************/
// set track drawing options
class AMTrkDel : public ActionMode{
public:
    AMTrkDel (Mapview * mapview) : ActionMode(mapview) {}
    std::string get_name() override { return "Delete track"; }

    void handle_click(const iPoint p, const int button,
                      const Gdk::ModifierType & state) override{
      if (button != 1) return;

      auto res1 = mapview->panel_trks->find_points(p);
      if (res1.size()>0)
        mapview->panel_trks->remove(res1.begin()->first);

      auto res2 = mapview->panel_trks->find_segments(p);
      if (res2.size()>0)
        mapview->panel_trks->remove(res2.begin()->first);

   }
};

#endif

