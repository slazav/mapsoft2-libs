#ifndef VIEWER_PANEL_H
#define VIEWER_PANEL_H

#include <gtkmm.h>
#include "viewer/gobj_multi.h"

/* PanelRecord -- TreeModelColumnRecord for WPT/TRK/MAP panels. */

template <typename Tl, typename Td>
class PanelRecord : public Gtk::TreeModelColumnRecord {
public:

    Gtk::TreeModelColumn<bool> checked;
    Gtk::TreeModelColumn<std::string> name;
    Gtk::TreeModelColumn<Pango::Weight> weight;
    Gtk::TreeModelColumn<std::shared_ptr<Tl> > gobj;
    Gtk::TreeModelColumn<std::shared_ptr<Td> > data;

    PanelRecord() {
      add(checked); add(name); add(weight); add(gobj); add(data);
    }
};

/* Panel -- Abstract class for WPT/TRK/MAP panels,
 * child of Gtk::TreeView and GObjMulti.
 */

template <typename Tl, typename Td>
class Panel : public Gtk::ScrolledWindow, public GObjMulti {
  sigc::signal<void> signal_data_changed_;
  Gtk::TreeView *treeview;

public:

  typedef std::shared_ptr<Tl> ptr_t;
  typedef std::map<ptr_t, std::vector<int> > search_t;

  Gtk::Menu *popup_menu; // access from ActionManager

  // This signal is emitted when data is changed
  // (everything we want to save in the file:
  //  name or order, but maybe not visibility).
  sigc::signal<void> & signal_data_changed(){
    return signal_data_changed_;}


  // Constructor.
  Panel() {

    treeview = manage(new Gtk::TreeView);

    store = Gtk::ListStore::create(columns);
    treeview->set_model(store);
    treeview->append_column_editable("V", columns.checked);
    int name_cell_n = treeview->append_column("V", columns.name);

    Gtk::TreeViewColumn* name_column = treeview->get_column(name_cell_n - 1);
    Gtk::CellRendererText* name_cell =
      (Gtk::CellRendererText*)treeview->get_column_cell_renderer(name_cell_n - 1);
    if (name_column && name_cell)
      name_column->add_attribute(
        name_cell->property_weight(), columns.weight);

    store->signal_row_changed().connect (
      sigc::mem_fun (this, &Panel::on_panel_edited));

    treeview->set_enable_search(false);
    treeview->set_headers_visible(false);
    treeview->set_reorderable(false);
    treeview->set_activate_on_single_click(true);

    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Gtk::ScrolledWindow::add(*treeview);

    treeview->set_events(Gdk::BUTTON_PRESS_MASK);
    treeview->signal_button_press_event().connect(
      sigc::mem_fun (this, &Panel::on_button_press), false);

//    treeview->signal_row_activated().connect(
//      sigc::mem_fun (this, &Panel::on_select), false);

  }

  // Add data
  virtual ptr_t add(const std::shared_ptr<Td> & data) = 0;

  // Remove all data
  void remove_all() {
    store->clear();
    GObjMulti::clear();
  }

  // Remove selected object
  void remove_selected(){
    auto it = treeview->get_selection()->get_selected();
    if (!it) return;
    GObjMulti::del(it->get_value(columns.gobj));
    store->erase(it);
  }

  // Remove object
  void remove(const ptr_t & obj){
    for (auto i = store->children().begin();
         i != store->children().end(); i++){
      if (i->get_value(columns.gobj) != obj) continue;
      store->erase(i);
      break;
    }
    GObjMulti::del(obj);
  }


  // get selected data
  Td * get_data() {
    auto i = treeview->get_selection()->get_selected();
    if (!i) return NULL;
    std::shared_ptr<Td> d = (*i)[columns.data];
    return d.get();
  }

  // Hide/Show all
  void show_all(bool show=true){
    for (auto row:store->children())
      row[columns.checked] = show;
  }

  // Invert visibility
  void invert_all(){
    for (auto row:store->children())
      row[columns.checked] = !row[columns.checked];
  }

