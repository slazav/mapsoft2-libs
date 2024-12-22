#include "vmap2io.h"
#include "vmap2tools.h"
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

    opts.add("trk_type",  1, 0, g, "select type for GPX waypoint import/export");
    opts.add("wpt_type",  1, 0, g, "select type for GPX track import/export");
    opts.add("wpt_pref",  1, 0, g, "prefix for waypoint names (names w/o prefix are skipped). Default: \"=\"");

    ms2opt_add_fig(opts); // FIG group, reading/writing fig files
    opts.remove("fig_header");
  }
  if (read) {
    opts.add("def_label_type",  1, 0, "VMAP",
        "When reading VMAP file, set type number for labels which are not "
        " defined in typeinfo file. Use -1 to skip unknown labels (default)."
        " If skip_unknown is set then labels are skipped with unknown objects.");

    opts.add("osm_conf",  1, 0, "OSM",
        "Configuration file for OSM XML -> VMAP2 conversion");
    opts.add("osm_ids",  0, 0, "OSM",
        "Put OSM IDs in object comments");
    opts.add("osm_tags",  0, 0, "OSM",
        "Put OSM tags in object comments");

    if (!write) ms2opt_add_mp_i(opts);// MP group, reading mp files
  }
  if (write) {
    opts.add("mp_id",        1, 0, "MP", "override MP ID");
    opts.add("mp_name",      1, 0, "MP", "override MP Name");
    opts.add("mp_levels",    1, 0, "MP", "set mp data levels (comma-separated integers),"
                                         " default: \"24,22,20,18,17,15\".");
    opts.add("crop_nom",     1, 0, "VMAP2", "Crop map to nomenclature region.");
    opts.add("crop_nom_fi",  1, 0, "VMAP2", "Crop map to Finnish nomenclature region.");
    opts.add("crop_rect",    1, 0, "VMAP2", "Crop map to a rectangle.");
    opts.add("crop_labels",  1, 0, "VMAP2", "By default crop_* commands are not applied to labels. This option can change it. Values: 0 or 1, default: 0.");

    opts.add("update_labels",1, 0, "VMAP2",
      "Update labels (0|1): find labels for each object, add missing labels, "
      "remove labels without objects, update label type, name, and reference. "
      "Values: 0 or 1, default: 0.");

    opts.add("label_names", 1, 0, "VMAP2",
      "When updating labels, get names from labels, not from objects. "
      "Useful for updating map from a source (e.g. gpx) where names are missing."
      "Values: 0 or 1, default: 0.");

    opts.add("keep_labels",  1, 0, "VMAP2", "Keep old labels. Values: 0 or 1, default: 0.");
    opts.add("replace_tag",   1, 0, "VMAP2", "Replace objects with a given tag.");
    opts.add("replace_type",  1, 0, "VMAP2", "Replace objects with a given type.");
    opts.add("replace_types", 1, 0, "VMAP2", "Replace objects with all types which exist in input files. "
      "Values: 0 or 1, default: 0.");
    opts.add("fix_rounding", 1, 0,
       "VMAP2", "Fix rounding errors using information from the output file."
                " This should be used when editing map in fig format and then returning"
                " changes to other format. Then untached points are detected and rounding"
                " errors fixed.");
    opts.add("rounding_acc", 1, 0, "VMAP2",
       "accuracy for fixing rounding errors (m). "
       "Fig accuracy is 1/450 cm, 2.2 m for 1:100'000 maps. Default: 5");
    opts.add("join_lines",0, 0, "VMAP2", "join similar line objects (same type, name, comment, tags)");
    opts.add("join_lines_d",1, 0, "VMAP2", "Max distance for joining lines in meters. Default 5.");
    opts.add("join_lines_a",1, 0, "VMAP2", "Max angle for joining lines in degrees. Default 45.");
    opts.add("filter_pts",0, 0,   "VMAP2", "Reduce number of points in lines and polygons.");
    opts.add("filter_pts_d",1, 0, "VMAP2", "Accuracy for point filtering in meters. Default: 10");
    if (!read) ms2opt_add_mp_o(opts);  // MP group, writing mp files
  }
  if (read && write)  ms2opt_add_mp_io(opts);
}

