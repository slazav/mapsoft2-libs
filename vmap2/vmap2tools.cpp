#include "vmap2tools.h"
#include <map>
#include "geo_data/geo_utils.h"
#include "geom/line_rectcrop.h"

/****************************************************************************/

// Replace labels in mapn with labels from mapo
void
do_keep_labels(VMap2 & mapo, VMap2 & mapn){
  // remove labels from mapn
  for (auto const i: mapn.find_class(VMAP2_TEXT))
    mapn.del(i);

  // transfer all labels from mapo to mapn
  for (auto const i: mapo.find_class(VMAP2_TEXT))
    mapn.add(mapo.get(i));
}

/****************************************************************************/

// Replace all objects in mapn with objects from mapo
// except ones with the given tag
void
do_update_tag(VMap2 & mapo, VMap2 & mapn, const std::string & tag){
  // remove objects without the tag in mapn
  mapn.iter_start();
  while (!mapn.iter_end()){
    auto p = mapn.iter_get_next();
    if (!p.second.tags.count(tag)>0)
      mapn.del(p.first);
  }
  // transfer objects without the tag from mapo to mapn
  mapo.iter_start();
  while (!mapo.iter_end()){
    auto p = mapo.iter_get_next();
    if (!p.second.tags.count(tag)>0)
      mapn.add(p.second);
  }
}

/****************************************************************************/

void
do_fix_rounding(VMap2 & mapo, VMap2 & mapn, double D){
  // Try to fix rounding errors in mapn using information from mapo.
  // Split plane to 4*D x 4*D cells with 2*D spacing, put all points
  // from mapo to the cells.
  // This is only useful for getting information from fig files,
  // but the function does not depend on fig format and located here.
  // Note that fig file has point resolution 1/450 cm. For map scale
  // 1:100'000 this is 2.2 m or 0.0001deg. Angle resolution is 0.001
  // radian which is 0.06 degrees.

  // spacing in degrees:
  double sp = 2*D * 180/M_PI/6380000;
  // angle accuracy
  double ang_acc = 0.002 * 180/M_PI;

  // cell structures
  struct pt_in_cell: public dPoint{
    uint32_t type;
    pt_in_cell(const dPoint & p, const uint32_t t): dPoint(p), type(t){}
  };
  std::multimap<Point<long>, pt_in_cell> pt_cells;

  // same for text objects
  struct lb_in_cell: public dPoint{
    uint32_t type, ref_type;
    float angle;
    lb_in_cell(const dPoint & pt, const uint32_t type,
               const uint32_t ref_type, const float angle):
      dPoint(pt), type(type), ref_type(ref_type), angle(angle){}
  };
  std::multimap<Point<long>, lb_in_cell> lb_cells;

  mapo.iter_start();
  while (!mapo.iter_end()){
    auto obj = mapo.iter_get_next().second;

    // put all object points in the cell map
    for (const auto & li:obj){
      for (const auto & pt:li){
        pt_in_cell p(pt, obj.type);
        long x = long(floor(p.x / sp));
        long y = long(floor(p.y / sp));
        // add point to 4 cells:
        for (long i=0; i<2; i++){
          for (int j=0; j<2; j++){
            pt_cells.emplace(Point<long>(x+i,y+j), p);
          }
        }
      }
    }
    // Same for text reference points and text angles
    // they can be also broken in fig format:
    if (obj.get_class()==VMAP2_TEXT && obj.get_ref_class()!=VMAP2_NONE){
      lb_in_cell p(obj.ref_pt, obj.type, obj.ref_type, obj.angle);
      long x = long(floor(p.x / sp));
      long y = long(floor(p.y / sp));
      // add point to 4 cells:
      for (long i=0; i<2; i++){
        for (int j=0; j<2; j++){
          lb_cells.emplace(Point<long>(x+i,y+j), p);
        }
      }
    }
  }

  // For each point of mapn find the cell, and then
  // find nearest point of correct type in this cell.
  // If distance is less then D update point.
  mapn.iter_start();
  while (!mapn.iter_end()){
    auto p = mapn.iter_get_next();
    auto & obj = p.second;
    for (auto & li:obj){
      for (auto & pt:li){
        Point<long> cell(lround(pt.x/sp), lround(pt.y/sp));
        double min_dist = D;
        auto ci_min = pt_cells.end();
        for (auto ci=pt_cells.lower_bound(cell);
                  ci!=pt_cells.upper_bound(cell); ci++){
          if (obj.type!=ci->second.type) continue;
          double d = geo_dist_2d(ci->second, pt);
          if (d < min_dist){
            ci_min = ci;
            min_dist = d;
          }
        }
        if (ci_min!=pt_cells.end()){
          pt = ci_min->second;
        }
      }
    }
    // labels
    if (obj.get_class()==VMAP2_TEXT && obj.get_ref_class()!=VMAP2_NONE){
      Point<long> cell(lround(obj.ref_pt.x/sp), lround(obj.ref_pt.y/sp));
      double min_dist = 1.0;
      auto ci_min = lb_cells.end();
      for (auto ci=lb_cells.lower_bound(cell);
                ci!=lb_cells.upper_bound(cell); ci++){
        if (obj.type!=ci->second.type) continue;
        if (obj.ref_type!=ci->second.ref_type) continue;
        double dd = geo_dist_2d(ci->second, obj.ref_pt);
        double da = fabs(ci->second.angle - obj.angle);
        if (std::isnan(ci->second.angle) && std::isnan(obj.angle)) da = 0;
        double d = hypot(dd/D, da/ang_acc);
        if (d < min_dist){
          ci_min = ci;
          min_dist = d;
        }
      }
      // If we have a label with same type and ref_type
      // with close ref_pt.
      if (ci_min!=lb_cells.end()){
        // adjust ref_pt:
        obj.ref_pt = ci_min->second;

        // adjust angle:
        obj.angle = ci_min->second.angle;
      }
    }

    mapn.put(p.first, p.second);
  }
}

