#ifndef AM_MAPDB_H
#define AM_MAPDB_H

/* Action modes for MapDB menu */
#include "action_mode.h"
//#include "geo_data/conv_geo.h"

/**********************************************************/
// Open MapBD project
class OpenMapDB : public ActionMode, public Gtk::FileChooserDialog{
    std::string folder; // current folder
  public:
    OpenMapDB (Mapview * mapview):
        ActionMode(mapview), folder("./"),
        Gtk::FileChooserDialog(get_name(), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) {

      add_button("_Cancel", GTK_RESPONSE_CANCEL);
      add_button("_Open",   GTK_RESPONSE_ACCEPT);

      set_transient_for(*mapview);
      set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }

    std::string get_name() { return "Open MapDB"; }
    std::string get_icon() { return "open"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate() {
      set_current_folder(folder);
      if (run() == GTK_RESPONSE_ACCEPT)
        mapview->open_mapdb(get_filename());
      hide();
    }
};

/**********************************************************/
// Close MapBD project

class CloseMapDB : public ActionMode{
  public:
    CloseMapDB (Mapview * mapview): ActionMode(mapview) { }

    std::string get_name() { return "Close MapDB"; }
    std::string get_icon() { return "close"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate() {
      mapview->close_mapdb();
    }
};


#endif

