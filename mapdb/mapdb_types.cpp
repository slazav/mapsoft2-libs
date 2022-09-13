#include <fstream>
#include "filename/filename.h"
#include "read_words/read_words.h"
#include "mapdb_types.h"

#define POINT_MASK  0x000000
#define LINE_MASK   0x100000
#define AREA_MASK   0x200000
#define TEXT_MASK   0x300000

void
MapDBTypeMap::load(const std::string & fname){
  clear();

  std::ifstream ff(fname);
  if (!ff) throw Err() << "can't open file: " << fname;

  int line_num[2] = {0,0}; // line counter for read_words
  read_words_defs defs;    // variables
  int type = -1;           // current type

  while (1){
    auto vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;


    try{
      // definitions
      defs.apply(vs);

      // define <key> <value> -- define a variable
      if (vs[0] == "define") {
        type=-1;
        if (vs.size()!=3) throw Err() << "define: 2 arguments expected: <key> <value>";
        defs.define(vs[1], vs[2]);
        continue;
      }

      // point <type> -- define a point object
      if (vs[0] == "point") {
        if (vs.size()!=2) throw Err() << "point: 1 argument expected: <type>";
        type = POINT_MASK + 0xFFFF * str_to_type<int>(vs[1]);
        emplace(type, MapBDTypeInfo());
        continue;
      }

      // line <type> -- define a line object
      if (vs[0] == "line") {
        if (vs.size()!=2) throw Err() << "line: 1 argument expected: <type>";
        type = LINE_MASK + 0xFFFF * str_to_type<int>(vs[1]);
        emplace(type, MapBDTypeInfo());
        continue;
      }

      // area <type> -- define an area object
      if (vs[0] == "area") {
        if (vs.size()!=2) throw Err() << "area: 1 argument expected: <type>";
        int type = AREA_MASK + 0xFFFF * str_to_type<int>(vs[1]);
        emplace(type, MapBDTypeInfo());
        continue;
      }

      // + <feature> <args> ... -- define feature of a point, a line or an area object
      if (vs[0] == "+") {
        if (type == -1)  throw Err() << "+ expected after point, line, or area command";
        auto o = find(type);
        if (o==end())    throw Err() << "can't find object type" << type;
        if (vs.size()<2) throw Err() << "+: at least one argument expected";

        // features
        if (vs[1] == "name"){
          if (vs.size()!=3) throw Err() << "+ name: 1 argument expected: <name>";
          o->second.name = vs[2];
          continue;
        }
        if (vs[1] == "comm"){
          if (vs.size()!=3) throw Err() << "+ comm: 1 argument expected: <description>";
          o->second.comm = vs[2];
          continue;
        }
        if (vs[1] == "fig_mask"){
          if (vs.size()!=3) throw Err() << "+ fig_mask: 1 argument expected: <fig mask>";
          o->second.fig_mask = vs[2];
          continue;
        }
        if (vs[1] == "mp_levels"){
          if (vs.size()!=4) throw Err() << "+ mp_levels: 2 arguments expected: <start> <end>";
          o->second.mp_sl = str_to_type<int>(vs[2]);
          o->second.mp_el = str_to_type<int>(vs[3]);
          continue;
        }
        if (vs[1] == "text_type"){
          if (vs.size()!=3) throw Err() << "+ text_type: 1 argument expected: <integer type>";
          o->second.text_type = TEXT_MASK + 0xFFFF * str_to_type<int>(vs[2]);
          continue;
        }


        throw Err() << "unknown feature: " << vs[1];
      }
      throw Err() << "unknown command: " << vs[0];

    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }

  }
}
