#ifndef AM_EDIT_H
#define AM_EDIT_H

#include "am.h"
#include "dlg_trk.h"
#include "dlg_wpt.h"
#include "dlg_tpt.h"

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

DlgTrk dlg_trk;
DlgWpt dlg_wpt;
DlgTpt dlg_tpt;
Opt o;

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
      Gtk::Action::create("EditData:wpt:edit", "Edit waypoint parameters", ""),
      sigc::mem_fun(this, &AMEditData::edit_wpt));
    actions->add(
      Gtk::Action::create("EditData:tpt:edit", "Edit trackpoint parameters", ""),
      sigc::mem_fun(this, &AMEditData::edit_tpt));

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
    actions->add(
      Gtk::Action::create("EditData:contseg", "Continue segment", ""),
      sigc::mem_fun(this, &AMEditData::cont_trkseg_start));
    actions->add(
      Gtk::Action::create("EditData:delpts", "Delete points", ""),
      sigc::mem_fun(this, &AMEditData::del_pts_start));
    actions->add(
      Gtk::Action::create("EditData:edittrk", "Edit track parameters", ""),
      sigc::mem_fun(this, &AMEditData::edit_trk));


    ui_manager->add_ui_from_string(
      "<ui>"
      "  <popup name='EditData:wpt'>"
      "    <menuitem action='EditData:wpt:edit'/>"
      "    <menuitem action='EditData:wpt:move'/>"
      "    <menuitem action='EditData:wpt:del'/>"
      "    <menuitem action='EditData:delpts'/>"
      "  </popup>"
      "  <popup name='EditData:tpt'>"
      "    <menuitem action='EditData:edittrk'/>"
      "    <menuitem action='EditData:tpt:edit'/>"
      "    <menuitem action='EditData:tpt:move'/>"
      "    <menuitem action='EditData:tpt:del'/>"
      "    <menuitem action='EditData:contseg'/>"
      "    <menuitem action='EditData:addseg'/>"
      "    <menuitem action='EditData:delseg'/>"
      "    <menuitem action='EditData:deltrk'/>"
      "    <menuitem action='EditData:delpts'/>"
      "  </popup>"
      "  <popup name='EditData:tseg'>"
      "    <menuitem action='EditData:edittrk'/>"
      "    <menuitem action='EditData:tseg:addpt'/>"
      "    <menuitem action='EditData:tseg:split'/>"
      "    <menuitem action='EditData:contseg'/>"
      "    <menuitem action='EditData:addseg'/>"
      "    <menuitem action='EditData:delseg'/>"
      "    <menuitem action='EditData:deltrk'/>"
      "    <menuitem action='EditData:delpts'/>"
      "  </popup>"
      "</ui>"
      );
    popup_menu_wpt  = (Gtk::Menu *)ui_manager->get_widget("/EditData:wpt");
    popup_menu_tpt  = (Gtk::Menu *)ui_manager->get_widget("/EditData:tpt");
    popup_menu_tseg = (Gtk::Menu *)ui_manager->get_widget("/EditData:tseg");


    dlg_trk.set_transient_for(*mapview);
    dlg_trk.signal_response().connect(
      sigc::mem_fun (this, &AMEditData::dlg_trk_res));
    dlg_trk.set_title(get_name());

    dlg_wpt.set_transient_for(*mapview);
    dlg_wpt.signal_response().connect(
      sigc::mem_fun (this, &AMEditData::dlg_wpt_res));
    dlg_wpt.set_title(get_name());
    dlg_wpt.signal_jump().connect(
          sigc::mem_fun (this, &AMEditData::on_jump));

    dlg_tpt.set_transient_for(*mapview);
    dlg_tpt.signal_response().connect(
      sigc::mem_fun (this, &AMEditData::dlg_tpt_res));
    dlg_tpt.set_title(get_name());
    dlg_tpt.signal_jump().connect(
          sigc::mem_fun (this, &AMEditData::on_jump));

  }

  std::string get_name() override { return "Edit tracks and waypoints"; }
  std::string get_desc() override { return "left click: move/add point; ctrl-left: delete point/split track; right: abort action/menu"; }

  void handle_click(const iPoint p, const int button,
                    const Gdk::ModifierType & state) override {
    switch (mystate){
      case 0: find_object(p, button, state); break;
      case 1: move_wpt_finish(p, button); break;
      case 2: move_tpt_finish(p, button); break;
      case 3: add_tpt_finish(p, button); break;
      case 4:
      case 5:
      case 6: add_trkseg_finish(p, button, state); break;
      case 7: del_pts_finish(p, button, state); break;
    }
  }

  void abort() override {
    mapview->rubber.clear();
    mystate = 0;
  }


