#ifndef AM_EDIT_H
#define AM_EDIT_H

#include "am.h"

class AMEditData : public ActionMode {
int mystate;
PanelTrks::ptr_t trk;
PanelWpts::ptr_t wpts;
size_t   idx;

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
      Gtk::Action::create("EditData:tseg:add", "Add trackpoint", ""),
      sigc::mem_fun(this, &AMEditData::add_tpt_start));
    actions->add(
      Gtk::Action::create("EditData:tseg:split", "Split track", ""),
      sigc::mem_fun(this, &AMEditData::split_trk));
    actions->add(
      Gtk::Action::create("EditData:tseg:del", "Delete segment", ""),
      sigc::mem_fun(this, &AMEditData::del_trkseg));


    ui_manager->add_ui_from_string(
      "<ui>"
      "  <popup name='EditData:wpt'>"
      "    <menuitem action='EditData:wpt:move'/>"
      "    <menuitem action='EditData:wpt:del'/>"
      "  </popup>"
      "  <popup name='EditData:tpt'>"
      "    <menuitem action='EditData:tpt:move'/>"
      "    <menuitem action='EditData:tpt:del'/>"
      "    <menuitem action='EditData:tseg:del'/>"
      "  </popup>"
      "  <popup name='EditData:tseg'>"
      "    <menuitem action='EditData:tseg:add'/>"
      "    <menuitem action='EditData:tseg:split'/>"
      "    <menuitem action='EditData:tseg:del'/>"
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
    if (button == 3 && mystate != 0) {
      abort();
      return;
    }
    switch (mystate){
      case 0: find_object(p, button, state); break;
      case 1: move_wpt_finish(p); break;
      case 2: move_tpt_finish(p); break;
      case 3: add_tpt_finish(p); break;
    }
  }

  void abort() override {
    mapview->rubber.clear();
    mystate = 0;
  }


private:
  void find_object(const iPoint p, const int button, const Gdk::ModifierType & state){

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

  void move_wpt_finish(iPoint p){
    wpts->set_point_crd(idx, p);
    mapview->rubber.clear();
    mystate=0;
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

  void move_tpt_finish(iPoint p){
    trk->set_point_crd(idx, p);
    mapview->rubber.clear();
    mystate=0;
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

  void add_tpt_finish(iPoint p){
    trk->add_point_crd(idx, p);
    mapview->rubber.clear();
    mystate=0;
  }

  void del_tpt()    { trk->del_point(idx); }
  void del_wpt()    { wpts->del_point(idx); }
  void split_trk()  { trk->split_trk(idx); }
  void del_trkseg() { trk->del_seg(idx); }

};


#endif
