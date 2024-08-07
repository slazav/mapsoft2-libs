#include "action_manager.h"
#include "mapview.h"

/**** Images *****/

//#include "../images/gps_download.h"
//#include "../images/gps_upload.h"

/**** ActionModes *****/

#include "am.h"
#include "am_edit.h"
#include "am_file.h"
#include "am_view.h"
#include "am_trks.h"
#include "am_maps.h"
#include "am_vmap.h"
#include "am_panel.h"
#include "am_srtm.h"

/*********/

// add action to the menu
#define ADD_ACT(name, group) AddAction(new name(mapview),\
  std::string("Mode") + #name, group);

// add icon
#define ADD_ICON(size, name) \
    icon_factory->add(\
      Gtk::StockID(#name),\
      Gtk::IconSet(\
        Gdk::Pixbuf::create_from_data (idata_##name, Gdk::COLORSPACE_RGB,\
        true /*alpha*/, 8 /*bps*/, size /*w*/, size /*h*/, (size)*4 /*rowstride*/))\
    );

ActionManager::ActionManager (Mapview * mapview_):
  mapview(mapview_), current_mode(0) {

    /***************************************/
    // Add my icons.
/*
    Glib::RefPtr<Gtk::IconFactory> icon_factory = Gtk::IconFactory::create();
    icon_factory->add_default();

    ADD_ICON(16, gps_download);
    ADD_ICON(16, gps_upload);
*/
    /***************************************/
    /// Menues
    actions = Gtk::ActionGroup::create();
    ui_manager = Gtk::UIManager::create();
    ui_manager->insert_action_group(actions);
    mapview->add_accel_group(ui_manager->get_accel_group());

    // Default mode in the begining
    modes.push_back(std::shared_ptr<ActionMode>(new ActionModeNone(mapview)));

    /***************************************/
    // Add actions to menus

    /* Main menu */

    ADD_ACT(New,             "File")
    ADD_ACT(LoadFile,        "File")
    AddSep("File");
    ADD_ACT(Quit,            "File")

    ADD_ACT(FullScreen,      "View")
    ADD_ACT(HidePanels,      "View")
    ADD_ACT(ShowPt,          "View")
    ADD_ACT(AMTrkOpt,        "View")

    ADD_ACT(AMWptAdd,        "Edit")
    ADD_ACT(AMTrkAdd,        "Edit")
    ADD_ACT(AMTrkDel,        "Edit")
    ADD_ACT(AMEditData,      "Edit")

    AddMaps("Maps", std::string("/usr/share/") + DATADIR + "/" + MAPS_MENU_FILE);

    if (getenv("HOME")) {
      AddSep("Maps");
      AddMaps("Maps", std::string(getenv("HOME")) + "/." + DATADIR + "/" + MAPS_MENU_FILE);
    }

    ADD_ACT(OpenVMap,        "VMap")
    ADD_ACT(CloseVMap,       "VMap")

    ADD_ACT(AMShowSRTM,      "SRTM")
    ADD_ACT(AMSrtmPano,      "SRTM")
    ADD_ACT(AMSrtmOpts,      "SRTM")

    // Wpt panel menu
    ADD_ACT(PanelGoto,        "PopupWPTs")
    ADD_ACT(PanelSave,        "PopupWPTs")
    AddSep("PopupWPTs");
    ADD_ACT(PanelShowAll,     "PopupWPTs")
    ADD_ACT(PanelHideAll,     "PopupWPTs")
    ADD_ACT(PanelInvert,      "PopupWPTs")
    AddSep("PopupWPTs");
    ADD_ACT(PanelMoveTop,     "PopupWPTs")
    ADD_ACT(PanelMoveUp,      "PopupWPTs")
    ADD_ACT(PanelMoveDown,    "PopupWPTs")
    ADD_ACT(PanelMoveBottom,  "PopupWPTs")
    AddSep("PopupWPTs");
    ADD_ACT(PanelJoinVis,     "PopupWPTs")
    ADD_ACT(PanelJoinAll,     "PopupWPTs")
    AddSep("PopupWPTs");
    ADD_ACT(PanelDelSel,      "PopupWPTs")
    ADD_ACT(PanelDelAll,      "PopupWPTs")

    // Trk panel menu
    ADD_ACT(PanelGoto,        "PopupTRKs")
    ADD_ACT(PanelSave,        "PopupTRKs")
    AddSep("PopupTRKs");
    ADD_ACT(PanelShowAll,     "PopupTRKs")
    ADD_ACT(PanelHideAll,     "PopupTRKs")
    ADD_ACT(PanelInvert,      "PopupTRKs")
    AddSep("PopupTRKs");
    ADD_ACT(PanelMoveTop,     "PopupTRKs")
    ADD_ACT(PanelMoveUp,      "PopupTRKs")
    ADD_ACT(PanelMoveDown,    "PopupTRKs")
    ADD_ACT(PanelMoveBottom,  "PopupTRKs")
    AddSep("PopupTRKs");
    ADD_ACT(PanelJoinVis,     "PopupTRKs")
    ADD_ACT(PanelJoinAll,     "PopupTRKs")
    AddSep("PopupTRKs");
    ADD_ACT(PanelDelSel,      "PopupTRKs")
    ADD_ACT(PanelDelAll,      "PopupTRKs")
//    AddSep("PopupTRKs");
//    ADD_ACT(DrawOpt,          "PopupTRKs")

    // Map panel menu
    ADD_ACT(PanelMapRef,      "PopupMAPs")
    AddSep("PopupMAPs");
    ADD_ACT(PanelGoto,        "PopupMAPs")
    ADD_ACT(PanelSave,        "PopupMAPs")
    AddSep("PopupMAPs");
    ADD_ACT(PanelShowAll,     "PopupMAPs")
    ADD_ACT(PanelHideAll,     "PopupMAPs")
    ADD_ACT(PanelInvert,      "PopupMAPs")
    AddSep("PopupMAPs");
    ADD_ACT(PanelMoveTop,     "PopupMAPs")
    ADD_ACT(PanelMoveUp,      "PopupMAPs")
    ADD_ACT(PanelMoveDown,    "PopupMAPs")
    ADD_ACT(PanelMoveBottom,  "PopupMAPs")
    AddSep("PopupMAPs");
    ADD_ACT(PanelJoinVis,     "PopupMAPs")
    ADD_ACT(PanelJoinAll,     "PopupMAPs")
    AddSep("PopupMAPs");
    ADD_ACT(PanelDelSel,      "PopupMAPs")
    ADD_ACT(PanelDelAll,      "PopupMAPs")


    /***************************************/

    /* Cleate menus */
    actions->add(Gtk::Action::create("MenuFile",  "_File"));
    actions->add(Gtk::Action::create("MenuView",  "_View"));
    actions->add(Gtk::Action::create("MenuEdit",  "_Edit"));
    actions->add(Gtk::Action::create("MenuMaps",  "_Maps"));
    actions->add(Gtk::Action::create("MenuVMap",  "VMa_p"));
    actions->add(Gtk::Action::create("MenuSRTM",  "_SRTM"));

    mapview->panel_wpts->popup_menu = (Gtk::Menu *)ui_manager->get_widget("/PopupWPTs");
    mapview->panel_trks->popup_menu = (Gtk::Menu *)ui_manager->get_widget("/PopupTRKs");
    mapview->panel_maps->popup_menu = (Gtk::Menu *)ui_manager->get_widget("/PopupMAPs");

    /// viewer mouse click -> action manager click
    mapview->viewer.signal_click().connect (
      sigc::mem_fun (this, &ActionManager::click));
}


void
ActionManager::AddSep(const std::string & menu){
  if (menu.substr(0,5) == "Popup"){
    ui_manager->add_ui_from_string(
      "<ui>"
      "  <popup name='" + menu + "'>"
      "    <separator/>"
      "  </popup>"
      "</ui>"
    );
  }
  else{
    ui_manager->add_ui_from_string(
      "<ui>"
      "  <menubar name='MenuBar'>"
      "    <menu action='Menu" + menu + "'>"
      "      <separator/>"
      "    </menu>"
      "  </menubar>"
      "</ui>"
    );
  }
}

void
ActionManager::AddAction(ActionMode *action,
                         const std::string & id, const std::string & menu){
  modes.push_back(std::shared_ptr<ActionMode>(action));
  int m = modes.size()-1;

  if (!actions->get_action(id)){

    auto name = action->get_name();
    auto desc = action->get_desc();
    auto acckey = action->get_acckey();

    auto gtk_action = Gtk::Action::create(menu + ":" + id, name, desc);
    auto slot = sigc::bind (sigc::mem_fun(this, &ActionManager::set_mode), m, menu);
    gtk_action->set_icon_name(action->get_icon());

    // I do not know how to create empty editable AccelKey. So i use
    // these stupid ifs...
    if (acckey.is_null())
      actions->add( gtk_action, slot);
    else
      actions->add( gtk_action, acckey, slot);
  }

  if (menu.substr(0,5) == "Popup"){
    ui_manager->add_ui_from_string(
      "<ui>"
      "  <popup name='" + menu + "'>"
      "    <menuitem action='" + menu + ":" + id + "'/>"
      "  </popup>"
      "</ui>"
    );
  }
  else{
    ui_manager->add_ui_from_string(
      "<ui>"
      "  <menubar name='MenuBar'>"
      "    <menu action='Menu" + menu + "'>"
      "      <menuitem action='" + menu + ":" + id + "'/>"
      "    </menu>"
      "  </menubar>"
      "</ui>"
    );
  }
}

void
ActionManager::AddMaps(const std::string & menu, const std::string & file){
  GeoData d;
  try { read_geo(file, d, Opt()); }
  catch (Err & e) {}

  int n = 0;
  for (auto const & m:d.maps){
    AddAction(new AddMap(mapview, m),\
    std::string("ModeMap:") + file + ":" + type_to_str(n), menu);
    n++;
  }
}

void
ActionManager::clear_state (){
  mapview->rubber.clear();
  modes[current_mode]->abort();
}

void
ActionManager::set_mode (int mode, const std::string & menu){
  if (modes[mode]->is_radio()){
    clear_state();
    auto n = modes[mode]->get_name();
    auto d = modes[mode]->get_desc();
    mapview->spanel.message(n + (d==""?"": ": " + d ));
    current_mode = mode;
  }
  modes[mode]->activate(menu);
}
