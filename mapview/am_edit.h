#ifndef AM_EDIT_H
#define AM_EDIT_H

#include "am.h"

/* Edit Geodata mode.

default regime -> click on waypoint, track point or track interval (visible line between points):
* Left button: start moving waypoint or trackpoint, or add point to the interval.
* Ctrl-left button: delete waypoint, trackpoint, or splin track at the given interval.
* right button: show waypoint, track point, or track interval menu.

Waypoint menu:
* move waypoint (same as left click on waypoint)
* delete waypoint (same as ctrl-left click on waypoint)

Trackpoint menu:
* move track point (same as left click on the point)
* delete track point (same as ctrl-left click on the point)
* add segment: add new segment to the track
* delete segment: delete track segment which contains this point
* delete track

Track interval menu:
* add trackpoint (same as left click on the interval)
* split track (same as ctrl-left click on the interval)
* add segment: add new segment to the track
* delete segment: delete track segment which contains this interval
* delete track

*/

class AMEditData : public ActionMode {
int mystate;
PanelTrks::ptr_t trk;
PanelWpts::ptr_t wpts;
size_t   idx;
dLine pts; // points for adding track parts

Glib::RefPtr<Gtk::ActionGroup> actions;
Glib::RefPtr<Gtk::UIManager> ui_manager;
Gtk::Menu *popup_menu_tpt, *popup_menu_tseg, *popup_menu_wpt;

public:
  AMEditData (Mapview * mapview) :
      ActionMode(mapview), mystate(0), trk(NULL), wpts(NULL) {

    actions = Gtk::ActionGroup::create();
    ui_manager = Gtk::UIManager::create();
    ui_manager->insert_action_group(actions);
    mapview->add_accel_group(ui_manager->get_accel_group());

    actions->add(
      Gtk::Action::create("EditData:tpt:move", "Move track point", ""),
      sigc::mem_fun(this, &AMEditData::move_tpt_start));
    actions->add(
      Gtk::Action::create("EditData:tpt:del", "Delete track point", ""),
      sigc::mem_fun(this, &AMEditData::del_tpt));

    actions->add(
      Gtk::Action::create("EditData:wpt:move", "Move waypoint", ""),
      sigc::mem_fun(this, &AMEditData::move_wpt_start));
    actions->add(
      Gtk::Action::create("EditData:wpt:del", "Delete waypoint", ""),
      sigc::mem_fun(this, &AMEditData::del_wpt));

    actions->add(
      Gtk::Action::create("EditData:tseg:addpt", "Add trackpoint", ""),
      sigc::mem_fun(this, &AMEditData::add_tpt_start));
    actions->add(
      Gtk::Action::create("EditData:tseg:split", "Split track", ""),
      sigc::mem_fun(this, &AMEditData::split_trk));

    actions->add(
      Gtk::Action::create("EditData:delseg", "Delete segment", ""),
      sigc::mem_fun(this, &AMEditData::del_trkseg));
    actions->add(
      Gtk::Action::create("EditData:deltrk", "Delete track", ""),
      sigc::mem_fun(this, &AMEditData::del_trk));
    actions->add(
      Gtk::Action::create("EditData:addseg", "Add new segment", ""),
      sigc::mem_fun(this, &AMEditData::add_trkseg_start));


    ui_manager->add_ui_from_string(
      "<ui>"
      "  <popup name='EditData:wpt'>"
      "    <menuitem action='EditData:wpt:move'/>"
      "    <menuitem action='EditData:wpt:del'/>"
      "  </popup>"
      "  <popup name='EditData:tpt'>"
      "    <menuitem action='EditData:tpt:move'/>"
      "    <menuitem action='EditData:tpt:del'/>"
      "    <menuitem action='EditData:addseg'/>"
      "    <menuitem action='EditData:delseg'/>"
      "    <menuitem action='EditData:deltrk'/>"
      "  </popup>"
      "  <popup name='EditData:tseg'>"
      "    <menuitem action='EditData:tseg:addpt'/>"
      "    <menuitem action='EditData:tseg:split'/>"
      "    <menuitem action='EditData:addseg'/>"
      "    <menuitem action='EditData:delseg'/>"
      "    <menuitem action='EditData:deltrk'/>"
      "  </popup>"
      "</ui>"
      );
    popup_menu_wpt  = (Gtk::Menu *)ui_manager->get_widget("/EditData:wpt");
    popup_menu_tpt  = (Gtk::Menu *)ui_manager->get_widget("/EditData:tpt");
    popup_menu_tseg = (Gtk::Menu *)ui_manager->get_widget("/EditData:tseg");
  }

  std::string get_name() override { return "Edit tracks and waypoints"; }

  void handle_click(const iPoint p, const int button,
                    const Gdk::ModifierType & state) override {
    switch (mystate){
      case 0: find_object(p, button, state); break;
      case 1: move_wpt_finish(p, button); break;
      case 2: move_tpt_finish(p, button); break;
      case 3: add_tpt_finish(p, button); break;
      case 4: add_trkseg_finish(p, button, state); break;
    }
  }

