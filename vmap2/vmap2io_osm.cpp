#include <fstream>
#include <deque>
#include "filename/filename.h"
#include "read_words/read_words.h"
#include "osmxml/osmxml.h"
#include "geo_data/geo_utils.h" // geo_dist_2d
#include "vmap2.h"
#include "vmap2obj.h"

void
load_osm_conf(const std::string & fname,
              std::list<std::pair<Opt, std::vector<uint32_t> > > & osm_conf,
              read_words_defs & defs){

  // read configuration
  if (fname=="") throw Err() << "empty OSM configuration file";
  auto path = file_get_prefix(fname);

  std::ifstream ff(fname);
  if (!ff) throw Err() << "can't open OSM configuration file: " << fname;

  int line_num[2] = {0,0}; // line counter for read_words
  std::deque<bool> ifs;  // for if/endif commands

  while (1){
    auto vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;

    try{
      // definitions
      defs.apply(vs);

      // include command
      if (vs[0] == "include"){
        if (vs.size()!=2) throw Err() << "include: filename expected";
        auto fn = vs[1];
        // should not happend, but lets check before accessing fn[0]:
        if (fn.size() == 0) throw Err() << "include: empty filename";

        if (fn[0] != '/') fn = path + fn;
        load_osm_conf(fn, osm_conf, defs);
        continue;
      }

      // endif command
      if (vs[0] == "endif"){
        if (ifs.size()<1) throw Err() << "unexpected endif command";
        ifs.pop_back();
        continue;
      }
      // else command
      if (vs[0] == "else"){
        if (ifs.size()<1) throw Err() << "unexpected else command";
        ifs.back() = !ifs.back();
        continue;
      }
      // if command
      if (vs[0] == "if"){
        if (vs.size() == 4 && vs[2] == "=="){
          ifs.push_back(vs[1] == vs[3]);
        }
        else if (vs.size() == 4 && vs[2] == "!="){
          ifs.push_back(vs[1] != vs[3]);
        }
        else
          throw Err() << "wrong if syntax";
        continue;
      }

      // check if conditions
      bool skip = false;
      for (auto const & c:ifs)
        if (c == false) {skip = true; break;}
      if (skip) continue;

      // define <key> <value> -- define a variable
      if (vs[0] == "define") {
        if (vs.size()!=3) throw Err() << "define: arguments expected: <key> <value>";
        defs.define(vs[1], vs[2]);
        continue;
      }

      // All other lines are "Opt -> VMap2type" pairs
      if (vs.size()<2) throw Err() << "argument expected: <tags> <type1> ...";

      std::vector<uint32_t> types;
      for (int i=1; i<vs.size(); i++)
        types.push_back(VMap2obj::make_type(vs[i]));

      osm_conf.emplace_back(Opt(vs[0]), types);

    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }
  }
}


// Match object tags using mask
// Mask can contain <tag>=<value> or <tag>=*
bool match_tags(const Opt & obj, const Opt & mask){
  for (auto const & m:mask){
    if (!obj.exists(m.first) ||
       (m.second!="*" &&
        obj.get(m.first, "")!=m.second)) return false;
  }
  return true;
}


// convert OSM to vmap
void
osm_to_vmap2(const std::string & fname, VMap2 & data, const Opt & opts){

  // read configuration
  std::string conf = opts.get("osm_conf","");
  if (conf=="")
    throw Err() << "empty configuration file, use --osm_conf option";

  std::list<std::pair<Opt, std::vector<uint32_t> > > osm_conf;
  read_words_defs defs;
  load_osm_conf(conf, osm_conf, defs);

  //read OSM data
  OSMXML data_in;
  read_osmxml(fname, data_in, opts);

  // For each OSM point go through configuration list
  // until it can be converted. Tags should match, "name"="*"
  // matches any tag value. Point objects can not be converted
  // to lines and areas, areas and lines can be converted to points
  // "none" type is used to skip the object.

  // convert OSM points
  for (auto const & e:data_in.points){
    bool done=false;
    for (auto const & conf:osm_conf){
      if (!match_tags(e.second, conf.first)) continue;

      // extract coordinates
      dPoint pt = data_in.get_node_coords(e.first);

      for (const auto t:conf.second){
        // we can't convert point to lines or areas:
        auto cl = VMap2obj::get_class(t);
        if (cl==VMAP2_LINE || cl==VMAP2_POLYGON) continue;
        if (cl==VMAP2_NONE) {done=true; continue;}
        if (cl!=VMAP2_POINT) throw Err()
          << "Bad object type in configuration file: "
          << VMap2obj::print_type(t);

        // make object
        VMap2obj obj(t);
        obj.name = e.second.get("name", "");
        obj.set_coords(pt);
        data.add(obj);
        done=true;
      }
      if (done) break;
    }
    if (!done) std::cerr
      << "osm object doen not match any rule:\n"
      << "node " << e.first << ": " << e.second << "\n";
  }

  // convert OSM ways
  for (auto const & e:data_in.ways){
    bool done=false;
    for (auto const & conf:osm_conf){
      if (!match_tags(e.second, conf.first)) continue;

      // extract coordinates
      dLine pts = data_in.get_way_coords(e.second);

      for (const auto t:conf.second){
        auto cl = VMap2obj::get_class(t);
        if (cl==VMAP2_NONE) {done=true; continue;}

        // make object
        VMap2obj obj(t);
        obj.name = e.second.get("name", "");

        // convert coordinates
        switch (cl){
          case VMAP2_POINT: 
            obj.set_coords(pts.bbox().cnt());
            break;
          case VMAP2_LINE: 
          case VMAP2_POLYGON:
            obj.set_coords(pts);
            break;
          default:
            throw Err()
                << "Bad object type in configuration file: "
                << VMap2obj::print_type(t);
        }
        data.add(obj);
        done=true;
      }
      if (done) break;
    }
    if (!done) std::cerr
      << "osm object doen not match any rule:\n"
      << "way " << e.first << ": " << e.second << "\n";
  }

  // Convert OSM multipolygon relations
  // (with outer/inner member roles)
  for (auto const & e:data_in.relations){
    bool done=false;
    for (auto const & conf:osm_conf){
      if (!match_tags(e.second, conf.first)) continue;

      // extract coordinates
      dMultiLine pts;
      for (const auto & m:e.second.members){
        if (m.role != "outer" && m.role != "inner") continue;
        if (data_in.ways.count(m.ref)==0) continue;
        pts.push_back(data_in.get_way_coords(
           data_in.ways.find(m.ref)->second));
      }
      if (pts.empty()) continue;

      for (const auto t:conf.second){
        auto cl = VMap2obj::get_class(t);
        if (cl==VMAP2_NONE) {done=true; continue;}

        // make object
        VMap2obj obj(t);
        obj.name = e.second.get("name", "");

        // fill coordinates
        switch (cl){
          case VMAP2_POINT:
            obj.set_coords(pts.bbox().cnt());
            break;
          case VMAP2_LINE:
          case VMAP2_POLYGON:
            obj.set_coords(pts);
            break;
          default:
            throw Err()
                << "Bad object type in configuration file: "
                << VMap2obj::print_type(t);
        }
        data.add(obj);
        done=true;
      }
      if (done) break;
    }
    if (!done) std::cerr
      << "osm object doen not match any rule:\n"
      << "rel " << e.first << ": " << e.second << "\n";
  }


}
