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
    std::string get_name() { return name; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      mapview->add_data(d, false);
    }
};

#endif

