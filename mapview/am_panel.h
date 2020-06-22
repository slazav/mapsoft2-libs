#ifndef AM_PANEL_H
#define AM_PANEL_H

#include "action_mode.h"
#include "geo_data/geo_io.h"

class PanelDelSel : public ActionMode{
public:
    PanelDelSel (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Delete selected"; }
    std::string get_icon() { return "edit-delete"; }
    bool is_radio() { return false; }
    void activate() {
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->remove_selected(); break;
        case PAGE_TRKS: mapview->panel_trks->remove_selected(); break;
        case PAGE_MAPS: mapview->panel_maps->remove_selected(); break;
      }
    }
};

class PanelDelAll : public ActionMode{
public:
    PanelDelAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Delete all"; }
    std::string get_icon() { return "edit-delete"; }
    bool is_radio() { return false; }
    void activate() {
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->remove_all(); break;
        case PAGE_TRKS: mapview->panel_trks->remove_all(); break;
        case PAGE_MAPS: mapview->panel_maps->remove_all(); break;
      }
    }
};

class PanelMoveUp : public ActionMode{
public:
    PanelMoveUp (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move _up"; }
    std::string get_icon() { return "go-up"; }
    bool is_radio() { return false; }
    void activate() {
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->move(true,false); break;
        case PAGE_TRKS: mapview->panel_trks->move(true,false); break;
        case PAGE_MAPS: mapview->panel_maps->move(true,false); break;
      }
    }
};

class PanelMoveDown : public ActionMode{
public:
    PanelMoveDown (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move _down"; }
    std::string get_icon() { return "go-down"; }
    bool is_radio() { return false; }
    void activate() {
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->move(false,false); break;
        case PAGE_TRKS: mapview->panel_trks->move(false,false); break;
        case PAGE_MAPS: mapview->panel_maps->move(false,false); break;
      }
    }
};

class PanelMoveTop : public ActionMode{
public:
    PanelMoveTop (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move to _top"; }
    std::string get_icon() { return "go-top"; }
    bool is_radio() { return false; }
    void activate() {
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->move(true,true); break;
        case PAGE_TRKS: mapview->panel_trks->move(true,true); break;
        case PAGE_MAPS: mapview->panel_maps->move(true,true); break;
      }
    }
};

class PanelMoveBottom : public ActionMode{
public:
    PanelMoveBottom (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move to _bottom"; }
    std::string get_icon() { return "go-buttom"; }
    bool is_radio() { return false; }
    void activate() {
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->move(false,true); break;
        case PAGE_TRKS: mapview->panel_trks->move(false,true); break;
        case PAGE_MAPS: mapview->panel_maps->move(false,true); break;
      }
    }
};

class PanelHideAll : public ActionMode{
public:
    PanelHideAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Hide All"; }
    std::string get_icon() { return "list-remove"; }
    bool is_radio() { return false; }
    void activate() {
//      mapview->viewer.start_waiting();
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->show_all(false); break;
        case PAGE_TRKS: mapview->panel_trks->show_all(false); break;
        case PAGE_MAPS: mapview->panel_maps->show_all(false); break;
      }
//      mapview->viewer.stop_waiting();
    }
};

class PanelShowAll : public ActionMode{
public:
    PanelShowAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Show All"; }
    std::string get_icon() { return "list-add"; }
    bool is_radio() { return false; }
    void activate() {
//      mapview->viewer.start_waiting();
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->show_all(); break;
        case PAGE_TRKS: mapview->panel_trks->show_all(); break;
        case PAGE_MAPS: mapview->panel_maps->show_all(); break;
      }
//      mapview->viewer.stop_waiting();
    }
};

class PanelInvert : public ActionMode{
public:
    PanelInvert (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Invert visibility"; }
    std::string get_icon() { return "view-refresh"; }
    bool is_radio() { return false; }
    void activate() {
//      mapview->viewer.start_waiting();
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->invert_all(); break;
        case PAGE_TRKS: mapview->panel_trks->invert_all(); break;
        case PAGE_MAPS: mapview->panel_maps->invert_all(); break;
      }
//      mapview->viewer.stop_waiting();
    }
};

class PanelJoinVis : public ActionMode{
public:
    PanelJoinVis (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Join visible"; }
    bool is_radio() { return false; }
    void activate() {
//      mapview->viewer.start_waiting();
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->join(true); break;
        case PAGE_TRKS: mapview->panel_trks->join(true); break;
        case PAGE_MAPS: mapview->panel_maps->join(true); break;
      }
//      mapview->viewer.stop_waiting();
    }
};

class PanelJoinAll : public ActionMode{
public:
    PanelJoinAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Join all"; }
    bool is_radio() { return false; }
    void activate() {
//      mapview->viewer.start_waiting();
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: mapview->panel_wpts->join(false); break;
        case PAGE_TRKS: mapview->panel_trks->join(false); break;
        case PAGE_MAPS: mapview->panel_maps->join(false); break;
      }
//      mapview->viewer.stop_waiting();
    }
};

class PanelGoto : public ActionMode{
public:
    PanelGoto (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Goto"; }
    std::string get_icon() { return "go-jump"; }
    bool is_radio() { return false; }
    void activate() {
      dRect r;
      switch (mapview->panels->get_current_page()){
        case PAGE_WPTS: r=mapview->panel_wpts->get_range(); break;
        case PAGE_TRKS: r=mapview->panel_trks->get_range(); break;
        case PAGE_MAPS: r=mapview->panel_maps->get_range(); break;
      }
      if (r) mapview->goto_range(r, false);
    }
};


class PanelSave : public ActionMode, public Gtk::FileChooserDialog{
  std::string folder; // current folder
public:
    PanelSave (Mapview * mapview) :
           ActionMode(mapview),
           Gtk::FileChooserDialog(get_name(), Gtk::FILE_CHOOSER_ACTION_SAVE),
           folder("./"){
      auto fg = Gtk::FileFilter::create();
      fg->add_pattern("*.gu");
      fg->add_pattern("*.gpx");
      fg->add_pattern("*.kml");
      fg->add_pattern("*.kmz");
      fg->add_pattern("*.plt");
      fg->add_pattern("*.wpt");
      fg->add_pattern("*.map");
      fg->add_pattern("*.js");
      fg->add_pattern("*.zip");
      fg->set_name("Geodata");
      add_filter(fg);

      auto fa = Gtk::FileFilter::create();
      fa->add_pattern("*");
      fa->set_name("All files");
      add_filter(fa);

      add_button("_Cancel", GTK_RESPONSE_CANCEL);
      add_button("_Save",   GTK_RESPONSE_ACCEPT);

      set_transient_for(*mapview);
      set_do_overwrite_confirmation();
      set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }

    std::string get_name() { return "_Save selected"; }
    std::string get_icon() { return "document-save-as"; }
    bool is_radio() { return false; }

    void activate() {
      set_current_folder(folder);
      if (run() == GTK_RESPONSE_ACCEPT){

        std::string f = get_filename();
        GeoData w;
        switch (mapview->panels->get_current_page()){
          case PAGE_WPTS: mapview->panel_wpts->get_sel_data(w); break;
          case PAGE_TRKS: mapview->panel_trks->get_sel_data(w); break;
          case PAGE_MAPS: mapview->panel_maps->get_sel_data(w); break;
        }
        if (!w.empty()){
          try {write_geo(f, w);}
          catch (Err & e) {mapview->dlg_err.call(e);}
        }
      }
      hide();
    }
};

#endif
