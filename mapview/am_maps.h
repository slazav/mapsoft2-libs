#ifndef AM_MAPS_H
#define AM_MAPS_H

/* Action modes for Maps menu */
#include "action_mode.h"
#include "geo_data/conv_geo.h"
#include "geo_mkref/geo_mkref.h"

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

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map = geo_mkref_web();
      map.image = "https://m{[1234]}.mapserver.mapy.cz/turist-m/{z}-{x}-{y}.png";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.name = "Mapy.cz";
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};


/**********************************************************/
// OSM map
// api usage policy: https://operations.osmfoundation.org/policies/api/
class AddOSM : public ActionMode{
    int state;
  public:
    AddOSM (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add OSM map"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map = geo_mkref_web();
      map.image = "https://{[abc]}.tile.openstreetmap.org/{z}/{x}/{y}.png";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.name = "OSM";
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};

/**********************************************************/
// ESRI sat

class AddESRI : public ActionMode{
    int state;
  public:
    AddESRI (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add ESRI sat"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map = geo_mkref_web();
      map.image = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.name = "ESRI sat";
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};

/**********************************************************/
// Yandex sat

class AddYandexSat : public ActionMode{
    int state;
  public:
    AddYandexSat (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add Yandex sat"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      // max latitude is set to have square map. cnv(mlon,mlat) = (1,1)*20037508.342789244
      double mlat = 85.0840591, mlon=180.0;
      double wr = 256;
      map.image = "https://sat0{[1234]}.maps.yandex.net/tiles?l=sat&x={x}&y={y}&z={z}";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "EWEB";
      map.name = "Yandex sat";
      map.ref.emplace(dPoint( 0, 0),dPoint(-mlon, mlat));
      map.ref.emplace(dPoint(wr, 0),dPoint( mlon, mlat));
      map.ref.emplace(dPoint(wr,wr),dPoint( mlon,-mlat));
      map.ref.emplace(dPoint( 0,wr),dPoint(-mlon,-mlat));
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};

/**********************************************************/
// Google sat

class AddGoogleSat : public ActionMode{
    int state;
  public:
    AddGoogleSat (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add Google sat"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map = geo_mkref_web();
      map.image = "https://mt{[0123]}.google.com/vt/lyrs=s&x={x}&y={y}&z={z}";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.name = "Google sat";
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};

/**********************************************************/
// Bing sat

class AddBingSat : public ActionMode{
    int state;
  public:
    AddBingSat (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add Bing sat"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map = geo_mkref_web();
      map.image = "https://ecn.t{[0123]}.tiles.virtualearth.net/tiles/a{q}.jpeg?g=8954";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.name = "Bing sat";
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};


/**********************************************************/
// GB topo
class AddGBmap : public ActionMode{
    int state;
  public:
    AddGBmap (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add GB topo map"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map = geo_mkref_web();
      map.image = "https://ecn.t{[0123]}.tiles.virtualearth.net/tiles/r{q}?g=8954&lbl=l1&productSet=mmOS";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.name = "GB topo";
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};

/**********************************************************/
// map_podm
class AddPodmMap : public ActionMode{
    int state;
  public:
    AddPodmMap (Mapview * mapview) : ActionMode(mapview), state(0){ }
    std::string get_name() { return "Add slazav map"; }
    std::string get_icon() { return "add"; }
//    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>f"); }
    bool is_radio() { return false; }

    void activate(const std::string & menu) {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map = geo_mkref_web();
      map.image = "https://{[abc]}.tiles.nakarte.me/map_podm/{z}/{x}/{y}";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_maxz = 14;
      map.tile_swapy = true;
      map.name = "slazav";
      maps.push_back(map);
      data_pre.maps.push_back(maps);
      mapview->add_data(data_pre, false);
    }
};

#endif