  void abort() override {
    mapview->rubber.clear();
    mystate = 0;
  }


private:
  void find_object(const iPoint p, const int button,
                   const Gdk::ModifierType & state){

    // find waypoints
    auto res1 = mapview->panel_wpts->find_points(p);
    if (res1.size()){

      // TODO: do something with multiple choices (menu?).
      // Choose first point of the first WaypointList:
      if (res1.begin()->second.size()<1) return;
      wpts = res1.begin()->first;
      idx = *res1.begin()->second.begin();

      if (button == 3) popup_menu_wpt->popup(button, 0);
      else if (button == 1 && state&Gdk::CONTROL_MASK) del_wpt();
      else if (button == 1) move_wpt_start();
      return;
    }

    // find trackpoints
    auto res2 = mapview->panel_trks->find_points(p);
    if (res2.size()){

      // Choose first point of the first track:
      if (res2.begin()->second.size()<1) return;
      trk = res2.begin()->first;
      idx = *res2.begin()->second.begin();

      if (button == 3) popup_menu_tpt->popup(button, 0);
      else if (button == 1 && state&Gdk::CONTROL_MASK) del_tpt();
      else if (button == 1) move_tpt_start();
      return;
    }

    // find track segments
    auto res3 = mapview->panel_trks->find_segments(p);
    if (res3.size()){

      // Choose first segment of the first track:
      if (res3.begin()->second.size()<1) return;
      trk = res3.begin()->first;
      idx = *res3.begin()->second.begin();

      if (button == 3) popup_menu_tseg->popup(button, 0);
      else if (button == 1 && state&Gdk::CONTROL_MASK) split_trk();
      else if (button == 1) add_tpt_start();
    }

  }

  void move_wpt_start() {
    dPoint pt = wpts->get_point_crd(idx);
    mapview->rubber.add_cr_mark(pt, false, 3);
    mapview->rubber.add_cr_mark(iPoint(), true, 3);
    mapview->rubber.add_line(pt);
    mystate=1;
  }

  void move_wpt_finish(const iPoint p, const int button){
    if (button != 3) wpts->set_point_crd(idx, p);
    abort();
  }

  void move_tpt_start() {
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
  }

  void move_tpt_finish(const iPoint p, const int button){
    if (button != 3) trk->set_point_crd(idx, p);
    abort();
  }

  void add_tpt_start() {
    // find_segments returns only visible segments.
    // pts should contain 2 or 3 points: start and end of the segment
    // and optional previous point (not needed here)
    auto pts = trk->get_point_crd(idx);
    if (pts.size()<2) return;
    mapview->rubber.add_line(pts[0]);
    mapview->rubber.add_line(pts[1]);
    mapview->rubber.add_cr_mark(iPoint(), true, 3);
    mystate=3;
  }

  void add_tpt_finish(const iPoint p, const int button){
    if (button != 3) trk->add_point_crd(idx, p);
    abort();
  }

  void del_tpt()    { trk->del_point(idx); }
  void del_wpt()    { wpts->del_point(idx); }
  void split_trk()  { trk->split_trk(idx); }
  void del_trkseg() { trk->del_seg(idx); }
  void del_trk()    { mapview->panel_trks->remove(trk); }

  void add_trkseg_start() {
    pts.clear();
    mystate = 4;
    mapview->spanel.message("Add new segment to the track "
      "(left click: add point, ctrl-left: remove last point, right: finish, ctrl-right: abort)");
  }

  void add_trkseg_finish(const iPoint p, const int button,
                         const Gdk::ModifierType & state) {

    if (button == 3) {
       if (! (state&Gdk::CONTROL_MASK)) trk->add_segment_crd(pts);
       if (pts.size()==0) {
         mystate = 0;
         mapview->spanel.message(get_name());
       }
       pts.clear();
       mapview->rubber.clear();
       return;
    }

    // remove point
    if (button == 1 && state&Gdk::CONTROL_MASK){
      if (pts.size()>0) pts.pop_back();
      if (mapview->rubber.size()>0){
        mapview->rubber.pop();
      }
      if (mapview->rubber.size()>0){
        RubberSegment s = mapview->rubber.pop();
        s.flags |= RUBBFL_MOUSE_P2;
        s.p2=iPoint(0,0);
        mapview->rubber.add(s);
      }
      return;
    }

    // add point
    if (button == 1){
      pts.push_back(dPoint(p));

      if (mapview->rubber.size()>0){
        RubberSegment s = mapview->rubber.pop();
        s.flags &= ~RUBBFL_MOUSE;
        s.p2 = dPoint(p);
        mapview->rubber.add(s);
      }
      mapview->rubber.add_line(p);
      return;
    }
  }

};

#endif
