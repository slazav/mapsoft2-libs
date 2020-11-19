#ifndef AM_MAPS_H
#define AM_MAPS_H

/* Action modes for Maps menu */
#include "am.h"
#include "geo_data/geo_io.h"

/**********************************************************/
// add map to the menu
class AddMap : public ActionMode{
    int state;
    std::string name;
    GeoData d;
  public:
    AddMap (Mapview * mapview, const GeoMapList & m) :
       ActionMode(mapview), state(0), name(m.name){
      d.maps.push_back(m);
    }
    std::string get_name() override { return name; }
    std::string get_icon() override { return "add"; }
//    Gtk::AccelKey get_acckey() override { return Gtk::AccelKey("<control>f"); }
    bool is_radio() override { return false; }

    void activate(const std::string & menu) override {
      mapview->add_data(d, false);
    }
};

#endif

