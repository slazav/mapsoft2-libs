#ifndef OSMXML_H
#define OSMXML_H

// https://wiki.openstreetmap.org/wiki/OSM_XML

#include "opt/opt.h"
#include "geom/point.h"
#include "geom/rect.h"
#include <cstdint>
#include <vector>
#include <string>

// Node ID
typedef int64_t osm_id_t;

struct OSMXML {

  // bounds
  dRect bbox;

  // Nodes: id -> coords, no tags
  std::map<osm_id_t, dPoint> nodes;

  // Point objects created from nodes with tags:
  // tags + Node ID
  struct OSM_Point: public Opt {
    osm_id_t id;
  };
  std::list<OSM_Point> points;

  // OSM way object: tags + id + vector<Node ID>
  struct OSM_Way: public Opt {
    osm_id_t id;
    std::vector<osm_id_t> nds;
  };
  std::list<OSM_Way> ways;

  // OSM relation member
  struct OSM_Memb{
    osm_id_t ref;
    std::string type, role;
  };

  // OSM relation object: tags + vector<Members>
  struct OSM_Rel: public Opt {
    osm_id_t id;
    std::list<OSM_Memb> members;
  };
  std::list<OSM_Rel> relations;

};

void read_osmxml(const std::string &filename, OSMXML & data,
                 const Opt & opts = Opt());

#endif
