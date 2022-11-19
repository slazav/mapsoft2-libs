#ifndef AM_VMAP_H
#define AM_VMAP_H

/* Action modes for VMap menu */
#include "am.h"
//#include "geo_data/conv_geo.h"

/**********************************************************/
// Open MapBD project
class OpenVMap : public ActionMode, public Gtk::FileChooserDialog{
    std::string folder; // current folder
  public:
    OpenVMap (Mapview * mapview):
        ActionMode(mapview),
        Gtk::FileChooserDialog(get_name(), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
        folder("./") {

      add_button("_Cancel", GTK_RESPONSE_CANCEL);
      add_button("_Open",   GTK_RESPONSE_ACCEPT);

      set_transient_for(*mapview);
      set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
    }

    std::string get_name() override { return "Open VMap"; }
    std::string get_icon() override { return "open"; }
//    Gtk::AccelKey get_acckey() override { return Gtk::AccelKey("<control>f"); }
    bool is_radio() override { return false; }

    void activate(const std::string & menu) override {
      set_current_folder(folder);
      if (run() == GTK_RESPONSE_ACCEPT){
        folder = get_current_folder();
        mapview->open_vmap(folder);
      }
      hide();
    }
};

/**********************************************************/
// Close MapBD project

class CloseVMap : public ActionMode{
  public:
    CloseVMap (Mapview * mapview): ActionMode(mapview) { }

    std::string get_name() override { return "Close VMap"; }
    std::string get_icon() override { return "close"; }
//    Gtk::AccelKey get_acckey() override { return Gtk::AccelKey("<control>f"); }
    bool is_radio() override { return false; }

    void activate(const std::string & menu) override {
      mapview->close_vmap();
    }
};


#endif

