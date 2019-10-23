#ifndef MAPVIEW_PANEL_MAPDB_H
#define MAPVIEW_PANEL_MAPDB_H

#include "mapdb/gobj_mapdb.h"

/**********************************************************/
/* Control panel for MapDB.
*/

class MapDBGrRecord : public Gtk::TreeModelColumnRecord {
public:
    Gtk::TreeModelColumn<bool> checked;
    Gtk::TreeModelColumn<std::string> name;

    MapDBGrRecord() {
      add(checked); add(name);
    }
};

class MapDBStRecord : public Gtk::TreeModelColumnRecord {
public:
    Gtk::TreeModelColumn<bool> checked;
    Gtk::TreeModelColumn<std::string> name;
    Gtk::TreeModelColumn<std::shared_ptr<GObj> > obj;

    MapDBStRecord() {
      add(checked); add(name); add(obj);
    }
};

/**********************************************************/

class PanelMapDB : public Gtk::Notebook, public GObjMapDB {
  Gtk::TreeView *tv_gr, *tv_st; // list of groups and steps
  Glib::RefPtr<Gtk::ListStore> store_gr, store_st;
  MapDBGrRecord cols_gr;
  MapDBStRecord cols_st;

public:
  PanelMapDB(const std::string & mapdir):
      GObjMapDB(mapdir) {

    /*******************************/
    // Setup group view

    tv_gr = manage(new Gtk::TreeView);
    store_gr = Gtk::ListStore::create(cols_gr);
    tv_gr->set_name("MapDB groups");
    tv_gr->set_model(store_gr);
    tv_gr->append_column_editable("V", cols_gr.checked);
    tv_gr->append_column("V", cols_gr.name);


    tv_gr->set_enable_search(false);
    tv_gr->set_headers_visible(false);
    tv_gr->set_reorderable(false);

    // pack into scroll window
    Gtk::ScrolledWindow * scr_gr = manage(new Gtk::ScrolledWindow);
    scr_gr->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scr_gr->add(*tv_gr);

    /*******************************/
    // Setup step view
    tv_st = manage(new Gtk::TreeView);
    store_st = Gtk::ListStore::create(cols_st);
    tv_st->set_name("MapDB groups");
    tv_st->set_model(store_st);
    tv_st->append_column_editable("V", cols_st.checked);
    tv_st->append_column("V", cols_st.name);

    tv_st->set_enable_search(false);
    tv_st->set_headers_visible(false);
    tv_st->set_reorderable(false);

    // pack into scroll window
    Gtk::ScrolledWindow * scr_st = manage(new Gtk::ScrolledWindow);
    scr_st->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scr_st->add(*tv_st);

    /*******************************/
    // Setup the main widget (Gtk::Notebook)
    set_name("MapDB viewes");
    set_scrollable(false);
    //set_size_request(100,-1);
    append_page(*scr_gr, "Groups", "Groups");
    append_page(*scr_st, "Steps", "Steps");

    /*******************************/
    // add data

    for (auto const & st:GObjMapDB::get_data()){
      auto it = store_st->append();
      Gtk::TreeModel::Row row = *it;
      row[cols_st.checked] = get_visibility(st);
      row[cols_st.name]    = ((DrawingStep*)st.get())->get_name();
      row[cols_st.obj]     = st;
    }

    for (auto const & gr:get_groups()){
      auto it = store_gr->append();
      Gtk::TreeModel::Row row = *it;
      row[cols_gr.checked] = true;
      row[cols_gr.name]    = gr;
    }

    // connect signals
    store_gr->signal_row_changed().connect (
      sigc::mem_fun (this, &PanelMapDB::on_gr_edited));
    store_st->signal_row_changed().connect (
      sigc::mem_fun (this, &PanelMapDB::on_st_edited));

  }

  // callback for updating data from the panel
  void on_gr_edited (const Gtk::TreeModel::Path& path,
                     const Gtk::TreeModel::iterator& iter) {
    set_group_visibility(
      (*iter)[cols_gr.name],
      (*iter)[cols_gr.checked]
    );
  }

  void on_st_edited (const Gtk::TreeModel::Path& path,
                     const Gtk::TreeModel::iterator& iter) {
    set_visibility(
      (*iter)[cols_st.obj],
      (*iter)[cols_st.checked]
    );
  }

};

#endif
