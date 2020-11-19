#ifndef AM_EDIT_H
#define AM_EDIT_H

#include "am.h"

class AMEditData : public ActionMode {
int mystate;
GObjTrk  * trk;
GObjWpts * wpts;
size_t   idx;

public:
  AMEditData (Mapview * mapview) :
    ActionMode(mapview), mystate(0), trk(NULL), wpts(NULL) { }

  std::string get_name() override { return "Edit tracks and waypoints"; }

  void handle_click(const iPoint p, const int button,
                    const Gdk::ModifierType & state) override {
    if (button == 3) {
      abort();
      return;
    }
    switch (mystate){
      case 0: find_object(p, state); break;
      case 1: move_wpt(p, state); break;
      case 2: move_tpt(p, state); break;
      case 3: add_tpt(p, state); break;
    }
  }

  void abort() override {
    mapview->rubber.clear();
    mystate = 0;
  }


private:
  void find_object(iPoint p, const Gdk::ModifierType & state){

    // find waypoints
    auto res1 = mapview->panel_wpts->find_points(p);
    if (res1.size()){

      // TODO: do something with multiple choices (menu?).
      // Choose first point of the first WaypointList:
      if (res1.begin()->second.size()<1) return;
      wpts = res1.begin()->first;
      idx = *res1.begin()->second.begin();

      dPoint pt = wpts->get_point_crd(idx);

      mapview->rubber.add_cr_mark(pt, false, 3);
      mapview->rubber.add_cr_mark(iPoint(), true, 3);
      mapview->rubber.add_line(pt);
      mystate=1;
      return;
    }

    // find trackpoints
    auto res2 = mapview->panel_trks->find_points(p);
    if (res2.size()){

      // Choose first point of the first track:
      if (res2.begin()->second.size()<1) return;
      trk = res2.begin()->first;
      idx = *res2.begin()->second.begin();

      auto pts = trk->get_point_crd(idx);
      switch (pts.size()){
        case 0: return;
        case 1:
          mapview->rubber.add_line(pts[0]);
          break;
        case 2:
          mapview->rubber.add_line(pts[1]);
          break;
        case 3:
          mapview->rubber.add_line(pts[1]);
          mapview->rubber.add_line(pts[2]);
          break;
      }
      mapview->rubber.add_cr_mark(iPoint(), true, 3);
      mystate=2;
      return;
    }

    // find track segments
    auto res3 = mapview->panel_trks->find_segments(p);
    if (res3.size()){

      // Choose first segment of the first track:
      if (res3.begin()->second.size()<1) return;
      trk = res3.begin()->first;
      idx = *res3.begin()->second.begin();

      // find_segments returns onlyy visible segments.
      // pts should contain 2 or 3 points: start and end of the segment
      // and optional previous point (not needed here)
      auto pts = trk->get_point_crd(idx);
      if (pts.size()<2) return;
      mapview->rubber.add_line(pts[0]);
      mapview->rubber.add_line(pts[1]);
      mapview->rubber.add_cr_mark(iPoint(), true, 3);
      mystate=3;

      return;
    }

  }

  void move_wpt(iPoint p, const Gdk::ModifierType & state){
    wpts->set_point_crd(idx, p);
    mapview->rubber.clear();
    mystate=0;
  }

  void move_tpt(iPoint p, const Gdk::ModifierType & state){
    trk->set_point_crd(idx, p);
    mapview->rubber.clear();
    mystate=0;
  }

  void add_tpt(iPoint p, const Gdk::ModifierType & state){
    trk->add_point_crd(idx, p);
    mapview->rubber.clear();
    mystate=0;
  }


};


#endif
