#include <cstdlib>
#include "mapview.h"
#include "geo_data/conv_geo.h"
#include "geo_mkref/geo_mkref.h"

Mapview::Mapview(const std::shared_ptr<Opt> & o) :
    haveref(false),
    changed(false),
    viewer(&gobj),
    rubber(&viewer),
    opts(o),
    panel_wpts(new PanelWpts),
    panel_trks(new PanelTrks),
    panel_maps(new PanelMaps),
    panel_mapdb(new PanelMapDB),
    amanager(this)
{

    /// window initialization
    signal_delete_event().connect_notify (
      sigc::bind(sigc::hide(sigc::mem_fun (this, &Mapview::exit)),false));
    set_default_size(640,480);

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
    opts->put("wpt_adj_brd", 0);

    gobj.set_opt(opts);
    // Add gobjs from panels.
    gobj.add(PAGE_WPTS, panel_wpts);
    gobj.add(PAGE_TRKS, panel_trks);
    gobj.add(PAGE_MAPS, panel_maps);

    /***************************************/
    /// Build panels (Notebook)
    panels = manage(new Gtk::Notebook());
    panels->set_name("panels");
    panels->set_scrollable(false);
    panels->set_size_request(100,-1);

    panels->append_page(*panel_wpts.get(), "WPT", "WPT");
    panels->append_page(*panel_trks.get(), "TRK", "TRK");
    panels->append_page(*panel_maps.get(), "MAP", "MAP");
    panels->append_page(*panel_mapdb.get(), "MAPDB", "MAPDB");


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

    show_all(); // show window

    // vmaps
    if (opts->exists("mapdb"))
      open_mapdb(opts->get("mapdb",""));
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

  // If there is no reference yet, try to make one.
  // Use data and options.
  // If reference can not be created do nothing.
  if (!haveref){
    try {
      GeoMap map = geo_mkref(data, *opts.get());
      set_cnv(std::shared_ptr<ConvMap>(new ConvMap(map)));
      scroll=true;
    }
    catch (Err & e) {}
  }

  if (scroll){
    if (data.trks.size()>0 && data.trks.begin()->size()>0){
       goto_point( (*data.trks.begin())[0] );
    }
    else if (data.wpts.size()>0 && data.wpts.begin()->size()>0){
       goto_point( (*data.wpts.begin())[0] );
    }
    else if (data.maps.size()>0 && data.maps.begin()->size()>0
         && data.maps.begin()->begin()->ref.size()>0){
       goto_point( data.maps.begin()->begin()->ref.begin()->second );
    }
  }

}

void
Mapview::clear_data() {
  panel_wpts->remove_all();
  panel_trks->remove_all();
  panel_maps->remove_all();
  panel_mapdb->close();
//  panel_misc.hide_all();
  haveref = false;
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

  GeoData data;
//  viewer.start_waiting();
  for (auto const & f:files){
    spanel.message("Load file: " + f);
    try { read_geo(f, data, *opts.get()); }
    catch(Err & e) { dlg_err.call(e); }
  }
  add_data(data, true);
//  viewer.stop_waiting();
}


void
Mapview::load_project(const std::string & file, bool force) {
  if (!force && get_changed()){
    dlg_chconf.call(
      sigc::bind(sigc::mem_fun(this, &Mapview::load_project), file, true));
    return;
  }
  GeoData data;
  try { read_geo(file, data, *opts.get()); }
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
    dlg_chconf.call(
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
  panel_mapdb->open(opts->get("mapdb",""));
  gobj.add(PAGE_VMAP, panel_mapdb->get_gobj());
  GeoMap r = panel_mapdb->get_gobj()->get_ref();
  if (!r.empty())
    set_cnv(std::shared_ptr<ConvMap>(new ConvMap(r)));

  std::cerr << "ref: " << r.ref.size() << "\n";
  for (auto & x : r.ref)
    std::cerr << x.first <<" -> " << x.second << "\n";
}

void
Mapview::close_mapdb(){
  gobj.del(panel_mapdb->get_gobj());
  panel_mapdb->close();
}


void
Mapview::exit(bool force) {

  // ask "do you want to exit?"
  if (!force && get_changed()){
    dlg_chconf.call(
      sigc::bind(sigc::mem_fun (this, &Mapview::exit), true));
    return;
  }

  save_acc();
  g_print ("Exiting...\n");
  hide();
}

/**********************************************************/

dRect
Mapview::get_range(bool wgs) const{
  dRect r = viewer.get_view_range();
  if (haveref && wgs) r = gobj.get_cnv()->frw_acc(r, 1);
  return r;
}

void
Mapview::set_cnv(const std::shared_ptr<ConvBase> & c){
  if (!c) return;
  dRect r = get_range();
  gobj.set_cnv(c);
  goto_range(r); // do nothing if there was no reference
  haveref=true;
}

void
Mapview::goto_point(dPoint p, bool wgs){
  if (!haveref) return;
  if (wgs) gobj.get_cnv()->bck(p);
  viewer.set_center(p);
}

void
Mapview::goto_range(const dRect & r, bool wgs){
  if (!haveref) return;
  // source and destination range in viewer coords
  dRect src_rng = viewer.get_view_range();
  dRect dst_rng = wgs? gobj.get_cnv()->bck_acc(r) : r;

  // calculate scaling factor (power of 2)
  double k = 1;
  while (src_rng.w > 2*k*dst_rng.w || src_rng.h > 2*k*dst_rng.h) k*=2;
  while (src_rng.w < k*dst_rng.w || src_rng.h < k*dst_rng.h) k/=2;

  // avoid rounding errors: first scaling, then moving
  viewer.rescale(k);
  viewer.set_center(dst_rng.cnt()*k);
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

