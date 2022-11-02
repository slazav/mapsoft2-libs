#ifndef MAPDB_H
#define MAPDB_H

#include <string>
#include <memory>
#include <list>
#include <map>
#include <set>

#include "geom/rect.h"
#include "geom/multiline.h"
#include "vmap2/vmap2.h"

/*********************************************************************/
// MapDB -- Main class for vector map.

class MapDB {
private:

  // Component are stored in shared_ptrs and filled only when needed.

  struct VmapPage {
    std::string file;
    dRect bbox; // wgs84 bbox
    std::shared_ptr<MapDBStorage> storage;
  };

  std::map<std::string, VmapPage> pages;
  dMultiLine      border;
  std::string     path;   // path to the main file
  std::string     name;   // map name
  std::string     render_conf;
  std::string     types_conf;

public:

  // load from a file
  void load(const std::string & fname);
  void list_pages(const bool lng=false) const;
  void import_page(const std::string & page, const std::string & file);
  void export_page(const std::string & page, const std::string & file);

  MapDB() {}
  MapDB(const std::string & fname) {load(fname);}


  ///////////////
  /* Import/export */
/*
  public:

  /// Import objects from MP file.
  void import_mp(const std::string & mp_file, const Opt & opts);

  /// Export objects to MP file.
  void export_mp(const std::string & mp_file, const Opt & opts);

  /// Import objects from VMAP file.
  void import_vmap(const std::string & vmap_file, const Opt & opts);

  /// Export objects to VMAP file.
  void export_vmap(const std::string & vmap_file, const Opt & opts);
*/
};

// add option groups:
//   MAPDB_MP_IMP, MAPDB_MP_EXP, MAPDB_VMAP_IMP, MAPDB_VMAP_EXP

/*
#include "getopt/getopt.h"

void ms2opt_add_mapdb_mp_imp(GetOptSet & opts);
void ms2opt_add_mapdb_mp_exp(GetOptSet & opts);

void ms2opt_add_mapdb_vmap_imp(GetOptSet & opts);
void ms2opt_add_mapdb_vmap_exp(GetOptSet & opts);
*/

#endif
