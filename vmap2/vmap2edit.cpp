#include <string>
#include <fstream>
#include <algorithm>
#include <deque>
#include "filename/filename.h"
#include "vmap2.h"
#include "vmap2types.h"
#include "vmap2obj.h"

/***********************************************************/

// Calculate conditions for an object, return true if all are valiid
bool
calc_cond(const std::vector<std::string> & cond, const VMap2obj & o){
  for (const auto & c:cond){

    // type conditions

    if (c == "type==point"){
      if (o.get_class() != VMAP2_POINT) return false;
      else continue;
    }

    if (c == "type!=point"){
      if (o.get_class() == VMAP2_POINT) return false;
      else continue;
    }

    if (c == "type==line"){
      if (o.get_class() != VMAP2_LINE) return false;
      else continue;
    }

    if (c == "type!=line"){
      if (o.get_class() == VMAP2_LINE) return false;
      else continue;
    }

    if (c == "type==area"){
      if (o.get_class() != VMAP2_POLYGON) return false;
      else continue;
    }

    if (c == "type!=area"){
      if (o.get_class() == VMAP2_POLYGON) return false;
      else continue;
    }

    if (c == "type==text"){
      if (o.get_class() != VMAP2_TEXT) return false;
      else continue;
    }

    if (c == "type!=text"){
      if (o.get_class() == VMAP2_TEXT) return false;
      else continue;
    }

    if (c.find("type==")==0){
      if (o.type != o.make_type(c.substr(6))) return false;
      else continue;
    }

    if (c.find("type!=")==0){
      if (o.type == o.make_type(c.substr(6))) return false;
      else continue;
    }

    // same with ref_type

    if (c == "ref_type==point"){
      if (o.get_ref_class() != VMAP2_POINT) return false;
      else continue;
    }

    if (c == "ref_type!=point"){
      if (o.get_ref_class() == VMAP2_POINT) return false;
      else continue;
    }

    if (c == "ref_type==line"){
      if (o.get_ref_class() != VMAP2_LINE) return false;
      else continue;
    }

    if (c == "ref_type!=line"){
      if (o.get_ref_class() == VMAP2_LINE) return false;
      else continue;
    }

    if (c == "ref_type==area"){
      if (o.get_ref_class() != VMAP2_POLYGON) return false;
      else continue;
    }

    if (c == "ref_type!=area"){
      if (o.get_ref_class() == VMAP2_POLYGON) return false;
      else continue;
    }

    if (c == "ref_type==text"){
      if (o.get_ref_class() != VMAP2_TEXT) return false;
      else continue;
    }

    if (c == "ref_type!=text"){
      if (o.get_ref_class() == VMAP2_TEXT) return false;
      else continue;
    }

    if (c.find("ref_type==")==0){
      if (o.ref_type != o.make_type(c.substr(10))) return false;
      else continue;
    }

    if (c.find("ref_type!=")==0){
      if (o.ref_type == o.make_type(c.substr(10))) return false;
      else continue;
    }

    throw Err() << "Unknown condition: " << c;
  }
  return true;
}

/***********************************************************/

// Run a command for an object.
// Return true if object has been modified.
void
run_cmd(const std::vector<std::string> & cmd, VMap2 & vmap, uint32_t id, VMap2obj & o){
  if (cmd.size()==0) return;

  if (cmd[0] == "set_type"){
    if (cmd.size()!=2) throw Err()
      << "wrong number of arguments (1 expected): " << cmd[0] << " <type>";
    o.set_type(cmd[1]);
    vmap.put(id, o);
    return;
  }

  if (cmd[0] == "delete"){
    if (cmd.size()!=1) throw Err()
      << "wrong number of arguments (0 expected): " << cmd[0];
    vmap.del(id);
    return;
  }

  throw Err() << "Unknown command: " << cmd[0];
}

/***********************************************************/

typedef enum { FLT_IF, FLT_AND, FLT_OR, FLT_CMD }  st_t;

typedef std::list<std::pair<st_t, std::vector<std::string> > > filter_t;


void
vmap2edit(VMap2 & vmap, const std::string & fname){

  filter_t filter;

  /********************************************************/
  // Read command list from a file

  std::ifstream ff(fname);
  if (!ff) throw Err() << "can't open file: " << fname;
  auto path = file_get_prefix(fname);

  int line_num[2] = {0,0}; // line counter for read_words
  read_words_defs defs;

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
        if (filter.size() && filter.rbegin()->first!=FLT_CMD)
          throw Err() << "IF: should follow a COMMAND";

        auto th = std::find(vs.begin()+1, vs.end(), "then");
        if (th == vs.end()){
          filter.emplace_back(FLT_IF, std::vector<std::string>(vs.begin()+1, vs.end()));
        }
        else {
          filter.emplace_back(FLT_IF, std::vector<std::string>(vs.begin()+1, th));
          if (th+1 == vs.end()) throw Err() << "empty COMMAND after THEN";
          filter.emplace_back(FLT_CMD, std::vector<std::string>(th+1, vs.end()));
        }
        continue;
      }

      // and <cond> -- continue condition
      if (vs[0] == "and") {
        if (filter.size()==0 || filter.rbegin()->first==FLT_CMD)
          throw Err() << "AND: should not follow a COMMAND or be first";
        if (vs.size()<2) throw Err() << "and: condition expected";
        filter.emplace_back(FLT_AND, std::vector<std::string>(vs.begin()+1, vs.end()));
        continue;
      }

      // or <cond> -- continue condition
      if (vs[0] == "or") {
        if (filter.size()==0 || filter.rbegin()->first==FLT_CMD)
          throw Err() << "OR: should not follow a COMMAND or be first";
        if (vs.size()<2) throw Err() << "or: condition expected";
        filter.emplace_back(FLT_OR, std::vector<std::string>(vs.begin()+1, vs.end()));
        continue;
      }

      // everything else is a command
      filter.emplace_back(FLT_CMD, std::vector<std::string>(vs.begin(), vs.end()));
    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }
  }

  /********************************************************/
  // Process objects

  // Loop through VMap2 objects:
  vmap.iter_start();
  while (!vmap.iter_end()){
    auto p = vmap.iter_get_next();
    auto id = p.first;
    auto & o = p.second;

    // loop through the filter
    bool cond = true;
    for (const auto & flt: filter){
      switch (flt.first){
        case FLT_IF:
          cond = calc_cond(flt.second, o);
          break;
        case FLT_AND:
          cond &= calc_cond(flt.second, o);
          break;
        case FLT_OR:
          cond |= calc_cond(flt.second, o);
          break;
        case FLT_CMD:
          if (cond) run_cmd(flt.second, vmap, id, o);
          cond = true;
      }
    }
  }
}

