#include "vmap2io.h"
#include "filename/filename.h"
#include "geo_mkref/geo_mkref.h"
#include "fig_geo/fig_geo.h"
#include "fig_opt/fig_opt.h"

/****************************************************************************/

void
ms2opt_add_vmap2io(GetOptSet & opts){
  const char *g = "VMAP2IO";
  opts.add("out",          1, 'o', g, "Output file.");
  opts.add("types",        1, 't', g, "File with type information.");
  opts.add("headers",      1, 0, g, "Use non-object information from output "
    "file (mp headers, fig reference and objects, etc.), 1 (default) or 0");
  opts.add("old_objects",  1, 0, g, "Keep old objects, 0 (default) or 1");
  opts.add("old_labels",   1, 0, g, "Keep old labels,  0 (default) or 1");
  opts.add("new_objects",  1, 0, g, "Copy new objects, 1 (default) or 0");
  opts.add("new_labels",   1, 0, g, "Copy new labels,  1 (default) or 0");

  opts.add("mp_ip",        1, 0, g, "override MP ID");
  opts.add("mp_name",      1, 0, g, "override MP Name");

  ms2opt_add_mp(opts);  // MP group, reading/writing mp files
  ms2opt_add_fig(opts); // FIG group, reading/writing fig files
  ms2opt_add_geofig_ref(opts);
  ms2opt_add_mkref_opts(opts);
  ms2opt_add_mkref_brd(opts);
}

/****************************************************************************/

void
vmap2_convert(const std::vector<std::string> & ifiles, const Opt & opts){

  // - Read output files (keep native format objects)
  // - Convert to vmap2
  // - Read objects from all input files to another vmap2
  // - Merge data according with options
  // - Save result in the output file (or close database)

  // check output file
  std::string ofile = opts.get("out", "");
  if (ofile == "") throw Err()
    << "vmap2_convert: non-empty output file expected (use -o option)";

  // check input files
  if (ifiles.size()<1) throw Err()
    << "vmap2_convert: at least one input file expected";

  // Read file with type information if it's available
  VMap2types types;
  if (opts.exists("types"))
    types.load(opts.get("types"));

  // options
  bool headers = opts.get("headers", 0);
  bool old_objects = opts.get("old_objects", 0);
  bool old_labels  = opts.get("old_labels", 0);
  bool new_objects = opts.get("new_objects", 1);
  bool new_labels  = opts.get("new_labels", 1);

  // Read output file, convert to vmap2.
  // Keep original format too (we may want to keep mp headers,
  //  fig reference, etc.)

  VMap2 vmap2o;
  VMap vmap;
  MP mp;
  Fig fig;
  if (file_ext_check(ofile, ".vmap2db")){
    vmap2o = VMap2(ofile, true); // open/create database
  }
  if (file_exists(ofile) && (old_labels || old_objects || headers)) {
    if (file_ext_check(ofile, ".vmap2")){
      vmap2o.read(ofile);
    }
    else if (file_ext_check(ofile, ".vmap")){
      vmap = read_vmap(ofile);
      vmap_to_vmap2(vmap, types, vmap2o);
      if (!headers){
        vmap = VMap();
      }
      else {
        vmap.clear(); // remove objects
      }
    }
    else if (file_ext_check(ofile, ".mp")){
      read_mp(ofile, mp, opts);
      mp_to_vmap2(mp, types, vmap2o);
      if (!headers){
        mp = MP();
      }
      else {
        mp.clear(); // remove objects
      }
      if (opts.exists("mp_id"))
        mp.ID = opts.get<int>("mp_id");
      if (opts.exists("mp_name"))
        mp.Name = opts.get("mp_name");
    }
    else if (file_ext_check(ofile, ".fig")){
      read_fig(ofile, fig);
      fig_to_vmap2(fig, types, vmap2o);
      if (!headers){
        fig = Fig();
      }
      else {
        //...
      }
      if (opts.exists("mkref")){
        GeoMap ref = geo_mkref_opts(opts);
        geo_mkref_brd(ref, opts);
        fig_del_ref(fig);
        fig_add_ref(fig, ref, opts);
      }
    }
    else throw Err() << "unsupported file extension: " << ofile;
  }

  // Read all input files to vmap2i
  VMap2 vmap2i;
  any_to_vmap2(ifiles, types, vmap2i, opts);

  // Merge data

  if (!old_labels || !old_objects){ // cleanup
    vmap2o.iter_start();
    while (!vmap2o.iter_end()){
      auto p = vmap2o.iter_get_next();
      auto c = p.second.get_class();
      if (!old_labels  && c == VMAP2_TEXT) vmap2o.del(p.first);
      if (!old_objects && c  < VMAP2_TEXT) vmap2o.del(p.first);
    }
  }
  if (new_labels || new_objects){
    vmap2i.iter_start();
    while (!vmap2i.iter_end()){
      auto p = vmap2i.iter_get_next();
      auto c = p.second.get_class();
      if (new_labels  && c == VMAP2_TEXT) vmap2o.add(p.second);
      if (new_objects && c  < VMAP2_TEXT) vmap2o.add(p.second);
    }
  }

  // Save different formats
  // Similar to vmap2_to_any function, but use
  // existning fig/mp/vmap objects


  if (file_ext_check(ofile, ".vmap2db")){
    return;
  }
  if (file_ext_check(ofile, ".vmap2")){
    vmap2o.write(ofile);
    return;
  }
  if (file_ext_check(ofile, ".vmap")){
    vmap2_to_vmap(vmap2o, types, vmap);
    write_vmap(ofile, vmap);
    return;
  }
  if (file_ext_check(ofile, ".mp")){
    vmap2_to_mp(vmap2o, types, mp);
    write_mp(ofile, mp);
    return;
  }
  else if (file_ext_check(ofile, ".fig")){
    vmap2_to_fig(vmap2o, types, fig);
    write_fig(ofile, fig);
    return;
  }
  else throw Err() << "unsupported file extension: " << ofile;
}

