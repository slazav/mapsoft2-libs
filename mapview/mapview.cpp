#include <cstdlib>
#include "mapview.h"
#include "geo_data/conv_geo.h"
#include "geo_mkref/geo_mkref.h"

Mapview::Mapview(const Opt & o) :
    tmpref(true),
    tmpview(true),
    changed(false),
    viewer(&gobj),
    rubber(&viewer),
    opts(o),
    dlg_confirm("Data have been changed. Continue?"),
    panel_wpts(new PanelWpts),
    panel_trks(new PanelTrks),
    panel_maps(new PanelMaps),
    panel_mapdb(new PanelMapDB),
    obj_srtm(new GObjSRTM(&srtm, o)),
    amanager(this)
{

    /// window initialization
    signal_delete_event().connect_notify (
      sigc::bind(sigc::hide(sigc::mem_fun (this, &Mapview::exit)),false));
    set_default_size(640,480);

    /// dialogs
    dlg_err.set_transient_for(*this);
    dlg_confirm.set_transient_for(*this);

    /// global keypress event -- send all keys to the viewer first:
    signal_key_press_event().connect (
      sigc::mem_fun (&viewer, &DThreadViewer::on_key_press));

    // mapview wants to know about data change only to set a star in
    // the window title.
    panel_wpts->signal_data_changed().connect(
      sigc::bind(sigc::mem_fun(this, &Mapview::set_changed), true));
    panel_trks->signal_data_changed().connect(
      sigc::bind(sigc::mem_fun(this, &Mapview::set_changed), true));
    panel_maps->signal_data_changed().connect(
      sigc::bind(sigc::mem_fun(this, &Mapview::set_changed), true));

    /// events from viewer
    viewer.signal_busy().connect(
      sigc::mem_fun (&spanel, &PanelStatus::set_busy));
    viewer.signal_idle().connect(
      sigc::mem_fun (&spanel, &PanelStatus::set_idle));

    // in the viewer we don't want to fit waypoints inside tiles
    opts.put("wpt_adj_brd", 0);

    gobj.set_opt(opts);
    // Add gobjs from panels.
    gobj.add(PAGE_WPTS, panel_wpts);
    gobj.add(PAGE_TRKS, panel_trks);
    gobj.add(PAGE_SRTM, obj_srtm);  gobj.set_visibility(obj_srtm, false);
    gobj.add(PAGE_MAPS, panel_maps);

    /***************************************/
    /// Build panels (Notebook)
    panels = manage(new Gtk::Notebook());
    panels->set_name("panels");
    panels->set_scrollable(false);
    panels->set_size_request(100,-1);

    // order is important: page number should be same as layer depth
    // (PAGE_* constants)
    panels->append_page(*panel_wpts.get(), "WPT", "WPT");
    panels->append_page(*panel_trks.get(), "TRK", "TRK");
    panels->append_page(*panel_mapdb.get(), "MAPDB", "MAPDB");
    panels->append_page(*panel_maps.get(), "MAP", "MAP");


    /// Build main paned: Viewer + Panels
    Gtk::HPaned * paned = manage(new Gtk::HPaned);
    paned->pack1(viewer, Gtk::EXPAND | Gtk::FILL);
    paned->pack2(*panels, Gtk::FILL);

    /// Build main vbox: menu + main pand + statusbar
    Gtk::VBox * vbox = manage(new Gtk::VBox);
    vbox->pack_start(*amanager.get_main_menu(), false, true, 0);
    vbox->pack_start(*paned, true, true, 0);

    vbox->set_spacing(0);
    vbox->pack_start(spanel, false, false, 0);
    add (*vbox);

    // set widget names (to be used in CSS)
    vbox->set_name("main_vbox");
    paned->set_name("main_paned");
    panels->set_name("panels");
    viewer.set_name("viewer");
    spanel.set_name("status");

    viewer.set_bgcolor(0xF5DEB3 /*wheat*/);

    project_fname="";
//    spanel.message("Welcome to mapsoft2 viewer!");

    load_css(); // Load CSS styles
    load_acc(); // Load accelerator map

    // set default map projection -- web mercator
    {
      viewer.set_cnv(std::shared_ptr<ConvMap>(new ConvMap(geo_mkref_web())), false);
      // viewer does not know window size. We requested 640x480, just scale 2 times
      viewer.set_origin(dPoint(128,64));
      viewer.rescale(4.0, iPoint());
      tmpref = true;
    }

    show_all(); // show window

    // vmaps
    if (opts.exists("mapdb"))
      open_mapdb(opts.get("mapdb",""));
    else
      panel_mapdb->hide();

}

/**********************************************************/

void
Mapview::set_changed(const bool c){
//  changed=c;
//  set_title(std::string("ms2view: ")
//    + (c?"*":"") + project_fname);
}

void
Mapview::set_project_fname(const std::string & f){
  project_fname=f;
  set_changed(false);
}

/**********************************************************/

