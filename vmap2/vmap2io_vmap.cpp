#include "vmap2io.h"
#include "geo_data/geo_utils.h"

/****************************************************************************/

// angle (deg->deg)
// In the old vmap format angle has a strange meaning: atan2(dlat,dlon)
// (lonlat coordinates have different scales in horizontal and vertical directions)
// We want to convert the angle to real angle from north direction
// We can do it only if we know latitude.
// Direction of angle is also different (ccw instead of cw)
double
angle_to_vmap(const double a, const double lat){
  if (std::isnan(a)) return a;
  return 180/M_PI*atan2(cos(M_PI/180*lat)*sin(-M_PI/180*a), cos(M_PI/180*a));
}
double
angle_from_vmap(const double a, const double lat){
  return 180/M_PI*atan2(sin(-M_PI/180*a), cos(M_PI/180*lat)*cos(M_PI/180*a));
}


/****************************************************************************/

void
vmap_to_vmap2(const std::string & ifile, const VMap2types & types,
              VMap2 & vmap2, const Opt & opts){

  VMap vmap = read_vmap(ifile);

  // this should go to options:
  // default type for unknown labels (use 0 to skip them)
  uint32_t def_label_type = opts.get<uint32_t>("def_label_type", 0);

  // be quiet (by default types of skipped objects are printed)
  bool quiet = opts.get("quite", false);

  // skip objects which are not in typeinfo file
  bool skip_unknown = opts.get("skip_unknown", false);

  std::set<uint32_t> skipped_types;
  std::set<uint32_t> skipped_labels;

  for (auto const & o:vmap){
   // skip empty objects
    if (o.is_empty()) continue;

    // get type in Vmap2 format
    uint16_t cl = o.type >> 20;
    if (cl>2) throw Err() << "bad object class: " << cl;
    uint16_t tnum = o.type & 0xFFFF;
    uint32_t type = (cl<<24) + tnum;

    // find type information if any.
    uint32_t label_type = def_label_type;
    float label_def_scale = 1.0;
    if (types.count(type)){
      auto t = types.find(type)->second;
      label_type = t.label_type;
      label_def_scale = t.label_def_scale;
    }
    else if (skip_unknown) {
      if (!quiet) skipped_types.insert(type);
      continue;
    }
    if (o.labels.size()>0 && label_type == 0 && !quiet)
      skipped_labels.insert(type);

    // make object
    VMap2obj o1(type);

    // name and comments
    o1.name = o.text;
    o1.comm = o.comm;

    // source
    if (o.opts.exists("Source"))
      o1.tags.insert(o.opts.get("Source"));

    // angle
    double angle = o.opts.get("Angle", std::nan(""));
    if (!std::isnan(angle)){
      if (o.size()>0 && o[0].size()>0){
        double lat = o.bbox().cnt().y;
        o1.angle = angle_from_vmap(angle, lat);
      }
      else
        o1.angle = -angle;
    }
    // set coordinates
    o1.dMultiLine::operator=(o);

    vmap2.add(o1);

    // lables attached to the object:
    if (label_type > 0){
      auto tt = VMap2obj::make_type(VMAP2_TEXT, label_type);
      for (auto const & l:o.labels){
        VMap2obj l1(tt);
        l1.name = o.text;
        l1.comm = o.comm;

        // angle
        if (!l.hor && !std::isnan(l.ang))
          l1.angle = angle_from_vmap(l.ang, l.pos.y);

        // coordinate
        dLine pts; pts.push_back(l.pos);
        l1.push_back(pts);

        // orientation
        switch (l.dir){
          case 0: l1.align = VMAP2_ALIGN_SW; break;
          case 1: l1.align = VMAP2_ALIGN_S; break;
          case 2: l1.align = VMAP2_ALIGN_SE; break;
        }

        // Convert scale:
        // In vmap format label scale is written
        // as a correction to font size: -1, 0, +1, etc.
        // Convert it to scaling factor using some
        // "natural" font size.
        // Then apply label_def_scale.
        if (l.fsize) l1.scale = (8.0 + l.fsize)/8.0 * label_def_scale;

        // Reference point and type
        l1.ref_type = type;
        l1.ref_pt = geo_nearest_pt(o, l.pos);

        vmap2.add(l1);
      }
    }
    // Separate labels are not converted: it's not that easy to
    // understand related objects. To fix this (if needed) one should implement
    // analog of join_labels/split_labels for vmap as it was done in mapsoft1.
  }
  if (skipped_types.size()){
    std::cerr <<
       "Reading VMAP file: some types were skipped because "
       "skip_unknown parameter is set and there is no information in "
       "the typeinfo file:\n";
    for (const auto & t:skipped_types)
      std::cerr << VMap2obj::print_type(t) << "\n";
  }
  if (skipped_labels.size()){
    std::cerr <<
       "Reading VMAP file: labels for following types were skipped because "
       "label_type in the typeinfo file is 0, or type is unknown and "
       "def_label_type parameter is 0:\n";
    for (const auto & t:skipped_labels)
      std::cerr << VMap2obj::print_type(t) << "\n";
  }
}