/****************************************************************************/
void any_to_vmap2(const std::vector<std::string> & ifiles, const VMap2types & types,
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
      VMap vmap = read_vmap(ifile);
      vmap_to_vmap2(vmap, types, vmap2);
    }
    else if (file_ext_check(ifile, ".mp")){
      MP mp;
      read_mp(ifile, mp, opts);
      mp_to_vmap2(mp, types, vmap2);
    }
    else if (file_ext_check(ifile, ".fig")){
      Fig fig;
      read_fig(ifile, fig, opts);
      fig_to_vmap2(fig, types, vmap2);
    }
    else throw Err() << "unsupported file extension: " << ifile;
  }
}

/****************************************************************************/

void
vmap2_to_any(VMap2 & vmap2, const VMap2types & types,
             const std::string & ofile, const Opt & opts){
  if (file_ext_check(ofile, ".vmap2")){
    vmap2.write(ofile);
    return;
  }
  if (file_ext_check(ofile, ".vmap")){
    VMap vmap;
    vmap2_to_vmap(vmap2, types, vmap);
    write_vmap(ofile, vmap);
    return;
  }
  if (file_ext_check(ofile, ".mp")){
    MP mp;
    if (opts.exists("mp_id"))
      mp.ID = opts.get<int>("mp_id");
    if (opts.exists("mp_name"))
      mp.Name = opts.get("mp_name");
    vmap2_to_mp(vmap2, types, mp);
    write_mp(ofile, mp);
    return;
  }
  else if (file_ext_check(ofile, ".fig")){
    Fig fig;

    if (opts.exists("mkref")){
      GeoMap ref = geo_mkref_opts(opts);
      geo_mkref_brd(ref, opts);
      fig_del_ref(fig);
      fig_add_ref(fig, ref, opts);
    }

    vmap2_to_fig(vmap2, types, fig);
    write_fig(ofile, fig);
    return;
  }
  else throw Err() << "unsupported file extension: " << ofile;
}

/****************************************************************************/
