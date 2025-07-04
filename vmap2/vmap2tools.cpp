#include "vmap2tools.h"
#include <map>
#include "geo_data/geo_utils.h"
#include "geo_data/conv_geo.h"
#include "geo_nom/geo_nom_fi.h"
#include "geom/line_rectcrop.h"
#include "geom/poly_tools.h"

/****************************************************************************/

// Replace labels in mapn with labels from mapo
void
do_keep_labels(VMap2 & mapo, VMap2 & mapn, const double dist){
  auto old_labels = mapo.find_class(VMAP2_TEXT);
  auto new_labels = mapn.find_class(VMAP2_TEXT);

  // transfer labels from mapo to mapn
  for (auto const i: old_labels)
    mapn.add(mapo.get(i));

  // remove labels from mapn if they match old labels
  for (auto const i: new_labels){

    if (dist > 0){
      bool match = false;
      const auto & on = mapo.get(i);
      for (auto const j: old_labels){
        const auto & oo = mapo.get(j);
        if (oo.ref_type != on.ref_type ||
           geo_dist_2d(oo.ref_pt, on.ref_pt) >= dist) continue;
        match = true;
        break;
      }
      if (!match) continue;
    }

    mapn.del(i);
  }

}


/****************************************************************************/

// Add source=src option to all objects in mapn, add all objects from mapo without it
void
do_replace_source(VMap2 & mapo, VMap2 & mapn, const std::string & src){
  // add source=src option to all objects in mapn
  mapn.iter_start();
  while (!mapn.iter_end()){
    auto p = mapn.iter_get_next();
    p.second.opts.emplace("Source", src);
    mapn.put(p.first, p.second);
  }
  // transfer objects without the option from mapo to mapn
  mapo.iter_start();
  while (!mapo.iter_end()){
    auto p = mapo.iter_get_next();
    if (p.second.opts.get("Source") != src) mapn.add(p.second);
  }
}

/****************************************************************************/

// Replace all objects in mapn with objects from mapo
// except ones with the given type
void
do_replace_type(VMap2 & mapo, VMap2 & mapn, const uint32_t type){
  // delete objects of other types in mapn
  mapn.iter_start();
  while (!mapn.iter_end()){
    auto p = mapn.iter_get_next();
    if (p.second.type!=type) mapn.del(p.first);
  }

  // transfer objects of other types from mapo to mapn
  mapo.iter_start();
  while (!mapo.iter_end()){
    auto p = mapo.iter_get_next();
    if (!p.second.type!=type)
      mapn.add(p.second);
  }
}

/****************************************************************************/

// Keep all objects in mapn, transfer objects of other types from mapo
void
do_replace_types(VMap2 & mapo, VMap2 & mapn){

  // transfer objects of other types from mapo to mapn
  mapo.iter_start();
  auto types = mapn.get_types();
  while (!mapo.iter_end()){
    auto p = mapo.iter_get_next();
    if (types.count(p.second.type)==0) mapn.add(p.second);
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
        double a = 180.0/M_PI*atan2(dpt.y,dpt.x);
        dRect r(pt,pt); r.expand(Dd);

        // Find all lines of same type near the end point.
        for (const auto & j:map.find(t,r)){
          // skip same object
          if (i==j) continue;
          auto o1=map.get(j);
          // name, comm, opts should match
          if (o1.name!=o.name ||
              o1.comm!=o.comm || o1.opts!=o.opts) continue;
          // for all segments:
          auto l1=o1.begin();
          while (l1!=o1.end()){
            // find strting point and angle
            if (l1->size()<2) {++l1; continue;}
            dPoint pt1 = (*l1)[0];
            dPoint dpt1 = (*l1)[1]-(*l1)[0];
            double a1 = 180.0/M_PI*atan2(dpt1.y,dpt1.x);
            double da = fabs(a1-a); // positive angle difference
            while (da>180) da-=360;   // -180..180
            if (geo_dist_2d(pt,pt1) > D || fabs(da) > A) {++l1; continue;}

            // join segment
            mod=true;
            l.insert(l.end(), l1->begin()+1, l1->end());
            l1 = o1.erase(l1);
          }
          if (mod){
            if (o1.size()==0){
              map.del(j);
              objs.erase(j);
            }
            else map.put(j, o1);
          }
          if (mod) break; // we want to re-write object and re-calculate it's end point and angle
        } // segments of second object
        if (mod) break;
      } // second objects
      // stay on the object if it was modified
      if (!mod) ++it;
      else map.put(i, o);
    }
  }
}

