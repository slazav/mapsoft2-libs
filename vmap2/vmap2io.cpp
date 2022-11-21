#include "vmap2io.h"
#include "filename/filename.h"
#include "geo_mkref/geo_mkref.h"
#include "fig_geo/fig_geo.h"
#include "fig_opt/fig_opt.h"
#include "geo_data/geo_utils.h"

/****************************************************************************/

// common for input and output
void
ms2opt_add_vmap2(GetOptSet & opts, bool read, bool write){
  const char *g = "VMAP2";

  if (read || write){
    opts.add("quite",  1, 'q', g,
        "Be quiet, do not print types of skipped objects. "
        " Default: 0. This works when reading/writing MP, VMAP, writing FIG.");
    opts.add("skip_unknown",  1, 0, g,
        "Skip objects which are not defined in typeinfo file."
        " Default: 0. This works when reading/writing MP, VMAP.");

    opts.add("min_depth",  1, 0, "FIG", "minimum depth of map object (default 40)");
    opts.add("max_depth",  1, 0, "FIG", "minimum depth of map object (default 200)");

    ms2opt_add_fig(opts); // FIG group, reading/writing fig files
    opts.remove("fig_header");
  }
  if (read) {
    opts.add("def_label_type",  1, 0, "VMAP",
        "When reading VMAP file, set type for labels which are not "
        " defined in typeinfo file. Use -1 to skip unknown labels (default)."
        " If skip_unknown is set then labels are skipped with unknown objects.");
    if (!write) ms2opt_add_mp_i(opts);// MP group, reading mp files
  }
  if (write) {
    opts.add("mp_ip",        1, 0, "MP", "override MP ID");
    opts.add("mp_name",      1, 0, "MP", "override MP Name");
    opts.add("keep_labels",  1, 0, "VMAP2", "keep old labels");
    opts.add("update_tag",   1, 0, "VMAP2", "only update objects with a tag");
    opts.add("fix_rounding", 1, 0,
       "VMAP2", "Fix rounding errors using information from the output file."
                " This should be used when editing map in fig format and then returning"
                " changes to other format. Then untached points are detected and rounding"
                " errors fixed.");
    opts.add("rounding_acc", 1, 0, "VMAP2",
       "accuracy for fixing rounding errors (m). "
       "Fig accuracy is 1/450 cm, 2.2 m for 1:100'000 maps. Default: 5");
    if (!read) ms2opt_add_mp_o(opts);  // MP group, writing mp files
  }
  if (read && write)  ms2opt_add_mp_io(opts);
}

void
ms2opt_add_vmap2t(GetOptSet & opts){
  const char *g = "VMAP2";
  opts.add("types", 1, 't', g, "File with type information.");
}

/****************************************************************************/
void vmap2_import(const std::vector<std::string> & ifiles, const VMap2types & types,
                 VMap2 & vmap2, const Opt & opts){

  for (const auto ifile:ifiles){
    if (file_ext_check(ifile, ".vmap2db")){
      if (ifile == vmap2.get_dbname()) throw Err()
        << "Can't copy from vmap2 database to itself: " << ifile;
      VMap2 in(ifile);
      in.iter_start();
      while (!in.iter_end()) vmap2.add(in.iter_get_next().second);
    }
    else if (file_ext_check(ifile, ".vmap2")){
      vmap2.read(ifile);
    }
    else if (file_ext_check(ifile, ".vmap")){
      vmap_to_vmap2(ifile, types, vmap2, opts);
    }
    else if (file_ext_check(ifile, ".mp")){
      mp_to_vmap2(ifile, types, vmap2, opts);
    }
    else if (file_ext_check(ifile, ".fig")){
      fig_to_vmap2(ifile, types, vmap2, opts);
    }
    else throw Err() << "unsupported file extension: " << ifile;
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
vmap2_export(VMap2 & vmap2, const VMap2types & types,
             const std::string & ofile, const Opt & opts){

  bool keep_labels = opts.get("keep_labels", false);
  std::string update_tag = opts.get("update_tag");
  bool fix_rounding = opts.get("fix_rounding", false);
  double rounding_acc = opts.get("rounding_acc", 5); // m


  // Merge with old data if needed
  if ((keep_labels || update_tag!="" || fix_rounding) &&
      file_exists(ofile)){

    VMap2 vmap2o;
    vmap2_import({ofile}, types, vmap2o, opts);

    if (keep_labels){
      // remove labels from vmap2
      for (auto const i: vmap2.find_class(VMAP2_TEXT))
        vmap2.del(i);

      // transfer all labels from vmap2o to vmap2
      for (auto const i: vmap2o.find_class(VMAP2_TEXT))
        vmap2.add(vmap2o.get(i));
    }

    if (update_tag!=""){
      // remove objects without the tag in vmap2
      vmap2.iter_start();
      while (!vmap2.iter_end()){
        auto p = vmap2.iter_get_next();
        if (!p.second.tags.count(update_tag)>0)
          vmap2.del(p.first);
      }
      // transfer objects without the tag from vmap2o to vmap2
      vmap2o.iter_start();
      while (!vmap2o.iter_end()){
        auto p = vmap2o.iter_get_next();
        if (!p.second.tags.count(update_tag)>0)
          vmap2.add(p.second);
      }
    }

    // fix rounding errors
    if (fix_rounding)
      do_fix_rounding(vmap2o, vmap2, rounding_acc);

  }

  // Save files
  if (file_ext_check(ofile, ".vmap2db")){
    // If we are saving vmap2db to itself - no need to do anything.
    // If it's a different database, then delete, create it and copy objects.
    if (ofile != vmap2.get_dbname()){
      VMap2::remove_db(ofile);
      VMap2 out(ofile, 1);
      vmap2.iter_start();
      while (!vmap2.iter_end()) out.add(vmap2.iter_get_next().second);
    }
  }
  else if (file_ext_check(ofile, ".vmap2")){
    vmap2.write(ofile);
  }
  else if (file_ext_check(ofile, ".vmap")){
    vmap2_to_vmap(vmap2, types, ofile, opts);
  }
  else if (file_ext_check(ofile, ".mp")){
    vmap2_to_mp(vmap2, types, ofile, opts);
  }
  else if (file_ext_check(ofile, ".fig")){
    vmap2_to_fig(vmap2, types, ofile, opts);
  }
  else throw Err() << "unsupported file extension: " << ofile;
}

/****************************************************************************/
