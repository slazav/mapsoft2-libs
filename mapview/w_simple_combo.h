#ifndef W_SIMPLE_COMBO_H
#define W_SIMPLE_COMBO_H

#include <gtkmm.h>

/*
  ComboBox with entries consists of two parts:
  - template type ID
  - string text for the interface
*/

template <typename ID>
class SimpleComboCols : public Gtk::TreeModelColumnRecord{
public:
  SimpleComboCols() { add(id); add(name); }
  Gtk::TreeModelColumn<ID> id;
  Gtk::TreeModelColumn<Glib::ustring> name;
};

template <typename ID>
class SimpleCombo : public Gtk::ComboBox {
public:
  typedef std::pair<ID, std::string> pair_t;
  SimpleCombo(bool has_entry = false): Gtk::ComboBox(has_entry){ }
  SimpleCombo(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder) :
    Gtk::ComboBox(cobject){
    // set widget style (for use in css)
    get_style_context()->add_class("simple_combo_widget");
  }

  void set_values(const pair_t *ib, const pair_t *ie){
    list = Gtk::ListStore::create(columns);
    const pair_t *i;
    for (i=ib; i!=ie; i++){
      Gtk::TreeModel::Row row = *(list->append());
      row[columns.id]   = i->first;
      row[columns.name] = i->second;
    }
    set_model(list);
    clear();
    pack_start(columns.name);
    set_active(0);
  }

  ID get_active_id(){
    Gtk::TreeModel::iterator iter = get_active();
    if(iter){
      Gtk::TreeModel::Row row = *iter;
      return row[columns.id];
    }
    else return ID();
  }

  void set_active_id(ID id){
    for (auto i=list->children().begin(); i!=list->children().end(); ++i){
      if ((ID)(*i)[columns.id] != id) continue;
      set_active(i);
      return;
    }
    set_active(-1);
  }

  void set_first_id(){
    auto i=list->children().begin();
    if (i!=list->children().end()) set_active(i);
  }

private:
  SimpleComboCols<ID> columns;
  Glib::RefPtr<Gtk::ListStore> list;
};

#endif