/****************************************************************************/

// make a new label for object
VMap2obj
do_make_label(const VMap2obj & o, const VMap2type & t){
  if (t.label_type<0) throw Err()
    << "can't make label: no label type";
  VMap2obj label(VMAP2_TEXT, t.label_type);

  dRect r = o.bbox();
  if (r.is_empty()) throw Err()
    << "can't make label: empty object";

  // Label position.
  dPoint pt0 = r.cnt();

  dLine pts;
  pts.push_back(pt0);
  label.push_back(pts);

  label.name = o.name;
  label.ref_type = o.type;
  label.ref_pt = geo_nearest_pt(o, pt0);
  label.scale = t.label_def_scale;
  return label;
}

void
do_update_labels(VMap2 & map, const VMap2types & types){
  double dist1 = 10.0; // move to options?
  double dist2 = 1000.0; // move to options?
  // Build multimap object_id -> label_id
  auto ref_tab = map.find_refs(dist1,dist2);

  std::set<uint32_t> ids_to_del;

  // Loop through VMap2 objects:
  map.iter_start();
  while (!map.iter_end()){
    auto p = map.iter_get_next();
    auto id = p.first;
    auto & o = p.second;

    // check that it's not types.end() before use!
    auto t = types.find(o.type);

    // create label if needed
    if (ref_tab.count(id)==0){

      // find label type in typeinfo
      if (t==types.end()) continue;
      if (t->second.label_type<0) continue;

      // create label object
      map.add(do_make_label(o, t->second));
      continue;
    }

    // for each existing label
    for (auto i=ref_tab.lower_bound(id);
              i!=ref_tab.upper_bound(id); ++i){
      auto id_l = i->second;
      auto label = map.get(id_l);

      // update label type
      if (t!=types.end()){
        if (t->second.label_type<0){
          ids_to_del.insert(id_l);
          continue;
        }
        label.set_type(VMAP2_TEXT, t->second.label_type);
      }

      // update label type, name and ref_pt
      label.name = o.name;
      if (label.size()==0 || label[0].size()==0) continue;
      label.ref_pt = geo_nearest_pt(o, label[0][0]);
      map.put(id_l, label);
    }
  }

  // delete unconnected labels
  uint32_t noid = 0xFFFFFFFF;
  for (auto i=ref_tab.lower_bound(noid);
            i!=ref_tab.upper_bound(noid); ++i)
    map.del(i->second);

  // delete labels connected to objects with empty label_type
  for (const auto & i:ids_to_del) map.del(i);
}

void
do_crop_rect(VMap2 & map, const dRect & r){
  // Loop through VMap2 objects:
  map.iter_start();
  while (!map.iter_end()){
    auto p = map.iter_get_next();
    auto id = p.first;
    auto & o = p.second;
    bool closed = (o.get_class() == VMAP2_POLYGON);

    dMultiLine pts;
    for (auto & line:o){
      rect_crop(r, line, closed);
      auto ml = rect_split_cropped(r, line, closed);
      pts.insert(pts.end(), ml.begin(), ml.end());
    }
    if (pts.empty()){
      map.del(id);
      continue;
    }
    o.set_coords(pts);
    map.put(id, o);
  }
}
