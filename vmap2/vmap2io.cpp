#include "vmap2io.h"
#include "filename/filename.h"
#include "geo_mkref/geo_mkref.h"
#include "fig_geo/fig_geo.h"
#include "fig_opt/fig_opt.h"
#include "geo_data/geo_utils.h"

/****************************************************************************/

// common for input and output
void
ms2opt_add_vmap2c(GetOptSet & opts){
  const char *g = "VMAP2";
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

void
ms2opt_add_vmap2i(GetOptSet & opts){
  const char *g = "VMAP2";
  opts.add("def_label_type",  1, 0, "VMAP",
      "When reading VMAP file, set type for labels which are not "
      " defined in typeinfo file. Use 0 to skip unknown labels (default)."
      " If skip_unknown is set then labels are skipped with unknown objects.");
  ms2opt_add_mp_i(opts);// MP group, reading mp files
}

void
ms2opt_add_vmap2o(GetOptSet & opts){
  const char *g = "VMAP2";
  opts.add("mp_ip",        1, 0, "MP", "override MP ID");
  opts.add("mp_name",      1, 0, "MP", "override MP Name");
  opts.add("keep_labels",  1, 0, "VMAP2", "keep old labels");
  opts.add("update_tag",   1, 0, "VMAP2", "only update objects with a tag");
  opts.add("fix_rounding", 1, 0,
     "VMAP2", "fix rounding errors (slow, use when copying from fig format)");
  opts.add("rounding_acc", 1, 0, "VMAP2",
     "accuracy for fixing rounding errors (m). "
     "Fig accuracy is 1/450 cm, 2.2 m for 1:100'000 maps. Default: 5");

  ms2opt_add_mp_o(opts);  // MP group, writing mp files
}

// both input and output
void
ms2opt_add_vmap2io(GetOptSet & opts){
  ms2opt_add_vmap2i(opts);
  opts.remove("mp_enc");
  ms2opt_add_vmap2o(opts);
  opts.remove("mp_enc");
  ms2opt_add_vmap2c(opts);
  ms2opt_add_mp_io(opts);
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

  // spacing in degrees:
  double sp = 2*D * 180/M_PI/6380000;

  struct pt_in_cell: public dPoint{
    uint32_t type;
    pt_in_cell(const dPoint & p, const uint32_t t): dPoint(p), type(t){}
  };

  std::multimap<Point<long>, pt_in_cell> cells;

  mapo.iter_start();
  while (!mapo.iter_end()){
    auto obj = mapo.iter_get_next().second;
    for (const auto & li:obj){
      for (const auto & pt:li){
        pt_in_cell p(pt, obj.type);
        long x = long(floor(p.x / sp));
        long y = long(floor(p.y / sp));
        // add point to 4 cells:
        for (long i=0; i<2; i++){
          for (int j=0; j<2; j++){
            cells.emplace(Point<long>(x+i,y+j), p);
          }
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
        auto ci_min = cells.end();
        for (auto ci=cells.lower_bound(cell);
                  ci!=cells.upper_bound(cell); ci++){
          if (obj.type!=ci->second.type) continue;
          double d = geo_dist_2d(ci->second, pt);
          if (d < min_dist){
            ci_min = ci;
            min_dist = d;
          }
        }
        if (ci_min!=cells.end()){
          pt = ci_min->second;
        }
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

    //
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
