#include <fstream>
#include "filename/filename.h"
#include "read_words/read_words.h"
#include "geo_data/geo_io.h"
#include "geo_data/conv_geo.h"
#include "geo_nom/geo_nom.h"

#include "mapdb.h"
#include "mapdb_storage_bdb.h"
#include "mapdb_storage_mem.h"
#include "mapdb_storage_io.h"

void
MapDB::load(const std::string & fname){
  *this = MapDB(); // clear

  path = file_get_prefix(fname); // file path

  std::ifstream ff(fname);
  if (!ff) throw Err() << "can't open file: " << fname;

  int line_num[2] = {0,0}; // line counter for read_words
  read_words_defs defs;    // variables

  while (1){
    auto vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;


    try{
      // definitions
      defs.apply(vs);

      // define <key> <value> -- define a variable
      if (vs[0] == "define") {
        if (vs.size()!=3) throw Err() << "define: arguments expected: <key> <value>";
        defs.define(vs[1], vs[2]);
        continue;
      }

      // map_name <name> -- set map name
      if (vs[0] == "map_name") {
        if (vs.size()!=2) throw Err() << "map_name: argument expected: <name>";
        name = vs[1];
        continue;
      }

      // border_file <file name> -- read border from a track file
      if (vs[0] == "border_file") {
        if (vs.size()!=2) throw Err() << "border_file: argument expected: <file name>";
        GeoData dat;
        read_geo(path + "/" + vs[1], dat);
        if (dat.trks.size()!=1)
          throw Err() << "border_file: a single track expected in the file: " << vs[1];
        border=*dat.trks.begin();
        continue;
      }

      // page_nom <file name> -- page with a soviet map name
      if (vs[0] == "page_nom") {
        if (vs.size()!=2 && vs.size()!=3)
          throw Err() << "page_nom: arguments expected: <file name> [<name>]";
        VmapPage p;
        p.file = vs[1];
        std::string name = vs.size()==3 ? vs[2] : file_get_basename(p.file);
        nom_scale_t sc;
        p.bbox = nom_to_range(name, sc, true);
        // convert bbox to WGS84
        ConvGeo cnv("SU_LL");
        p.bbox=cnv.frw_acc(p.bbox, 1/3600.0);
        pages.emplace(name, p);
        continue;
      }

      // page_box <file name> <bbox> -- rectangular page in WGS84 coordinates
      if (vs[0] == "page_box") {
        if (vs.size()!=3 && vs.size()!=4)
          throw Err() << "page_nom: arguments expected: <file name> <bbox> [<name>]";
        VmapPage p;
        p.file = vs[1];
        std::string name = vs.size()==4 ? vs[3]: file_get_basename(p.file);
        p.bbox = str_to_type<dRect>(vs[2]);
        pages.emplace(name, p);
        continue;
      }

      // render_conf <file name> -- render configuration file
      if (vs[0] == "render_conf") {
        if (vs.size()!=2) throw Err() << "render_conf: argument expected: <file name>";
        render_conf = vs[1];
        continue;
      }

      // types_conf <file name> -- types configuration file
      if (vs[0] == "types_conf") {
        if (vs.size()!=2) throw Err() << "types_conf: argument expected: <file name>";
        types_conf = vs[1];
        continue;
      }

      throw Err() << "unknown command: " << vs[0];
    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }

  }
}

void
MapDB::list_pages(const bool lng) const {
  for (const auto & p: pages){
    if (lng)
      std::cout << p.first << "\t" << p.second.file << "\t" << p.second.bbox << "\n";
    else
      std::cout << p.first << "\n";
  }
}

void
MapDB::import_page(const std::string & page, const std::string & file){
  // Find page
  auto pp = pages.find(page);
  if (pp == pages.end()) throw Err() << "Unknown page: " << page;
  auto & p = pp->second;

//  auto storage = mapdb_load(p.file,  )


  // load external file

  // save page if needed
}

void
MapDB::export_page(const std::string & page, const std::string & file){
  // Find page
  auto pp = pages.find(page);
  if (pp == pages.end()) throw Err() << "Unknown page: " << page;
  auto & p = pp->second;

  // Save file
}