void
Mapview::add_data(const GeoData & data, bool scroll) {

  for (auto const & m:data.maps)
    panel_maps->add(
      std::shared_ptr<GeoMapList>(new GeoMapList(m)));

  for (auto const & w:data.wpts)
    panel_wpts->add(
      std::shared_ptr<GeoWptList>(new GeoWptList(w)));

  for (auto const & t:data.trks)
    panel_trks->add(
      std::shared_ptr<GeoTrk>(new GeoTrk(t)));

  set_changed();

  // Set projection + zoom.
  // Ignore all errors

  try {
    // set projection of the first map
    if (data.maps.size()>0 && data.maps.front().size()>0)
      set_cnv_map(data.maps.front().front());

    // scroll and zoom to wpts+trks data
    if (scroll){
      dRect box = expand(data.bbox_trks(), data.bbox_wpts());
      if (!box.is_zsize()) viewer.set_range(box, true);
      tmpview=false;
    }
  }
  catch (Err & e) {}

}

void
Mapview::clear_data() {
  panel_wpts->remove_all();
  panel_trks->remove_all();
  panel_maps->remove_all();
  close_mapdb();
  close_srtm();
  tmpref = true;
}

void
Mapview::get_data(GeoData & data, bool visible){
  panel_maps->get_data(data, visible);
  panel_trks->get_data(data, visible);
  panel_wpts->get_data(data, visible);
}

/**********************************************************/

void
Mapview::add_files(const std::vector<std::string> & files) {
  if (files.size()==0) return;

  try {
    GeoData data;
//  viewer.start_waiting();
    for (auto const & f:files){
      spanel.message("Load file: " + f);
      read_geo(f, data, opts);
    }
    add_data(data, true);
//  viewer.stop_waiting();
  }
  catch(Err & e) { dlg_err.call(e); }
}


void
Mapview::load_project(const std::string & file, bool force) {
  if (!force && get_changed()){
    dlg_confirm.call(
      sigc::bind(sigc::mem_fun(this, &Mapview::load_project), file, true));
    return;
  }
  GeoData data;
  try { read_geo(file, data, opts); }
  catch(Err & e) { dlg_err.call(e); }
  spanel.message("Open new project: " + file);

//  viewer.start_waiting();
  clear_data();
  add_data(data, true);
//  viewer.stop_waiting();
//  if (file_ext_check(file, ".xml")) project_fname = file;
  set_changed(false);
}

void
Mapview::new_project(bool force) {
  if (!force && get_changed()){
    dlg_confirm.call(
      sigc::bind(sigc::mem_fun (this, &Mapview::new_project), true));
    return;
  }
  project_fname = "";
  spanel.message("New project");
//  viewer.start_waiting();
  clear_data();
//  viewer.stop_waiting();
  set_changed(false);
}

void
Mapview::open_mapdb(const std::string & dir){
  close_mapdb();
  try {
    panel_mapdb->open(dir);
    gobj.add(PAGE_VMAP, panel_mapdb->get_gobj());
    GeoMap r = panel_mapdb->get_gobj()->get_ref();
    if (!r.empty()){
      viewer.set_cnv(std::shared_ptr<ConvMap>(new ConvMap(r)), false);
      tmpref = false;
    }
  }
  catch (Err & e) { dlg_err.call(e); }
}

void
Mapview::close_mapdb(){
  try {
    gobj.del(panel_mapdb->get_gobj());
    panel_mapdb->close();
  }
  catch (Err & e) { dlg_err.call(e); }
}

void
Mapview::open_srtm(){
  gobj.set_visibility(obj_srtm, true);
}

void
Mapview::close_srtm(){
  gobj.set_visibility(obj_srtm, false);
}

void
Mapview::exit(bool force) {

  // ask "do you want to exit?"
  if (!force && get_changed()){
    dlg_confirm.call(
      sigc::bind(sigc::mem_fun (this, &Mapview::exit), true));
    return;
  }

  save_acc();
  g_print ("Exiting...\n");
  hide();
}

/**********************************************************/
#include <sys/stat.h>

// Load accelerator map
void
Mapview::load_acc(){
  std::string home = getenv("HOME");
  std::string acc_loc = home + "/." + DATADIR + "/" + ACCEL_FILE;
  Gtk::AccelMap::load(acc_loc);
}

// Save accelerator map
void
Mapview::save_acc(){
  std::string home = getenv("HOME");
  std::string acc_loc = home + "/." + DATADIR + "/" + ACCEL_FILE;
  Gtk::AccelMap::save(acc_loc);
}

// CSS styles
void
Mapview::load_css(){
  std::string home = getenv("HOME");
  std::string css_glo = std::string() + "/usr/share/" + DATADIR + "/" + CSS_FILE;
  std::string css_loc = home + "/." + DATADIR + "/" + CSS_FILE;

  // load css files
  auto css_provider = Gtk::CssProvider::create();
  if (!css_provider) throw Err() << "Mapview: can't get Gtk::CssProvider";

  auto style_context = Gtk::StyleContext::create();
  if (!style_context) throw Err() << "Mapview: can't get Gtk::StyleContext";

  struct stat st_buf;
  try{
    if (stat(css_glo.c_str(), &st_buf) == 0 &&
      css_provider->load_from_path(css_glo)){
      auto screen = get_screen();
      if (!screen) throw Err() << "Mapview: can't get screen";
      style_context->add_provider_for_screen(
         screen, css_provider,
         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    if (stat(css_loc.c_str(), &st_buf) == 0 &&
      css_provider->load_from_path(css_loc)){
      auto screen = get_screen();
      if (!screen) throw Err() << "Mapview: can't get screen";
      style_context->add_provider_for_screen(
         screen, css_provider,
         GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
  }

  catch (Glib::Error e){
    std::cerr << "Mapview: Reading CSS files: " << e.what() << "\n";
  }
}

