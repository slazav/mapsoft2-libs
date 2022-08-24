#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <gtkmm.h>
#include <map>
#include <string>
#include <memory>

#include "viewer/dthread_viewer.h"
#include "viewer/rubber.h"
#include "viewer/gobj_multi.h"
#include "geo_data/geo_io.h"
#include "geo_data/conv_geo.h"
#include "geom/rect.h"
#include "action_manager.h"
#include "dlg_confirm.h"
#include "dlg_err.h"

#include "panel_wpts.h"
#include "panel_trks.h"
#include "panel_maps.h"
#include "panel_mapdb.h"
#include "panel_status.h"
#include "srtm/srtm_surf.h"
#include "geo_render/gobj_srtm.h"

#define DATADIR    "mapsoft2"
#define ACCEL_FILE "mapsoft2.acc"
#define CSS_FILE   "mapsoft2.css"
#define MAPS_MENU_FILE "maps_menu.json"

// Same order as in mapview constructor where pages are added
// to the Gtk::Notebook:
#define PAGE_WPTS 0
#define PAGE_TRKS 1
#define PAGE_VMAP 2
#define PAGE_SRTM 3
#define PAGE_MAPS 4

class Mapview : public Gtk::ApplicationWindow {
public:
    /// Mapview components (order is important for constructing):
    GObjMulti     gobj;            // Main object
    DThreadViewer viewer;          // Viewer, gtk widget which shows main_gobj
    Rubber        rubber;          // Rubber lines
    Opt           opts;            // Command-line options

    SRTMSurf srtm;
    // signal is emitted when srtm is reconfigured
    sigc::signal<void> signal_srtm_conf_;
    sigc::signal<void> signal_srtm_conf() {return signal_srtm_conf_;}

    // Right panel, a Gtk::Notebook with separate
    // panels for waypoints, tracks, maps ...
    Gtk::Notebook * panels;
    std::shared_ptr<PanelTrks> panel_trks;
    std::shared_ptr<PanelWpts> panel_wpts;
    std::shared_ptr<PanelMaps> panel_maps;
    std::shared_ptr<PanelMapDB> panel_mapdb;
    std::shared_ptr<GObjSRTM>   obj_srtm;

    PanelStatus spanel; // status bar
    ActionManager amanager; // menus and action handling

    DlgConfirm dlg_confirm; // confirmation
    DlgErr     dlg_err;     // error dialog

private:
    // Project filename.
    std::string project_fname;

    // Changed flag:
    // has the project been changed since last saving/loading?
    bool changed;

    // Do we have a "temporary" projection?
    // If yes, it will be replaced by projection of the first loaded map
    bool tmpref;

    // Do we have a "temporary" view?
    // If yes, it will be scrolled to the first loaded map/data
    bool tmpview;

public:

    // Constructor.
    Mapview(const Opt & opts);

    /**********************************/

    // Get project changed status.
    bool get_changed() const {return changed;}

    // Set project changed status, modify window title.
    void set_changed(const bool c=true);

    // Get project filename.
    std::string get_project_fname() const {return project_fname;}

    // Set project filename, do set_changed().
    void set_project_fname(const std::string & f);

    /**********************************/

    // Add data from GeoData object. Scroll to data
    // position if `scroll==true`.
    void add_data(const GeoData & data, bool scroll=false);

    // Remove all data from the current project.
    void clear_data();

    // Fill GeoData object with all/visible data
    // from the current project.
    void get_data(GeoData & data, bool visible=true);

    /**********************************/

    // Add data from files.
    void add_files(const std::vector<std::string> & files);

    // Load new project (confirmation dialog will appear
    // if previous project has been changed and force==false).
    void load_project(const std::string & file, bool force=false);

    // Start new project (confirmation dialog will appear
    // if previous project has been changed and force==false).
    void new_project(bool force=false);

    // Open MapDB database. Rendering configuration is taken from options
    // (by default it is <dir>/raster.txt)
    void open_mapdb(const std::string & dir);

    // Close MapDB project
    void close_mapdb();

    // Open SRTM layer
    void open_srtm();

    // Close SRTM layer
    void close_srtm();

    // Exit Mapview application (confirmation dialog will appear
    // if previous project has been changed and force==false).
    void exit(bool force=false);

    /**********************************/

    // Set geo reference
    void set_cnv_map(const GeoMap & m, const bool force = false);

    // Open panel tab (PAGE_WPTS, PAGE_TRKS, etc.)
    void open_panel(const int n) {panels->set_current_page(n);}

    /**********************************/

    // Load accelerator map from file $HOME/.mapsoft2/mapsoft2.acc
    // If file does not exists then it is ignored silently.
    // In case of parse error a message is printed to stderr.
    // This file is rewrited every time when mapsoft2 program
    // is closed. This function is called in the Mapview constructor.
    void load_acc();

    // Save accelerator map to file $HOME/.mapsoft2/mapsoft2.acc
    // This function is called when Mapview is exiting.
    void save_acc();

    // Read CSS styles. Two files are checked: /usr/share/mapsoft2/mapsoft2.css
    // and $HOME/.mapsoft2/mapsoft2.css. If file does not exists then
    // it is ignored silently. In case of parse error a message is printed to
    // stderr. This function is called in the Mapview constructor.
    void load_css();

};


#endif /* MAPVIEW_H */
