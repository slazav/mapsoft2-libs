#ifndef OSMXML_H
#define OSMXML_H

// https://wiki.openstreetmap.org/wiki/OSM_XML

#include "opt/opt.h"
#include "geom/point.h"
#include "geom/line.h"
#include "geom/multiline.h"
#include "geom/rect.h"
#include <cstdint>
#include <vector>
#include <string>

// Node ID
typedef int64_t osm_id_t;

struct OSMXML {

  // bounds
  dRect bbox;

  // Nodes:
  // id -> coords, no tags
  std::map<osm_id_t, dPoint> nodes;

  // Point objects created from nodes with tags:
  // id -> tags
  typedef Opt OSM_Point;
  std::map<osm_id_t, OSM_Point> points;

  // OSM way objects:
  // id -> tags + vector<Node ID>
  struct OSM_Way: public Opt {
    std::vector<osm_id_t> nodes;
  };
  std::map<osm_id_t, OSM_Way> ways;

  // OSM relation member: ref, type, role
  struct OSM_Memb{
    osm_id_t ref;
    std::string type, role;
  };

  // OSM relations
  // id -> tags + list<members>
  struct OSM_Rel: public Opt {
    std::list<OSM_Memb> members;
  };
  std::map<osm_id_t, OSM_Rel> relations;

  /***************************/
  // functions

  // get coordinates of OSM node object:
  dPoint get_node_coords(const osm_id_t id);

  // get coordinates of OSM way object:
  dLine get_way_coords(const OSMXML::OSM_Way & way);

  // Get coordinates of multipolygon relation object:
  // See: https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
  // https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
  dMultiLine get_rel_coords(const OSMXML::OSM_Rel & rel);

};

void read_osmxml(const std::string &filename, OSMXML & data,
                 const Opt & opts = Opt());

#endif
