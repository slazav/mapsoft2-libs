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
  // add the tag to all objects in mapn
  mapn.iter_start();
  while (!mapn.iter_end()){
    auto p = mapn.iter_get_next();
    p.second.tags.insert(tag);
    mapn.put(p.first, p.second);
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

void
do_join_lines(VMap2 & map, const double D, const double A){

  // approx distance in degrees, for finding objects
  double Dd = D * 180/M_PI/6380000;

  // Work separately for each type:
  for (auto const t: map.get_types()){
    // lines only
    if (VMap2obj::get_class(t) != VMAP2_LINE) continue;

    // Get set of all objects of this type:
    auto objs = map.find(t);
    auto it = objs.begin();
    while (it!=objs.end()){
      auto i = *it; // object id
      auto o=map.get(i);
      bool mod=false;
      // for all line segments:
      for (auto & l:o){
        // find end point and angle
        if (l.size()<2) continue;
        dPoint pt = l[l.size()-1];
        dPoint dpt = l[l.size()-1] - l[l.size()-2];
        double a = 180.0/2/M_PI*atan2(dpt.y,dpt.x);
        dRect r(pt,pt); r.expand(Dd);

        // Find all lines of same type near the end point.
        for (const auto & j:map.find(t,r)){
          auto o1=map.get(j);
          // skip same object,
          // name, comm, tags should match
          if (i==j || o1.name!=o.name ||
              o1.comm!=o.comm || o1.tags!=o.tags) continue;
          // for all segments:
          auto l1=o1.begin();
          while (l1!=o1.end()){
            // find strting point and angle
            if (l1->size()<2) {++l1; continue;}
            dPoint pt1 = (*l1)[0];
            dPoint dpt1 = (*l1)[1]-(*l1)[0];
            double a1 = 180.0/2/M_PI*atan2(dpt1.y,dpt1.x);
            double da = fabs(a1-a); // positive angle difference
            while (da>180) da-=360;   // -180..180
            if (geo_dist_2d(pt,pt1) > D || fabs(da) > A) {++l1; continue;}

            // join segment
            mod=true;
            l.insert(l.end(), l1->begin()+1, l1->end());
            l1 = o1.erase(l1);
          }
          if (o1.size()==0){
            map.del(j);
            objs.erase(j);
          }
          else map.put(j, o1);
        }
      }
      // stay on the object if it was modified
      if (!mod) ++it;
      else map.put(i, o);
    }
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

  // Create labels
  map.iter_start();
  while (!map.iter_end()){
    auto p = map.iter_get_next();
    auto id = p.first;
    auto & obj = p.second;

    auto t = types.find(obj.type);
    if (t==types.end()) continue;
    // create label if needed
    if (ref_tab.count(id)==0 &&
        t->second.label_type>=0 &&
        obj.get_class() != VMAP2_TEXT &&
        obj.name!=""){
      auto id_l = map.add(do_make_label(obj, t->second));
      ref_tab.emplace(id, id_l);
    }

    // Reconnect label to a point (if label_mkpt>=0)
    if (t->second.label_mkpt >=0 &&
        obj.name!="" &&
        obj.get_class() != VMAP2_TEXT &&
        ref_tab.count(id)>0){
      // create point, move object name to it
      VMap2obj pt(VMAP2_POINT, t->second.label_mkpt);
      dPoint p0 = obj.bbox().cnt();
      pt.set_coords(p0);
      pt.name.swap(obj.name);
      auto id_pt = map.add(pt);
      // reconnect all labels to the point
      for (auto i = ref_tab.lower_bound(id);
                i != ref_tab.upper_bound(id); ++i) {
        auto id_l = i->second;
        auto lab = map.get(id_l);
        lab.ref_pt = p0;
        lab.ref_type = pt.type;
        map.put(id_l, lab);
        ref_tab.emplace(id_pt, id_l);
      }
      ref_tab.erase(ref_tab.lower_bound(id),
                    ref_tab.upper_bound(id));
    }

  }

  // Update existing labels
  for (const auto & i:ref_tab) {
    auto id_o = i.first;
    auto id_l = i.second;
    if (id_o == 0xFFFFFFFF) continue;
    auto obj = map.get(id_o);
    auto lab = map.get(id_l);

    // update label type
    auto t = types.find(obj.type);
    if (t!=types.end()){
      if (t->second.label_type<0){
        ids_to_del.insert(id_l);
        continue;
      }
      lab.set_type(VMAP2_TEXT, t->second.label_type);
    }

    // update label name and ref_pt
    lab.name = obj.name;
    if (lab.size()==0 || lab[0].size()==0) continue;
    lab.ref_pt = geo_nearest_pt(obj, lab[0][0]);
    map.put(id_l, lab);
  }

  // Delete unconnected labels:
  uint32_t noid = 0xFFFFFFFF;
  for (auto i=ref_tab.lower_bound(noid);
            i!=ref_tab.upper_bound(noid); ++i)
    map.del(i->second);

  // Delete labels connected to objects with empty label_type:
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
