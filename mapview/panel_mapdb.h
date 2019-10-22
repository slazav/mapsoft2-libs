#ifndef MAPVIEW_PANEL_MAPDB_H
#define MAPVIEW_PANEL_MAPDB_H

#include "mapdb/gobj_mapdb.h"

/**********************************************************/
/* Control panel for MapDB.
*/

class PanelMapDBRecord : public Gtk::TreeModelColumnRecord {
public:
    Gtk::TreeModelColumn<bool> checked;
    Gtk::TreeModelColumn<std::string> step;

    PanelMapDBRecord() {
      add(checked); add(step);
    }
};

/**********************************************************/

class PanelMapDB : public Gtk::ScrolledWindow, public GObjMapDB {
  Gtk::TreeView *treeview;
  Glib::RefPtr<Gtk::ListStore> store;
  PanelMapDBRecord columns;

public:
  PanelMapDB(const std::string & mapdir):
      GObjMapDB(mapdir) {

    treeview = manage(new Gtk::TreeView);
    store = Gtk::ListStore::create(columns);
    treeview->set_name("VMAP");
    treeview->set_model(store);
    treeview->append_column_editable("V", columns.checked);
    treeview->append_column_editable("V", columns.step);

    store->signal_row_changed().connect (
      sigc::mem_fun (this, &PanelMapDB::on_panel_edited));

    treeview->set_enable_search(false);
    treeview->set_headers_visible(false);
    treeview->set_reorderable(false);

    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Gtk::ScrolledWindow::add(*treeview);
  }

  // callback for updating data from the panel
  void on_panel_edited (const Gtk::TreeModel::Path& path,
                        const Gtk::TreeModel::iterator& iter) {
  }

};

#endif