  // Join visible/all objects
  void join(bool visible) {
    std::shared_ptr<Td> newd(NULL);

    auto i = store->children().begin();
    while (i != store->children().end()){
      if (visible && !(*i)[columns.checked]){
        ++i; continue;
      }

      auto curr = i->get_value(columns.gobj);
      auto data = i->get_value(columns.data);
      if (!curr || !data){ ++i; continue; }

      if (!newd){
        newd = data;
      }
      else {
        newd->insert(newd->end(), data->begin(), data->end());
        newd->name = "JOIN";
      }

      i = store->erase(i);
      GObjMulti::del(curr);
    }
    if (newd && newd->size()) add(newd);
  }

  // Find selected object
  ptr_t find_selected() {
    auto const it = treeview->get_selection()->get_selected();
    if (!it) return NULL;
    return it->get_value(columns.gobj);
  }

  // Find first visible object
  ptr_t find_first() const {
    for (auto row:store->children()) {
      if (row[columns.checked])
        return row[columns.gobj];
    }
    return NULL;
  }

  // Get all/visible data
  void get_data(GeoData & data, bool visible) const {
    for (auto row:store->children()) {
       if (visible && !row[columns.checked]) continue;
       std::shared_ptr<Td> d = row[columns.data];
       data.push_back(*d.get());
    }
  }

  // Get selected data
  void get_sel_data(GeoData & data) {
    auto i = treeview->get_selection()->get_selected();
    if (!i) return;
    std::shared_ptr<Td> d = (*i)[columns.data];
    data.push_back(*d.get());
  }

  // get sub-object range
  dRect get_range(){
    auto i = treeview->get_selection()->get_selected();
    if (!i) return dRect();
    ptr_t gobj = (*i)[columns.gobj];
    return gobj->bbox();
  }

  // move selected object (up/down/top/bottom)
  void move(bool up, bool onestep){
    auto it1 = treeview->get_selection()->get_selected();
    auto it2 = it1;
    if (!it1) return;
    if (up && (it1 == store->children().begin())) return;

    if (!onestep){
      if (up) it2--; else it2++;
      if (!it2) return;
    }
    else{
      if (up) it2 = store->children().begin();
      else {it2 = store->children().end(); it2--;}
    }
    store->iter_swap(it1, it2);
    upd_wp();
  }

  // callback for updating data from the panel
  void on_panel_edited (const Gtk::TreeModel::Path& path,
                        const Gtk::TreeModel::iterator& iter) {
    upd_wp();
    upd_name();
  }

  // callback for opening the popup menu
  bool on_button_press (GdkEventButton * event) {
    if (event->button != 3) return false;
    if (popup_menu) popup_menu->popup(event->button, event->time);
    return true;
  }

  // call when object is selected
  virtual void on_select(const Gtk::TreeModel::Path& path,
                               Gtk::TreeViewColumn* col) {};


  // update names in data (no need to redraw)
  virtual bool upd_name(ptr_t sel_gobj = NULL, bool dir=true) = 0;

  // update gobj depth and visibility in GObjMulti
  // according to TreeView
  bool upd_wp(){
    bool ret=false;
    Gtk::TreeNodeChildren::const_iterator i;
    int d=1;

    for (auto const & row:store->children()){
      auto gobj = row.get_value(columns.gobj);
      if (!gobj) continue;
      // update depth
      if (get_depth(gobj) != d){
        set_depth(gobj, d);
        ret = true;
      }
      d++;
    }
    if (ret) signal_data_changed().emit();

    for (auto const & row:store->children()){
      auto gobj = row.get_value(columns.gobj);
      if (!gobj) continue;
      // update visibility
      bool act = row[columns.checked];
      if (get_visibility(gobj) != act){
        set_visibility(gobj, act);
        ret = true;
      }
    }
    if (ret) signal_redraw_me().emit(iRect());
    return ret;
  }

  // number of objects in the panel
  int size() {return GObjMulti::size();}


protected:
  Glib::RefPtr<Gtk::ListStore> store;
  PanelRecord<Tl, Td> columns;
};

#endif
