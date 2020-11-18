#include "panel_trks.h"

void
PanelTrks::add(const std::shared_ptr<GeoTrk> & trk) {
  // depth is set to 0 to evoke refresh!
  std::shared_ptr<GObjTrk> gobj(new GObjTrk(*trk.get()));
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
std::map<GObjTrk*, std::vector<size_t> >
PanelTrks::find_points(const iRect & r) const{
  std::map<GObjTrk*, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    std::shared_ptr<GObjTrk> gobj = c[columns.gobj];
    auto pts = gobj->find_points(r);
    if (pts.size()>0) ret.emplace(gobj.get(), pts);
  }
  return ret;
}

// Find track points near pt.
std::map<GObjTrk*, std::vector<size_t> >
PanelTrks::find_points(const dPoint & pt) const{
  std::map<GObjTrk*, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    std::shared_ptr<GObjTrk> gobj = c[columns.gobj];
    auto pts = gobj->find_points(pt);
    if (pts.size()>0) ret.emplace(gobj.get(), pts);
  }
  return ret;
}

// Find segments near pt.
std::map<GObjTrk*, std::vector<size_t> >
PanelTrks::find_segments(const dPoint & pt) const{
  std::map<GObjTrk*, std::vector<size_t> > ret;
  for (const auto & c: store->children()){
    if (!c[columns.checked]) continue;
    std::shared_ptr<GObjTrk> gobj = c[columns.gobj];
    auto pts = gobj->find_segments(pt);
    if (pts.size()>0) ret.emplace(gobj.get(), pts);
  }
  return ret;
}

bool
PanelTrks::upd_name(GObjTrk * sel_gobj, bool dir){
  bool ret=false;
  for (auto const & row:store->children()){
    std::string name = row[columns.name];
    std::shared_ptr<GObjTrk> gobj = row[columns.gobj];
    if (!gobj) continue;
    // select gobj if sel_gobj!=NULL
    if (sel_gobj && sel_gobj!=gobj.get()) continue;
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
    std::shared_ptr<GObjTrk> gobj = row[columns.gobj];
    gobj->select(store->get_iter(path) == row);
  }
}
