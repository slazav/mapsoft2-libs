#include "panel_trks.h"

void
PanelTrks::add(const std::shared_ptr<GeoTrk> & trk) {
  // depth is set to 0 to evoke refresh!
  ptr_t gobj(new GObjTrk(*trk.get()));
  GObjMulti::add(0, gobj);

  Gtk::TreeModel::iterator it = store->append();
  Gtk::TreeModel::Row row = *it;
  // note: signal_row_changed() is emitted three times from here:
  row[columns.checked] = true;
  row[columns.name]    = trk->name;
  row[columns.weight]  = Pango::WEIGHT_NORMAL;
  row[columns.data]    = trk;
  row[columns.gobj]    = gobj;
}

// Find track points in a rectangular area
std::map<PanelTrks::ptr_t, std::vector<size_t> >
PanelTrks::find_points(const iRect & r) const{
  std::map<ptr_t, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    ptr_t gobj = c[columns.gobj];
    auto pts = gobj->find_points(r);
    if (pts.size()>0) ret.emplace(gobj, pts);
  }
  return ret;
}

// Find track points near pt.
std::map<PanelTrks::ptr_t, std::vector<size_t> >
PanelTrks::find_points(const dPoint & pt) const{
  std::map<ptr_t, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    ptr_t gobj = c[columns.gobj];
    auto pts = gobj->find_points(pt);
    if (pts.size()>0) ret.emplace(gobj, pts);
  }
  return ret;
}

// Find segments near pt.
std::map<PanelTrks::ptr_t, std::vector<size_t> >
PanelTrks::find_segments(const dPoint & pt) const{
  std::map<ptr_t, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    ptr_t gobj = c[columns.gobj];
    auto pts = gobj->find_segments(pt);
    if (pts.size()>0) ret.emplace(gobj, pts);
  }
  return ret;
}

void
PanelTrks::del_points(const iRect & r, const ptr_t & obj){
  if (obj) obj->del_points(r);
  else {
    for (auto const & row:store->children()){
      ptr_t o = row[columns.gobj];
      o->del_points(r);
    }
  }
}

bool
PanelTrks::upd_name(ptr_t sel_gobj, bool dir){
  bool ret=false;
  for (auto const & row:store->children()){
    std::string name = row[columns.name];
    ptr_t gobj = row[columns.gobj];
    if (!gobj) continue;
    // select gobj if sel_gobj!=NULL
    if (sel_gobj && sel_gobj!=gobj) continue;
    std::shared_ptr<GeoTrk> trk = row[columns.data];

    if (name!=trk->name){
      if (dir) trk->name = name;
      else row[columns.name] = trk->name;
      ret = true;
    }
  }
  if (ret) signal_data_changed().emit();
  return ret;
}

void
PanelTrks::on_select(const Gtk::TreeModel::Path& path,
                     Gtk::TreeViewColumn* col){
  std::cerr << "TRK select\n";
  for (auto const & row:store->children()){
    ptr_t gobj = row[columns.gobj];
    gobj->select(store->get_iter(path) == row);
  }
}
