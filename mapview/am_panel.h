#ifndef AM_PANEL_H
#define AM_PANEL_H

#include "am.h"
#include "geo_data/geo_io.h"

class PanelDelSel : public ActionMode{
public:
    PanelDelSel (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Delete selected"; }
    std::string get_icon() { return "edit-delete"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      if      (menu == "PopupWPTs") mapview->panel_wpts->remove_selected();
      else if (menu == "PopupTRKs") mapview->panel_trks->remove_selected();
      else if (menu == "PopupMAPs") mapview->panel_maps->remove_selected();
    }
};

class PanelDelAll : public ActionMode{
public:
    PanelDelAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Delete all"; }
    std::string get_icon() { return "edit-delete"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      if      (menu == "PopupWPTs") mapview->panel_wpts->remove_all();
      else if (menu == "PopupTRKs") mapview->panel_trks->remove_all();
      else if (menu == "PopupMAPs") mapview->panel_maps->remove_all();
    }
};

class PanelMoveUp : public ActionMode{
public:
    PanelMoveUp (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move _up"; }
    std::string get_icon() { return "go-up"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      if      (menu == "PopupWPTs") mapview->panel_wpts->move(true,false);
      else if (menu == "PopupTRKs") mapview->panel_trks->move(true,false);
      else if (menu == "PopupMAPs") mapview->panel_maps->move(true,false);
    }
};

class PanelMoveDown : public ActionMode{
public:
    PanelMoveDown (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move _down"; }
    std::string get_icon() { return "go-down"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      if      (menu == "PopupWPTs") mapview->panel_wpts->move(false,false);
      else if (menu == "PopupTRKs") mapview->panel_trks->move(false,false);
      else if (menu == "PopupMAPs") mapview->panel_maps->move(false,false);
    }
};

class PanelMoveTop : public ActionMode{
public:
    PanelMoveTop (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move to _top"; }
    std::string get_icon() { return "go-top"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      if      (menu == "PopupWPTs") mapview->panel_wpts->move(true,true);
      else if (menu == "PopupTRKs") mapview->panel_trks->move(true,true);
      else if (menu == "PopupMAPs") mapview->panel_maps->move(true,true);
    }
};

class PanelMoveBottom : public ActionMode{
public:
    PanelMoveBottom (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Move to _bottom"; }
    std::string get_icon() { return "go-buttom"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      if      (menu == "PopupWPTs") mapview->panel_wpts->move(false,true);
      else if (menu == "PopupTRKs") mapview->panel_trks->move(false,true);
      else if (menu == "PopupMAPs") mapview->panel_maps->move(false,true);
    }
};

class PanelHideAll : public ActionMode{
public:
    PanelHideAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Hide All"; }
    std::string get_icon() { return "list-remove"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
//      mapview->viewer.start_waiting();
      if      (menu == "PopupWPTs") mapview->panel_wpts->show_all(false);
      else if (menu == "PopupTRKs") mapview->panel_trks->show_all(false);
      else if (menu == "PopupMAPs") mapview->panel_maps->show_all(false);
//      mapview->viewer.stop_waiting();
    }
};

class PanelShowAll : public ActionMode{
public:
    PanelShowAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Show All"; }
    std::string get_icon() { return "list-add"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
//      mapview->viewer.start_waiting();
      if      (menu == "PopupWPTs") mapview->panel_wpts->show_all();
      else if (menu == "PopupTRKs") mapview->panel_trks->show_all();
      else if (menu == "PopupMAPs") mapview->panel_maps->show_all();
//      mapview->viewer.stop_waiting();
    }
};

class PanelInvert : public ActionMode{
public:
    PanelInvert (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Invert visibility"; }
    std::string get_icon() { return "view-refresh"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
//      mapview->viewer.start_waiting();
      if      (menu == "PopupWPTs") mapview->panel_wpts->invert_all();
      else if (menu == "PopupTRKs") mapview->panel_trks->invert_all();
      else if (menu == "PopupMAPs") mapview->panel_maps->invert_all();
//      mapview->viewer.stop_waiting();
    }
};

class PanelJoinVis : public ActionMode{
public:
    PanelJoinVis (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Join visible"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
//      mapview->viewer.start_waiting();
      if      (menu == "PopupWPTs") mapview->panel_wpts->join(true);
      else if (menu == "PopupTRKs") mapview->panel_trks->join(true);
      else if (menu == "PopupMAPs") mapview->panel_maps->join(true);
//      mapview->viewer.stop_waiting();
    }
};

class PanelJoinAll : public ActionMode{
public:
    PanelJoinAll (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Join all"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
//      mapview->viewer.start_waiting();
      if      (menu == "PopupWPTs") mapview->panel_wpts->join(false);
      else if (menu == "PopupTRKs") mapview->panel_trks->join(false);
      else if (menu == "PopupMAPs") mapview->panel_maps->join(false);
//      mapview->viewer.stop_waiting();
    }
};

class PanelGoto : public ActionMode{
public:
    PanelGoto (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "_Goto"; }
    std::string get_icon() { return "go-jump"; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      dRect r;
      if      (menu == "PopupWPTs") r=mapview->panel_wpts->get_range();
      else if (menu == "PopupTRKs") r=mapview->panel_trks->get_range();
      else if (menu == "PopupMAPs") r=mapview->panel_maps->get_range();
      if (r) mapview->viewer.set_range(r, false);
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

    void activate(const std::string & menu) {
      set_current_folder(folder);
      if (run() == GTK_RESPONSE_ACCEPT){

        std::string f = get_filename();
        GeoData w;
        if      (menu == "PopupWPTs") mapview->panel_wpts->get_sel_data(w);
        else if (menu == "PopupTRKs") mapview->panel_trks->get_sel_data(w);
        else if (menu == "PopupMAPs") mapview->panel_maps->get_sel_data(w);
        if (!w.empty()){
          try {write_geo(f, w);}
          catch (Err & e) {mapview->dlg_err.call(e);}
        }
      }
      hide();
    }
};

class PanelMapRef : public ActionMode{
public:
    PanelMapRef (Mapview * mapview) : ActionMode(mapview){ }
    std::string get_name() { return "Use map reference"; }
    std::string get_icon() { return ""; }
    bool is_radio() { return false; }
    void activate(const std::string & menu) {
      auto * ml = mapview->panel_maps->get_data();
      if (ml == NULL || ml->size()<1) return;
      mapview->set_cnv_map(ml->front(), true);
    }
};

#endif
