#include <fstream>
#include "filename/filename.h"
#include "read_words/read_words.h"
#include "geo_data/geo_io.h"

#include "mapdb.h"

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
        if (vs.size()!=3) throw Err() << "define: 2 arguments expected: <key> <value>";
        defs.define(vs[1], vs[2]);
        continue;
      }

      // map_name <name> -- set map name
      if (vs[0] == "map_name") {
        if (vs.size()!=2) throw Err() << "map_name: 1 argument expected: <name>";
        name = vs[1];
        continue;
      }

      // border_file <file name> -- read border from a track file
      if (vs[0] == "border_file") {
        if (vs.size()!=2) throw Err() << "border_file: 1 argument expected: <file name>";
        GeoData dat;
        read_geo(vs[1], dat);
        if (dat.trks.size()!=1)
          throw Err() << "border_file: a single track expected in the file: " << vs[1];
        border=*dat.trks.begin();
        continue;
      }

      // page_nom <file name> -- page with a soviet map name
      if (vs[0] == "page_nom") {
        if (vs.size()!=2) throw Err() << "page_nom: 1 argument expected: <file name>";
        VmapPage p;
        p.file = vs[0];
//        p.name = vs[0];
//        p.bbox = 
// file_get_basename()
//...
        continue;
      }

      // render_conf <file name> -- render configuration file
      if (vs[0] == "render_conf") {
        if (vs.size()!=2) throw Err() << "render_conf: 1 argument expected: <file name>";
        render_conf = vs[1];
        continue;
      }

      // types_conf <file name> -- types configuration file
      if (vs[0] == "types_conf") {
        if (vs.size()!=2) throw Err() << "types_conf: 1 argument expected: <file name>";
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
