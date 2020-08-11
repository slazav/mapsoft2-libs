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

    void activate() {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      double mlat = 85.0511288, mlon=180.0;
      double wr = 256;
      map.image = "https://{[abc]}.tile.openstreetmap.org/{z}/{x}/{y}.png";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "WEB";
      map.name = "OSM";
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

    void activate() {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      double mlat = 85.0511288, mlon=180.0;
      double wr = 256;
      map.image = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "WEB";
      map.name = "ESRI sat";
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

    void activate() {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      double mlat = 85.0511288, mlon=180.0;
      double wr = 256;
      map.image = "https://sat0{[1234]}.maps.yandex.net/tiles?l=sat&x={x}&y={y}&z={z}";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "WEB";
      map.name = "Yandex sat";
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

    void activate() {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      double mlat = 85.0511288, mlon=180.0;
      double wr = 256;
      map.image = "https://mt{[0123]}.google.com/vt/lyrs=s&x={x}&y={y}&z={z}";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "WEB";
      map.name = "Google sat";
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

    void activate() {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      double mlat = 85.0511288, mlon=180.0;
      double wr = 256;
      map.image = "https://ecn.t{[0123]}.tiles.virtualearth.net/tiles/a{q}.jpeg?g=8954";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "WEB";
      map.name = "Bing sat";
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

    void activate() {
      GeoData data_pre;
      GeoMapList maps;
      GeoMap map;
      double mlat = 85.0511288, mlon=180.0;
      double wr = 256;
      map.image = "https://ecn.t{[0123]}.tiles.virtualearth.net/tiles/r{q}?g=8954&lbl=l1&productSet=mmOS";
      map.is_tiled = true;
      map.tile_size = 256;
      map.tile_swapy = false;
      map.image_size = iPoint(wr,wr);
      map.proj = "WEB";
      map.name = "GB topo";
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