/****************************************************************************/

void
do_filter_pts(VMap2 & map, const double D){
  map.iter_start();
  while (!map.iter_end()){
    auto p = map.iter_get_next();
    auto & obj(p.second);
    auto cl = obj.get_class();
    if (cl!=VMAP2_LINE && cl!=VMAP2_POLYGON) continue;
    line_filter_v1((dMultiLine&)obj, D, 0, &geo_dist_2d);
    if (obj.size()==0) map.del(p.first);
    else map.put(p.first, obj);
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
  dPoint pt0;
  nearest_vertex(o, r.cnt(), &pt0,
    (double (*)(const dPoint&, const dPoint&))geo_dist_2d);

  if (t.label_def_mshift != dPoint()){
    // convert meters to degrees
    dPoint sh = t.label_def_mshift;
    sh *= 1.0/6380e3 * 180/M_PI;
    sh.y /= cos(M_PI*pt0.y/180.0);
    pt0+=sh;
  }

  label.add_point(pt0);
  nearest_vertex(o, pt0, &label.ref_pt,
    (double (*)(const dPoint&, const dPoint&))geo_dist_2d);
  label.name = o.name;
  label.ref_type = o.type;
  nearest_vertex(o, pt0, &label.ref_pt,
    (double (*)(const dPoint&, const dPoint&))geo_dist_2d);
  label.scale = t.label_def_scale * o.scale;
  label.align = t.label_def_align;
  return label;
}

void
do_update_labels(VMap2 & map, const VMap2types & types, const bool label_names){
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

    // remove extra labels
    auto maxnum = t->second.label_maxnum;
    if (maxnum == -2) {
      if (obj.get_class() == VMAP2_POINT) maxnum=1;
      else if (obj.get_class() == VMAP2_TEXT) maxnum=0;
    }
    if (maxnum >= 0) {
      while (ref_tab.count(id) > maxnum){
        // remove label with max.dist between pt and ref_pt
        auto it = ref_tab.lower_bound(id);
        auto m_it = it;
        double m_dist=0;
        while (it!=ref_tab.upper_bound(id)){
          auto l = map.get(it->second);
          auto d = nearest_vertex(l, l.ref_pt, (dPoint*)NULL,
                   (double (*)(const dPoint&, const dPoint&))geo_dist_2d);
          if (m_dist < d){ m_dist=d; m_it=it; }
          ++it;
        }
        auto id_l = m_it->second;
        ref_tab.erase(m_it);
        ref_tab.emplace(0xFFFFFFFF, id_l);
      }
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

    // update name
    if (label_names) {
      obj.name = lab.name;
      map.put(id_o, obj);
    }
    else {
      lab.name = obj.name;
    }

    // update ref_pt
    if (lab.size()==0 || lab[0].size()==0) continue;
    nearest_vertex(obj, lab[0][0], &lab.ref_pt,
      (double (*)(const dPoint&, const dPoint&))geo_dist_2d);
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
do_crop_rect(VMap2 & map, const dRect & r, const bool crop_labels){
  // Loop through VMap2 objects:
  map.iter_start();
  while (!map.iter_end()){
    auto p = map.iter_get_next();
    auto id = p.first;
    auto & o = p.second;
    if (!crop_labels && o.get_class() == VMAP2_TEXT) continue;

    bool closed = (o.get_class() == VMAP2_POLYGON);

    o.set_coords(rect_crop_multi(r, o, closed));

    if (o.empty()) map.del(id);
    else map.put(id, o);
  }
}

void
do_crop_nom_fi(VMap2 & map, const std::string & name, const bool crop_labels){
  ConvGeo cnv("ETRS-TM35FIN");
  auto box = nom_to_range_fi(name);

  // Loop through VMap2 objects:
  map.iter_start();
  while (!map.iter_end()){
    auto p = map.iter_get_next();
    auto id = p.first;
    auto & o = p.second;
    if (!crop_labels && o.get_class() == VMAP2_TEXT) continue;

    bool closed = (o.get_class() == VMAP2_POLYGON);

    cnv.bck(o); // convert to ETRS-TM35FIN;
    o.set_coords(rect_crop_multi(box, o, closed)); // crop
    cnv.frw(o); // convert to WGS;

    if (o.empty()) map.del(id);
    else map.put(id, o);
  }

}

/********************************************************************/

// helper for vmap_move_ends
int
vmap_move_ends_pt(VMap2 & vmap, const int id0, dPoint & p1, dPoint & p2,
             const std::list<std::string> types, double r){

  // some range around p1 (larger then r)
  dRect rng(p1,p1);
  double d2m = 6380e3*M_PI/180.0; // m/d
  rng.expand(r / d2m);

  dLine dest; // possible destination points

  for (const auto type: types){
    for (const auto id: vmap.find(type, rng)){
      if (id==id0) continue;
      auto obj = vmap.get(id);
      auto cl = obj.get_class();
      for (auto & l:obj){

        // object node near point p1 -> move p1 there
        for (auto & p:l){
          if (geo_dist_2d(p1,p) > r) continue;
          //std::cerr << " p: " << p1 << " -> " << p << "\n";
          dest.push_back(p);
        }

        // object segment near p1 -> change length of p1-p2 segment
        if (cl == VMAP2_POINT) continue;
        for (size_t j=0; j<l.size(); j++){
          auto q1 = l[j];
          auto q2 = (j==l.size()-1) ? l[0]:l[j+1];
          if (q1==q2) continue;
          if (cl == VMAP2_LINE && j==l.size()-1) continue;

          dPoint cr;
          segment_cross_2d(p1, p2, q1, q2, cr);

          if (std::isnan(cr.x) || std::isinf(cr.x) ||
              std::isnan(cr.y) || std::isinf(cr.y)) continue;
          double dp = geo_dist_2d(p1,p2);
          double dq = geo_dist_2d(q1,q2);
          if (geo_dist_2d(cr,q1) >= dq || geo_dist_2d(cr,q2) >= dq) continue;
          if (geo_dist_2d(cr,p1) > r) continue;
          //std::cerr << " s: " << p1 << " -> " << cr << "\n";
          dest.push_back(cr);
        }
      }
    }
  }
  if (dest.size()==0) return 0;
  // choose nearest destination point
  dPoint pm = dest[0];
  for (const auto p:dest)
    if (geo_dist_2d(p1,p) < geo_dist_2d(p1, pm)) pm=p;
  p1 = pm;
  return 1;
}

void
vmap_move_ends(VMap2 & vmap,
    const std::string & type0,
    const std::list<std::string> types,
    double r){

  size_t gc=0;
  //std::cerr << "  vmap_move_ends (" << type0 << ")\n";
  for (const auto id0: vmap.find(type0)){
    auto obj0 = vmap.get(id0);

    int count = 0;
    for (auto & l:obj0){
      if (l.size()<2) continue;
      // too short line can collapse into one point
      if (geo_dist_2d(l[0], l[l.size()-1]) <= 2*r) continue;
      count +=
        vmap_move_ends_pt(vmap, id0, l[0], l[1], types, r) +
        vmap_move_ends_pt(vmap, id0, l[l.size()-1], l[l.size()-2], types, r);
    }
    if (count) vmap.put(id0, obj0);
    gc+=count;
  }
  if (gc) std::cerr << "  move segment end (" << type0 << "): " << gc << "\n";
}

/********************************************************************/

void
vmap_rem_dups(VMap2 & vmap, const std::string & type, double r){
  size_t gcp=0, gcs=0;
  for (const auto id: vmap.find(type)){
    auto obj = vmap.get(id);

    size_t cs=0, cp=0;
    auto l = obj.begin();
    while (l!=obj.end()) {

      // remove short segments
      if (l->size()<2) {
        //std::cerr << "remove empty segment in " << id << "\n";
        l=obj.erase(l);
        cs++;
        continue;
      }
      auto i = l->begin();
      while (i+1 != l->end()){
        if (geo_dist_2d(*i, *(i+1)) < r){
          //std::cerr << "remove duplicated point in " << id << "\n";
          i=l->erase(i);
          cp++;
        }
        else i++;
      }
      l++;
    }
    if (cs || cp){
      if (obj.size()) vmap.put(id, obj);
      else vmap.del(id);
    }
    gcp+=cp;
    gcs+=cs;
  }
  if (gcs) std::cerr << "  remove 1-point segments (" << type << "):  " << gcs << "\n";
  if (gcp) std::cerr << "  remove duplicated points (" << type << "): " << gcp << "\n";
}

/********************************************************************/

// Join objects with best matching angles (with min_ang limit)
// (should be run after vmap_move_ends and vmap_rem_dups)
void
vmap_join(VMap2 & vmap, const std::string & type, double min_ang){

  // fill main data structure, collect information about all line ends:
  // point -> [object id - segment number - begin/end - direction]
  struct inf {
    size_t id, seg, end;
    dPoint p1,p2;
  };
  std::multimap<iPoint, inf> data;
  std::set<iPoint> keys;

  size_t count=0;

  //std::cerr << "  vmap_join (" << type << ")\n";
  for (const auto id: vmap.find(type)){
    auto obj = vmap.get(id);
    for (size_t seg = 0; seg<obj.size(); ++seg){
      if (obj[seg].size()<2) continue;
      size_t N = obj[seg].size()-1;
      data.emplace(obj[seg][0]*1e6, inf({id, seg, 0, obj[seg][0], obj[seg][1]}));
      data.emplace(obj[seg][N]*1e6, inf({id, seg, 1, obj[seg][N], obj[seg][N-1]}));
      keys.emplace(obj[seg][0]*1e6);
      keys.emplace(obj[seg][N]*1e6);
    }
  }

  for (const auto p: keys){
    if (data.count(p)<2) continue;

    // find pair of points with best direction match
    auto range = data.equal_range(p);
    double minc = 2.0; // min cos value
    auto i1m(range.first), i2m(range.first);
    for (auto i1 = range.first; i1 != range.second; ++i1){
      for (auto i2 = range.first; i2 != i1; ++i2){
        if (i1->second.id == i2->second.id) continue; // circular/short object
        auto v1 = norm2d(i1->second.p2 - i1->second.p1);
        auto v2 = norm2d(i2->second.p2 - i2->second.p1);
        auto c = pscal2d(v1,v2);
        if (c<minc) { minc=c; i1m=i1; i2m=i2; }
      }
    }
    if (minc>1.0) continue;
    if (acos(minc)<min_ang*M_PI/180.0) continue;

    //std::cerr << " join " << i1m->second.id << " " << i2m->second.id
    //          << " angle: " << acos(minc)*180/M_PI << "\n";

    // join objects
    auto A = i1m->second; // make copy (objects will be modified)
    auto B = i2m->second;
    auto O1 = vmap.get(A.id);
    auto O2 = vmap.get(B.id);

    if (A.end && !B.end)
      O1[A.seg].insert(O1[A.seg].end(), O2[B.seg].begin()+1, O2[B.seg].end());
    else if (A.end && B.end)
      O1[A.seg].insert(O1[A.seg].end(), O2[B.seg].rbegin()+1, O2[B.seg].rend());
    else if (!A.end && !B.end){
      O1[A.seg].erase(O1[A.seg].begin());
      O1[A.seg].insert(O1[A.seg].begin(), O2[B.seg].rbegin(), O2[B.seg].rend());
    }
    else if (!A.end && B.end){
      O1[A.seg].erase(O1[A.seg].begin());
      O1[A.seg].insert(O1[A.seg].begin(), O2[B.seg].begin(), O2[B.seg].end());
    }

    O2.erase(O2.begin()+B.seg);
    if (O2.size()) vmap.put(B.id, O2);
    else vmap.del(B.id);

    vmap.put(A.id, O1);

    // update data (we will never return to same point,
    // but can meet other end of deleted segment of O2)
    for (auto & d:data){
      if (d.second.id==B.id && d.second.seg==B.seg){
        //std::cerr << "  replace: " << B.id << " " << A.id << "\n";
        d.second.id = A.id;
        d.second.seg = A.seg;
        d.second.end = A.end;
      }
    }
    count++;
  }
  if (count) std::cerr << "  join segments (" << type << "): " << count << "\n";
}
