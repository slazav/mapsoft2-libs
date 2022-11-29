#include <fstream>
#include <deque>
#include "filename/filename.h"
#include "read_words/read_words.h"
#include "osmxml/osmxml.h"
#include "geo_data/geo_utils.h" // geo_dist_2d
#include "vmap2.h"
#include "vmap2obj.h"

struct OSM_ConfEl: public Opt {
  uint32_t type1, type2; // type for big/small objects
  double min_size;
  OSM_ConfEl(): type1{0xFFFFFFFF}, type2(0xFFFFFFFF), min_size(0) {}
  OSM_ConfEl(const Opt & o, const uint32_t t1, const uint32_t t2, const double ms):
    Opt(o), type1(t1), type2(t2), min_size(ms){ }
};

void
load_osm_conf(const std::string & conf,
              std::list<OSM_ConfEl> & osm_points,
              std::list<OSM_ConfEl> & osm_ways,
              read_words_defs & defs,
              const Opt & opts){

  // read configuration
  double def_min_size=opts.get("osm_min_size",10.0); // m
  if (conf=="") throw Err() << "empty OSM configuration file";
  auto path = file_get_prefix(conf);

  std::ifstream ff(conf);
  if (!ff) throw Err() << "can't open OSM configuration file: " << conf;

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
        load_osm_conf(fn, osm_points, osm_ways, defs, opts);
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

      // set <var> <value> -- set variable
      if (vs[0] == "set") {
        if (vs.size()!=3) throw Err() << "set: two argument expected: <variable> <value>";
        if (vs[1] == "min_size"){
          def_min_size = str_to_type<double>(vs[2]);
          continue;
        }
        throw Err() << "unknown variable name: " << vs[1];
      }

      // point <json tags> <type> -- convert point with <tags> to <type>
      if (vs[0] == "point") {
        if (vs.size()!=3) throw Err()
          << "point: two argument expected: <json tags> <type>";
        auto t1 = VMap2obj::make_type(vs[2]);
        if (VMap2obj::get_class(t1) != VMAP2_POINT &&
            VMap2obj::get_class(t1) != VMAP2_NONE )
          throw Err() << "type should be point:<n> or none: " << vs[2];
        osm_points.emplace_back(Opt(vs[1]), t1, 0xFFFFFFFF, def_min_size);
        continue;
      }

      // way <json tags> <type1> <type2> -- convert point with <tags> to <type1>/<type2>
      if (vs[0] == "way") {
        if (vs.size()!=3 && vs.size()!=4) throw Err()
          << "point: two or three argument expected: <json tags> <type1> [<type2>]";
        if (vs.size()==3) vs.push_back("none");
        auto t1 = VMap2obj::make_type(vs[2]);
        auto t2 = VMap2obj::make_type(vs[3]);
        if (VMap2obj::get_class(t1) != VMAP2_LINE &&
            VMap2obj::get_class(t1) != VMAP2_POLYGON &&
            VMap2obj::get_class(t1) != VMAP2_NONE )
          throw Err() << "type1 should be line:<n>, area:<n>, or none: " << vs[2];
        if (VMap2obj::get_class(t2) != VMAP2_POINT &&
            VMap2obj::get_class(t2) != VMAP2_NONE )
          throw Err() << "type2 should be point:<n> or none: " << vs[3];
        osm_ways.emplace_back(Opt(vs[1]), t1,t2, def_min_size);
        continue;
      }
      throw Err() << "unknown command: " << vs[0];

    }
    catch (Err & e) {
      throw Err() << conf << ":" << line_num[0] << ": " << e.str();
    }
  }
}


void
osm_to_vmap2(const std::string & fname, VMap2 & data, const Opt & opts){

  // configuration:
  double def_min_size=10; // m


  // read configuration
  std::string conf = opts.get("osm_conf","");
  if (conf=="")
    throw Err() << "empty configuration file, use --osm_conf option";

  std::list<OSM_ConfEl> osm_points, osm_ways;
  read_words_defs defs;
  load_osm_conf(conf, osm_points, osm_ways, defs, opts);

  //read OSM data
  OSMXML data_in;
  read_osmxml(fname, data_in, opts);

  // convert OSM points
  for (auto const & e:data_in.points){
    for (auto const & conf:osm_points){
      if (VMap2obj::get_class(conf.type1) == VMAP2_NONE) continue;
      bool match = true;
      for (auto const & o:conf)
        if (e.get(o.first, "") != o.second) match=false;
      if (!match) continue;
      if (data_in.nodes.count(e.id)==0)
        throw Err() << "OSM node does not exist: " << e.id;
      VMap2obj pt(conf.type1);
      pt.name = e.get("name", "");
      pt.set_coords(data_in.nodes.find(e.id)->second);
      data.add(pt);
    }
  }

  // convert OSM ways
  for (auto const & e:data_in.ways){
    for (auto const & conf:osm_ways){
      auto cl1 = VMap2obj::get_class(conf.type1);
      auto cl2 = VMap2obj::get_class(conf.type2);
      if (cl1 == VMAP2_NONE && cl2 == VMAP2_NONE) continue;

      bool match = true;
      for (auto const & o:conf)
        if (e.get(o.first, "") != o.second) match=false;
      if (!match) continue;
      dLine pts;
      for (auto const i:e.nds){
        if (data_in.nodes.count(i)==0)
          throw Err() << "OSM node does not exist: " << i;
        pts.push_back(data_in.nodes.find(i)->second);
      }

      // make object
      dRect r = pts.bbox();
      double d = geo_dist_2d(r.tlc(), r.brc());
      if (d>=conf.min_size && cl1!=VMAP2_NONE){
        VMap2obj li(conf.type1);
        li.name = e.get("name", "");
        // TODO: filter coords
        li.set_coords(pts);
        data.add(li);
      }
      else if (cl2!=VMAP2_NONE){
        VMap2obj pt(conf.type2);
        pt.name = e.get("name", "");
        pt.set_coords(r.cnt());
        data.add(pt);
      }
    }
  }

}