private:

  /**************************/

  void find_object(const iPoint p, const int button,
                   const Gdk::ModifierType & state){

    wpts.reset();
    trk.reset();

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

  /**************************/

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

  /**************************/

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

  /**************************/

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

  /**************************/

  void add_trkseg_start() {
    pts.clear();
    mystate = 4;
    mapview->spanel.message("Add new segment to the track: "
      "left click - add point; ctrl-left - remove last point; right - finish; ctrl-right - abort");
  }

  void cont_trkseg_start() {
    pts.clear();
    mystate = 5;
    mapview->spanel.message("Add points to the track: "
      "left click - add point; ctrl-left - remove last point; right - finish; ctrl-right - abort");
    idx = trk->get_nearest_segment_end(idx);
    auto pts = trk->get_point_crd(idx);
    if (pts.size()<1) {abort(); return;}
    mapview->rubber.add_line(pts[0]);
  }

  void add_trkseg_finish(const iPoint p, const int button,
                         const Gdk::ModifierType & state) {

    if (button == 3) {
       if (! (state&Gdk::CONTROL_MASK)){
         if (mystate == 4) trk->add_segment_crd(pts);
         if (mystate == 5) trk->add_points_crd(idx, pts);
       }

       if (pts.size()==0 || mystate == 5) {
         mystate = 0;
         mapview->spanel.message(get_name());
       }
       pts.clear();
       mapview->rubber.clear();
       return;
    }

    // remove point
    if (button == 1 && state&Gdk::CONTROL_MASK){
      if (pts.size()==0) return;
      pts.pop_back();
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

  /**************************/
  void del_pts_start() {
    mystate = 7;
    pts.clear();
    std::string obj = trk? "track":"waypoint list";
    mapview->spanel.message(std::string("Delete points in the ") + obj +
      ": left click - draw rectangular area to delete; right click - abort");
  }

  void del_pts_finish(const iPoint p, const int button,
                      const Gdk::ModifierType & state) {
    // button 3 - cancel
    if (button == 3) {
      pts.clear();
      mapview->spanel.message(get_name());
      abort();
      return;
    }
    // ctrl + button 1 - draw rectangle
    if (button == 1){
      if (pts.size() == 0){
        pts.push_back(p);
        mapview->rubber.add_rect(p);
      }
      else {
        dRect rect(pts[0], p);
        mapview->rubber.clear();
        pts.clear();
        if (trk)  mapview->panel_trks->del_points(rect, trk);
        if (wpts) mapview->panel_wpts->del_points(rect, wpts);
      }
      return;
    }
  }

  /**************************/

  void dlg_trk_res(int r){
    if (r==Gtk::RESPONSE_OK){
      if (!trk) return;
      auto lk = trk->get_lock();
      dlg_trk.dlg2trk(&(trk->get_data()));
      trk->update_opt();
      trk->redraw_me();
    }
    dlg_trk.hide();
    abort();
  }

  void edit_trk() {
    if (!trk) return;
    auto lk = trk->get_lock();
    dlg_trk.trk2dlg(&(trk->get_data()));
    dlg_trk.set_info(&(trk->get_data()));
    dlg_trk.show_all();
  }

  /**************************/

  void dlg_tpt_res(int r){
    if (r==Gtk::RESPONSE_OK){
      if (!trk) return;
      auto lk = trk->get_lock();
      auto & t = trk->get_data();
      if (idx < t.size()) {
        dlg_tpt.dlg2tpt(t[idx]);
        trk->update_opt();
        trk->redraw_me();
      }
    }
    dlg_tpt.hide();
    abort();
  }

  void edit_tpt() {
    if (!trk) return;
    auto lk = trk->get_lock();
    auto & t = trk->get_data();
    if (idx < t.size())
      dlg_tpt.tpt2dlg(t[idx]);
    dlg_tpt.show_all();
  }

  /**************************/

  void dlg_wpt_res(int r){
    if (r==Gtk::RESPONSE_OK){
      if (!wpts) return;
      auto lk = wpts->get_lock();
      auto & w = wpts->get_data();
      if (idx < w.size()) {
        dlg_wpt.dlg2wpt(w[idx]);
        wpts->update_data();
        wpts->redraw_me();
      }
    }
    dlg_wpt.hide();
    abort();
  }

  void edit_wpt() {
    if (!wpts) return;
    auto lk = wpts->get_lock();
    auto & w = wpts->get_data();
    if (idx < w.size())
      dlg_wpt.wpt2dlg(w[idx]);
    dlg_wpt.show_all();
  }

  void on_jump(dPoint p){
    mapview->rubber.clear();
    mapview->viewer.get_cnv().bck(p);
    mapview->viewer.set_center(p,false);
    mapview->rubber.add_cr_mark(p, false);
    mapview->rubber.add_cr_mark(p, true);
  }

};

/********************************************************************/
class AMWptAdd : public ActionMode {
public:
  AMWptAdd (Mapview * mapview) : ActionMode(mapview) {
    dlg.set_transient_for(*mapview);
    dlg.signal_jump().connect(
        sigc::mem_fun (this, &AMWptAdd::on_jump));
    dlg.signal_response().connect(
      sigc::mem_fun (this, &AMWptAdd::on_result));
    dlg.set_title(get_name());
  }

  std::string get_name() override { return "Add waypoint"; }
  std::string get_desc() override { return "Select waypoint list to add points. "
    "If nothing is selected a new list will be created"; }

  void activate(const std::string & menu) override {
    abort();
    // Switch to waypoint panel, select first entry
    // if nothing is selected.
    mapview->open_panel(PAGE_WPTS);
    auto obj = mapview->panel_wpts->find_selected();
    if (!obj) {
      obj = mapview->panel_wpts->find_first();
      if (obj) mapview->panel_wpts->select(obj);
    }
  }

  void abort() override  {
    mapview->rubber.clear();
    dlg.hide();
  }

  void handle_click(const iPoint p, const int button,
                            const Gdk::ModifierType & state) override {
    if (button == 3) { abort(); return; }

    // Create new waypoint. Set coordinates,
    // Show dialog with point parameters
    wpt = GeoWpt();
    wpt.dPoint::operator=(p);
    mapview->viewer.get_cnv().frw(wpt);
    dlg.wpt2dlg(wpt);
    dlg.show_all();

    mapview->rubber.clear();
    mapview->rubber.add_cr_mark(p);
  }

private:
  DlgWpt dlg;
  GeoWpt wpt;

  void on_result(int r){
    if (r == Gtk::RESPONSE_OK){
      dlg.dlg2wpt(wpt);

      // get currently selected waypoint list
      auto obj = mapview->panel_wpts->find_selected();

      // if nothing is selected, create new waypoint list
      if (!obj){
        std::shared_ptr<GeoWptList> wptl(new GeoWptList);
        wptl->name = "NEW";
        obj = mapview->panel_wpts->add(wptl);
        mapview->panel_wpts->select(obj);
      }

      // add waypoint
      if (obj) obj->add_point(wpt);

    }
    abort();
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