/****************************************************************************/
void
vmap2_import(const std::string & ifile, const VMap2types & types,
             VMap2 & vmap2, const Opt & opts){

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
  else if (file_ext_check(ifile, ".osm")){
    osm_to_vmap2(ifile, vmap2, opts);
  }
  else if (file_ext_check(ifile, ".gpx")){
    gpx_to_vmap2(ifile, vmap2, opts);
  }
  else throw Err() << "unsupported file extension: " << ifile;
}

void vmap2_import(const std::vector<std::string> & ifiles,
                  const VMap2types & types, VMap2 & vmap2, const Opt & opts){
  for (const auto & ifile:ifiles) vmap2_import(ifile, types, vmap2, opts);
}

/****************************************************************************/

void
vmap2_export(VMap2 & vmap2, const VMap2types & types,
             const std::string & ofile, const Opt & opts){

  bool update_labels = opts.get("update_labels", false);
  bool label_names = opts.get("label_names", false);
  bool keep_labels = opts.get("keep_labels", false);
  std::string replace_tag = opts.get("replace_tag");
  uint32_t replace_type = VMap2obj::make_type(opts.get("replace_type", "none"));
  bool replace_types = opts.get("replace_types", false);
  bool fix_rounding = opts.get("fix_rounding", false);
  double rounding_acc = opts.get("rounding_acc", 5); // m
  bool join_lines = opts.exists("join_lines");
  double join_lines_d = opts.get("join_lines_d", 5); // m
  double join_lines_a = opts.get("join_lines_a", 45); // deg
  bool filter_pts = opts.exists("filter_pts");
  double filter_pts_d = opts.get("filter_pts_d", 10); // m
  std::string crop_nom = opts.get("crop_nom");
  std::string crop_nom_fi = opts.get("crop_nom_fi");
  std::string crop_rect = opts.get("crop_rect");
  bool crop_labels = opts.get("crop_labels", false);

  opts.check_conflict({"replace_tag", "replace_type", "replace_types"});

  // Merge with old data if needed
  if ((keep_labels || replace_tag!="" || fix_rounding ||
      replace_types || VMap2obj::get_class(replace_type)!=VMAP2_NONE) &&
      file_exists(ofile)){

    VMap2 vmap2o;
    vmap2_import({ofile}, types, vmap2o, opts);

    if (keep_labels)
      do_keep_labels(vmap2o, vmap2);

    if (replace_tag!="")
      do_replace_tag(vmap2o, vmap2, replace_tag);

    if (replace_types)
      do_replace_types(vmap2o, vmap2);

    if (VMap2obj::get_class(replace_type)!=VMAP2_NONE)
      do_replace_type(vmap2o, vmap2, replace_type);

    if (fix_rounding)
      do_fix_rounding(vmap2o, vmap2, rounding_acc);
  }

  if (join_lines)
      do_join_lines(vmap2, join_lines_d, join_lines_a);

  if (filter_pts)
      do_filter_pts(vmap2, filter_pts_d);

  if (update_labels)
      do_update_labels(vmap2, types, label_names);

  if (crop_nom.size())
     do_crop_rect(vmap2, nom_to_wgs(crop_nom), crop_labels);

  if (crop_nom.size())
     do_crop_nom_fi(vmap2, crop_nom_fi, crop_labels);

  if (crop_rect.size())
    do_crop_rect(vmap2, dRect(crop_rect), crop_labels);

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
  else if (file_ext_check(ofile, ".gpx")){
    vmap2_to_gpx(vmap2, ofile, opts);
  }
  else throw Err() << "unsupported file extension: " << ofile;
}

/****************************************************************************/
