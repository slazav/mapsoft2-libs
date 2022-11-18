#include <fstream>
#include "filename/filename.h"
#include "read_words/read_words.h"
#include "vmap2types.h"
#include "vmap2obj.h"

void
VMap2types::load(const std::string & fname){
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
        if (vs.size()!=3) throw Err() << "define: arguments expected: <key> <value>";
        defs.define(vs[1], vs[2]);
        continue;
      }

      // type <type> -- add description of an object type
      if (vs[0] == "type") {
        if (vs.size()!=2) throw Err() << "type: argument expected: <type>";
        type = VMap2obj::make_type(vs[1]);
        emplace(type, VMap2type());
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
          if (vs.size()!=3) throw Err() << "+ name: argument expected: <name>";
          o->second.name = vs[2];
          continue;
        }
        if (vs[1] == "comm"){
          if (vs.size()!=3) throw Err() << "+ comm: argument expected: <description>";
          o->second.comm = vs[2];
          continue;
        }
        if (vs[1] == "fig_mask"){
          if (vs.size()!=3) throw Err() << "+ fig_mask: argument expected: <fig mask>";
          o->second.fig_mask = vs[2];
          continue;
        }
        if (vs[1] == "fig_pic"){
          if (vs.size()!=3) throw Err() << "+ fig_pic: argument expected: <fig path>";
          o->second.fig_pic = vs[2];
          continue;
        }
        if (vs[1] == "mp_start"){
          if (vs.size()!=3) throw Err() << "+ mp_start: argument expected: <start level>";
          o->second.mp_start = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "mp_end"){
          if (vs.size()!=3) throw Err() << "+ mp_end: arguments expected: <end level>";
          o->second.mp_end = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "label_type"){
          if (vs.size()!=3) throw Err() << "+ label_type: argument expected: <integer type>";
          o->second.label_type = VMap2obj::make_type(VMAP2_TEXT, str_to_type<int>(vs[2]));
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
