#include "panel_wpts.h"

PanelWpts::ptr_t
PanelWpts::add(const std::shared_ptr<GeoWptList> & wpts) {
  // depth is set to 0 to evoke refresh!
  ptr_t gobj(new GObjWpts(*wpts.get()));
  GObjMulti::add(0, gobj);

  Gtk::TreeModel::iterator it = store->append();
  Gtk::TreeModel::Row row = *it;
  // note: signal_row_changed() is emitted three times from here:
  row[columns.checked] = true;
  row[columns.name]    = wpts->name;
  row[columns.weight]  = Pango::WEIGHT_NORMAL;
  row[columns.data]    = wpts;
  row[columns.gobj]    = gobj;
  select(gobj);
  return gobj;
}

// Find waypoints in a rectangular area
std::map<PanelWpts::ptr_t, std::vector<size_t> >
PanelWpts::find_points(const iRect & r) const{
  std::map<ptr_t, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    ptr_t gobj = c[columns.gobj];
    auto pts = gobj->find_points(r);
    if (pts.size()>0) ret.emplace(gobj, pts);
  }
  return ret;
}

// Find waypoints
std::map<PanelWpts::ptr_t, std::vector<size_t> >
PanelWpts::find_points(const dPoint & pt) const{
  std::map<ptr_t, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    ptr_t gobj = c[columns.gobj];
    auto pts = gobj->find_points(pt);
    if (pts.size()>0) ret.emplace(gobj, pts);
  }
  return ret;
}

void
PanelWpts::del_points(const iRect & r, const ptr_t & obj){
  if (obj) obj->del_points(r);
  else {
    for (auto const & row:store->children()){
      ptr_t o = row[columns.gobj];
      o->del_points(r);
    }
  }
}

bool
PanelWpts::upd_name(ptr_t sel_gobj, bool dir){
  bool ret=false;
  for (auto const & row:store->children()){
    std::string name = row[columns.name];
    ptr_t gobj = row[columns.gobj];
    if (!gobj) continue;
    // select gobj if sel_gobj!=NULL
    if (sel_gobj && sel_gobj!=gobj) continue;

    std::shared_ptr<GeoWptList> wpts = row[columns.data];
    if (name!=wpts->name){
      if (dir) wpts->name = name;
      else row[columns.name] = wpts->name;
      ret = true;
    }
  }
  if (ret) signal_data_changed().emit();
  return ret;
}

void
PanelWpts::on_select(const Gtk::TreeModel::Path& path,
                     Gtk::TreeViewColumn* col){
  std::cerr << "WPT select\n";
  for (auto const & row:store->children()){
    ptr_t gobj = row[columns.gobj];
    gobj->select(store->get_iter(path) == row);
  }
}
