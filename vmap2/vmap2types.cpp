#include <fstream>
#include <deque>
#include "filename/filename.h"
#include "vmap2types.h"
#include "vmap2obj.h"

void
ms2opt_add_vmap2t(GetOptSet & opts){
  const char *g = "VMAP2";
  opts.add("types",  1, 't', g, "File with type information. Default: /usr/share/mapsoft2/types.cfg");
  opts.add("define", 1, 'D', g, "Define variables for type information file.");
}

VMap2types::VMap2types(const Opt & o){
  std::string file = o.get("types", "/usr/share/mapsoft2/types.cfg");
  if (file == "") return;
  read_words_defs defs(o.get("define", Opt()));
  load(file, defs);
}

void
VMap2types::load(const std::string & fname, read_words_defs & defs){

  std::ifstream ff(fname);
  if (!ff) throw Err() << "can't open file: " << fname;
  auto path = file_get_prefix(fname);

  int line_num[2] = {0,0}; // line counter for read_words
  int type = -1;           // current type
  std::deque<bool> ifs;  // for if/endif commands

  while (1){
    auto vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;

    try{
      if (read_words_stdcmds(vs, defs, ifs)){
        type=-1; continue; }

      // include command
      if (vs[0] == "include"){
        if (vs.size()!=2) throw Err() << "include: filename expected";
        auto fn = vs[1];
        // should not happend, but lets check before accessing fn[0]:
        if (fn.size() == 0) throw Err() << "include: empty filename";

        if (fn[0] != '/') fn = path + fn;
        load(fn, defs);
        type=-1;
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
          read_fig(path + vs[2], o->second.fig_pic);
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
          if (vs.size()!=3) throw Err() << "+ label_type: argument expected: <type number or -1>";
          o->second.label_type = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "label_def_scale"){
          if (vs.size()!=3) throw Err() << "+ label_def_scale: argument expected: <scale>";
          o->second.label_def_scale = str_to_type<float>(vs[2]);
          continue;
        }
        if (vs[1] == "label_mkpt"){
          if (vs.size()!=3) throw Err() << "+ label_mkpt: argument expected: <type number or -1>";
          o->second.label_mkpt = str_to_type<int>(vs[2]);
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