/****************************************************************************/

void
vmap2_to_vmap(VMap2 & vmap2, const VMap2types & types,
              const std::string & ofile, const Opt & opts){
  VMap vmap;

  double dist1 = 10.0; // move to options?
  double dist2 = 1000.0; // move to options?

  // Build multimap object_id -> label_id
  auto ref_tab = vmap2.find_refs(dist1,dist2);

  // be quiet (by default types of skipped objects are printed)
  bool quiet = opts.get("quite", false);

  // skip objects which are not in typeinfo file
  bool skip_unknown = opts.get("skip_unknown", false);

  std::set<uint32_t> skipped_types;

  // Loop through VMap2 objects:
  vmap2.iter_start();
  while (!vmap2.iter_end()){
    auto p = vmap2.iter_get_next();
    auto id = p.first;
    auto o = p.second;
    VMapObj o1;

    // Convert type to vmap format
    auto c = o.type>>24;
    switch (c){
      case VMAP2_POINT:   o1.type = (o.type & 0xFFFF); break;
      case VMAP2_LINE:    o1.type = (o.type & 0xFFFF) | 0x100000; break;
      case VMAP2_POLYGON: o1.type = (o.type & 0xFFFF) | 0x200000; break;
      case VMAP2_TEXT:    continue;
      default: throw Err() << "vmap2_to_vmap: unsupported object class: " << c;
    }

    // skip unknown types if needed.
    if (!types.count(o.type) && !quiet){
      skipped_types.insert(o.type);
      continue;
    }

    // name, comment
    o1.text = o.name;
    o1.comm = o.comm;

    // source
    if (o.tags.size()>0)
      o1.opts.put("Source", *o.tags.begin());

    // angle
    if (!std::isnan(o.angle)){
      if (o.size()>0 && o[0].size()>0){
        double lat = o.bbox().cnt().y;
        o1.opts.put("Angle", angle_to_vmap(o.angle, lat));
      }
      else
        o1.opts.put("Angle", -o.angle);
    }

    // set coordinates
    o1.dMultiLine::operator=(o);

    // Process labels:
    auto itp = ref_tab.equal_range(id);
    for (auto it=itp.first; it!=itp.second; it++){

      auto l = vmap2.get(it->second);
      VMapLab l1;
      if (l.get_class() != VMAP2_TEXT) continue;
      if (l.size()==0 || l[0].size()==0) continue;
      l1.pos = l[0][0];
      if (std::isnan(l.angle)){
        l1.hor = true;
        l1.ang = 0;
      }
      else {
        l1.hor = false;
        l1.ang = angle_to_vmap(l.angle, l1.pos.y);
      }
      switch (l.align) {
        case VMAP2_ALIGN_NW:
        case VMAP2_ALIGN_W:
        case VMAP2_ALIGN_SW:
          l1.dir = 0; break;
        case VMAP2_ALIGN_N:
        case VMAP2_ALIGN_C:
        case VMAP2_ALIGN_S:
          l1.dir = 1; break;
        case VMAP2_ALIGN_NE:
        case VMAP2_ALIGN_E:
        case VMAP2_ALIGN_SE:
          l1.dir = 2; break;
      }

      // Convert scale:
      // In vmap format label scale is written
      // as a correction to font size: -1, 0, +1, etc.
      // Convert it to scaling factor using some
      // "natural" font size.
      l1.fsize = 0;
      if (l.scale!=1.0) l1.fsize = rint(8.0*(l.scale-1));

      o1.labels.push_back(l1);
    }
    vmap.push_back(o1);
  }

  if (skipped_types.size()){
    std::cerr <<
       "Writing VMAP file: some types were skipped because "
       "skip_unknown parameter is set and there is no information in "
       "the typeinfo file:\n";
    for (const auto & t:skipped_types)
      std::cerr << VMap2obj::print_type(t) << "\n";
  }

  write_vmap(ofile, vmap);
}

