#include "vmap2io.h"
#include "filename/filename.h"
#include "geo_mkref/geo_mkref.h"
#include "fig_geo/fig_geo.h"
#include "fig_opt/fig_opt.h"

/****************************************************************************/

void
ms2opt_add_vmap2i(GetOptSet & opts){
  const char *g = "VMAP";
  ms2opt_add_mp_i(opts);// MP group, reading mp files
  ms2opt_add_fig(opts); // FIG group, reading/writing fig files
  opts.remove("fig_header");
}

void
ms2opt_add_vmap2o(GetOptSet & opts){
  const char *g = "VMAP2";
  opts.add("mp_ip",        1, 0, "MP", "override MP ID");
  opts.add("mp_name",      1, 0, "MP", "override MP Name");
  ms2opt_add_mp_o(opts);  // MP group, writing mp files
  ms2opt_add_fig(opts); // FIG group, reading/writing fig files
  opts.remove("fig_header");
}

void
ms2opt_add_vmap2io(GetOptSet & opts){
  const char *g = "VMAP2";
  opts.add("mp_ip",        1, 0, "MP", "override MP ID");
  opts.add("mp_name",      1, 0, "MP", "override MP Name");
  ms2opt_add_mp_io(opts);  // MP group, reading/writing mp files
  ms2opt_add_fig(opts); // FIG group, reading/writing fig files
  opts.remove("fig_header");
}

void
ms2opt_add_vmap2t(GetOptSet & opts){
  const char *g = "VMAP2T";
  opts.add("types",        1, 't', g, "File with type information.");
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
vmap2_export(VMap2 & vmap2, const VMap2types & types,
             const std::string & ofile, const Opt & opts){

  // TODO: merging with old data

  if (file_ext_check(ofile, ".vmap2db")){
    // If we are saving vmap2db to itself - no need to do anything.
    // If it's a different database, then delete, create it and copy objects.
    if (ofile != vmap2.get_dbname()){
      VMap2::remove_db(ofile);
      VMap2 out(ofile, 1);
      vmap2.iter_start();
      while (!vmap2.iter_end()) out.add(vmap2.iter_get_next().second);
    }
    return;
  }

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

  if (file_ext_check(ofile, ".fig")){
    Fig fig;

    // todo: read reference and non-map objects!

    vmap2_to_fig(vmap2, types, fig);
    write_fig(ofile, fig);
    return;
  }

  throw Err() << "unsupported file extension: " << ofile;
}

/****************************************************************************/
