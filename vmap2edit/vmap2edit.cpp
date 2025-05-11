#include <algorithm>
#include <deque>

#include "vmap2/vmap2types.h"
#include "filename/filename.h"
#include "geom/line_rectcrop.h"
#include "geo_data/conv_geo.h"
#include "geo_data/geo_utils.h"
#include "geo_nom/geo_nom_fi.h"

#include "vmap2edit.h"


void
vmap2edit_action(VMap2 & vmap, const std::vector<std::string> & cmd, const VMap2cond & conds){
  if (cmd.size()==0) return;
  auto action = VMap2action::get_action(vmap, cmd);
  if (action->do_none) {
    if (conds.size()>0) throw Err() << "action does not require any object conditions";
    return;
  }

  // Loop through VMap2 objects:
  vmap.iter_start();
  size_t cnt = 0;
  while (!vmap.iter_end()){
    auto p = vmap.iter_get_next();
    auto id = p.first;
    auto & o = p.second;
    if (conds.eval(o)) {
      cnt += action->process_object(id, o) ? 1:0;
      if (action->do_once) break;
    }
  }
  std::cerr << cmd[0] << ": " << cnt << " objects processed\n";
}


void
vmap2edit(VMap2 & vmap, const std::string & fname){

  // Read command list from a file
  std::ifstream ff(fname);
  if (!ff) throw Err() << "can't open file: " << fname;
  auto path = file_get_prefix(fname);

  int line_num[2] = {0,0}; // line counter for read_words
  read_words_defs defs;

  VMap2cond conds;
  while (1){
    auto vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;

    try{

/*
      // apply definitions
      defs.apply(vs);

      // define <key> <value> -- define a variable
      if (vs[0] == "define") {
        if (vs.size()!=3) throw Err() << "define: arguments expected: <key> <value>";
        defs.define(vs[1], vs[2]);
        continue;
      }
      // define_if_undef <key> <value> -- define a variable if it is not defined
      if (vs[0] == "define_if_undef") {
        if (vs.size()!=3) throw Err() << "define_if_undef: arguments expected: <key> <value>";
        if (defs.count(vs[1])==0) defs.define(vs[1], vs[2]);
        continue;
      }
*/

      // if <cond> then [<cmd>] -- start condition
      if (vs[0] == "if") {
        auto th = std::find(vs.begin()+1, vs.end(), "then");
        if (th == vs.end()){
          conds.emplace_back(vs.begin()+1, vs.end());
        }
        else {
          conds.emplace_back(vs.begin()+1, th);
          if (th+1 == vs.end()) throw Err() << "empty COMMAND after THEN";
          vmap2edit_action(vmap, std::vector<std::string>(th+1, vs.end()), conds);
          conds.pop_back();
        }
        continue;
      }

      // endif
      if (vs[0] == "endif") {
        if (conds.size()==0) throw Err() << "unexpected ENDIF";
        conds.pop_back();
        continue;
      }

      // everything else is a command
      vmap2edit_action(vmap, std::vector<std::string>(vs.begin(), vs.end()), conds);
    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }
  }
  if (conds.size()>0) throw Err() << fname << ":" << line_num[0] << ": " << "missing ENDIF";
}

