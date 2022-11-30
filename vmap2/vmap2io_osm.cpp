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
              std::list<std::pair<Opt, uint32_t> > & osm_conf,
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
      if (vs.size()!=2) throw Err() << "tags - type pair expected";

      // fail if objects can not be parsed
      osm_conf.emplace_back(
         Opt(vs[0]), VMap2obj::make_type(vs[1]));

    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }
  }
}


// get points of OSM way object:
dLine get_way_points(const OSMXML & data_in, const OSMXML::OSM_Way & way){
  dLine ret;
  for (auto const i:way.nodes){
    if (data_in.nodes.count(i)==0)
      throw Err() << "OSM node does not exist: " << i;
    ret.push_back(data_in.nodes.find(i)->second);
  }
  return ret;
}

// convert OSM to vmap
void
osm_to_vmap2(const std::string & fname, VMap2 & data, const Opt & opts){

  // read configuration
  std::string conf = opts.get("osm_conf","");
  if (conf=="")
    throw Err() << "empty configuration file, use --osm_conf option";

  std::list<std::pair<Opt, uint32_t> > osm_conf;
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
      bool match = true;
      // match tags
      for (auto const & o:conf.first){
        if (!e.second.exists(o.first) ||
           (o.second!="*" &&
            e.second.get(o.first, "")!=o.second)) match=false;
      }
      if (!match) continue;

      // we can't convert point to lines or areas:
      auto cl = VMap2obj::get_class(conf.second);
      if (cl==VMAP2_LINE || cl==VMAP2_POLYGON) continue;
      if (cl==VMAP2_NONE) {done=true; break;}

      if (cl!=VMAP2_POINT) throw Err()
        << "Bad object type in configuration file: "
        << VMap2obj::print_type(conf.second);

      // make object
      VMap2obj obj(conf.second);
      obj.name = e.second.get("name", "");
      if (data_in.nodes.count(e.first)==0)
        throw Err() << "OSM node does not exist: " << e.first;
      obj.set_coords(data_in.nodes.find(e.first)->second);
      data.add(obj);
      done=true; break;
    }
    if (!done) std::cerr
      << "osm object doen not match any rule:\n"
      << "node " << e.first << ": " << e.second << "\n";
  }

  // convert OSM ways
  for (auto const & e:data_in.ways){
    bool done=false;
    for (auto const & conf:osm_conf){
      bool match = true;
      // match tags
      for (auto const & o:conf.first){
        if (!e.second.exists(o.first) ||
           (o.second!="*" &&
            e.second.get(o.first, "")!=o.second)) match=false;
      }
      if (!match) continue;
      auto cl = VMap2obj::get_class(conf.second);
      if (cl==VMAP2_NONE) {done=true; break;}

      // make object
      VMap2obj obj(conf.second);
      obj.name = e.second.get("name", "");

      // convert coordinates
      dLine pts = get_way_points(data_in, e.second);
      switch (cl){
        case VMAP2_POINT: 
          obj.set_coords(pts.bbox().cnt());
          break;
        case VMAP2_LINE: 
        case VMAP2_POLYGON:
          obj.set_coords(pts);
          break;
        default:
          if (cl!=VMAP2_POINT) throw Err()
              << "Bad object type in configuration file: "
              << VMap2obj::print_type(conf.second);
      }
      data.add(obj);
      done=true; break;
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
      bool match = true;
      // match tags
      for (auto const & o:conf.first){
        if (!e.second.exists(o.first) ||
           (o.second!="*" &&
            e.second.get(o.first, "")!=o.second)) match=false;
      }
      if (!match) continue;
      auto cl = VMap2obj::get_class(conf.second);
      if (cl==VMAP2_NONE) {done=true; break;}

      // make object
      VMap2obj obj(conf.second);
      obj.name = e.second.get("name", "");

      // extract coordinates
      dMultiLine pts;
      for (const auto & m:e.second.members){
        if (m.role != "outer" && m.role != "inner") continue;
        if (data_in.ways.count(m.ref)==0) continue;
        pts.push_back(get_way_points(data_in,
          data_in.ways.find(m.ref)->second));
      }
      if (pts.empty()) continue;

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
          if (cl!=VMAP2_POINT) throw Err()
              << "Bad object type in configuration file: "
              << VMap2obj::print_type(conf.second);
      }
      data.add(obj);
      done=true; break;
    }
    if (!done) std::cerr
      << "osm object doen not match any rule:\n"
      << "rel " << e.first << ": " << e.second << "\n";
  }


}
