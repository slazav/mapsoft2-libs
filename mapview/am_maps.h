#ifndef AM_MAPS_H
#define AM_MAPS_H

/* Action modes for Maps menu */
#include "action_mode.h"
#include "geo_data/conv_geo.h"

/**********************************************************/
// mapy.cz map

class AddMapyCZ : public ActionMode{
    int state;
  public:
    AddMapyCZ (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add mapy.cz map"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate() {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      double mlat = 85.0511288, mlon=180.0;
      double wr = 256;
      map.image = "https://m{[1234]}.mapserver.mapy.cz/turist-m/{z}-{x}-{y}.png";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "WEB";
      map.name = "Mapy.cz";
      map.ref.emplace(dPoint( 0, 0),dPoint(-mlon, mlat));
      map.ref.emplace(dPoint(wr, 0),dPoint( mlon, mlat));
      map.ref.emplace(dPoint(wr,wr),dPoint( mlon,-mlat));
      map.ref.emplace(dPoint( 0,wr),dPoint(-mlon,-mlat));
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
      mapview->set_cnv(std::shared_ptr<ConvMap>(new ConvMap(map)));

    }
};

#endif
