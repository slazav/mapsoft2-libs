#include "osmxml.h"
#include <libxml/xmlreader.h>
#include <set>


// same as in io_gpx.cpp, io_kml.cpp
#define TYPE_ELEM      1
#define TYPE_TEXT      3
#define TYPE_CDATA     4
#define TYPE_COMM      8
#define TYPE_SWS      14
#define TYPE_ELEM_END 15

#define NAMECMP(x) (xmlStrcasecmp(name,(const xmlChar *)x)==0)
#define GETATTR(x) (const char *)xmlTextReaderGetAttribute(reader, (const xmlChar *)x)
#define GETVAL     (const char *)xmlTextReaderConstValue(reader)

/********************************************************************/

dPoint
OSMXML::get_node_coords(const osm_id_t id){
  if (nodes.count(id)==0)
    throw Err() << "OSM node does not exist: " << id;
  return nodes.find(id)->second;
}

dLine
OSMXML::get_way_coords(const OSMXML::OSM_Way & way){
  dLine ret;
  for (auto const i:way.nodes)
    ret.push_back(get_node_coords(i));
  return ret;
}

dMultiLine
OSMXML::get_rel_coords(const OSMXML::OSM_Rel & rel){
  dMultiLine ret;

  // multipolygons should have type=multipolygon|boundary
  if (rel.get("type")!="multipolygon" && rel.get("type")!="boundary")
    return ret;

  // Collect all members with type=way, role=inner|outer
  // Also check that way ID exists, and that each way contains >1 points.
  std::set<osm_id_t> parts;
  for (auto const & p:rel.members){
    if (p.type!="way") continue;
    if (p.role!="inner" && p.role!="outer") continue;
    if (ways.count(p.ref)==0) continue;
//      throw Err() << "way does not exist: " << p.ref;
    if (ways.find(p.ref)->second.nodes.size()<2)
      throw Err() << "way is too short: " << p.ref;
    parts.insert(p.ref);
  }

  // build rings
  while (parts.size()>0){
    // take first part
    auto i = parts.begin();
    auto ring = ways.find(*i)->second.nodes;
    parts.erase(i);

    while (ring[0]!=ring[ring.size()-1]){
      bool mod=false;
      auto j = parts.begin();
      while (j!=parts.end()){
        auto seg = ways.find(*j)->second.nodes;
        if (seg[0] == ring[ring.size()-1]){
          ring.insert(ring.end(), seg.begin()+1, seg.end());
          mod=true;
        }
        else if (seg[seg.size()-1] == ring[ring.size()-1]){
          ring.insert(ring.end(), seg.rbegin()+1, seg.rend());
          mod=true;
        }
        if (mod){
          j=parts.erase(j);
          break;
        }
        else ++j;
      }
      if (!mod) break;
    }
    dLine crd;
    for (auto const i:ring)
      crd.push_back(get_node_coords(i));
    ret.push_back(crd);
  }
  return ret;
}


/********************************************************************/

// read <node> tag
int
read_nod(xmlTextReaderPtr reader, OSMXML & data, const Opt & opts){
  auto id  = str_to_type<osm_id_t>(GETATTR("id"));
  dPoint p(str_to_type<double>(GETATTR("lon")),
           str_to_type<double>(GETATTR("lat")));

  data.nodes.emplace(id, p);
  if (xmlTextReaderIsEmptyElement(reader)) return 1;

  // if node is not empty (have tags) create point object
  OSMXML::OSM_Point pt;

  while(1){
    int ret =xmlTextReaderRead(reader);
    if (ret != 1) return ret;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    if (type == TYPE_ELEM && NAMECMP("tag")) {
      pt.emplace(GETATTR("k"), GETATTR("v"));
      continue;
    }
    if (type == TYPE_ELEM_END && NAMECMP("node")) break;
    std::cerr << "Warning: Unknown node \"" << name
              << "\" in <node> (type: " << type << ")\n";
  }
  data.points.emplace(id, pt);
  return 1;
}

// read <way> tag
int
read_way(xmlTextReaderPtr reader, OSMXML & data, const Opt & opts){
  OSMXML::OSM_Way way;
  auto id = str_to_type<osm_id_t>(GETATTR("id"));

  while(1){
    int ret =xmlTextReaderRead(reader);
    if (ret != 1) return ret;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    if (type == TYPE_ELEM && NAMECMP("nd")) {
      way.nodes.emplace_back(str_to_type<osm_id_t>(GETATTR("ref")));
      continue;
    }
    if (type == TYPE_ELEM && NAMECMP("tag")) {
      way.emplace(GETATTR("k"), GETATTR("v"));
      continue;
    }
    if (type == TYPE_ELEM_END && NAMECMP("way")) break;
    std::cerr << "Warning: Unknown node \"" << name
              << "\" in <way> (type: " << type << ")\n";
  }
  data.ways.emplace(id, way);
  return 1;
}

// read <relation> tag
int
read_rel(xmlTextReaderPtr reader, OSMXML & data, const Opt & opts){
  OSMXML::OSM_Rel rel;
  auto id  = str_to_type<osm_id_t>(GETATTR("id"));

  while(1){
    int ret =xmlTextReaderRead(reader);
    if (ret != 1) return ret;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    if (type == TYPE_ELEM && NAMECMP("member")) {
      OSMXML::OSM_Memb m;
      m.type = GETATTR("type");
      m.role = GETATTR("role");
      m.ref  = str_to_type<osm_id_t>(GETATTR("ref"));
      rel.members.push_back(m);
      continue;
    }
    if (type == TYPE_ELEM && NAMECMP("tag")) {
      rel.emplace(GETATTR("k"), GETATTR("v"));
      continue;
    }
    if (type == TYPE_ELEM_END && NAMECMP("relation")) break;
    std::cerr << "Warning: Unknown node \"" << name
              << "\" in <relation> (type: " << type << ")\n";
  }
  data.relations.emplace(id, rel);
  return 1;
}


int
read_osm_node(xmlTextReaderPtr reader, OSMXML & data, const Opt & opts){

  while(1){
    int ret =xmlTextReaderRead(reader);
    if (ret != 1) return ret;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;

    if (NAMECMP("bounds")){
      std::string x1 = GETATTR("minlon");
      std::string y1 = GETATTR("minlat");
      std::string x2 = GETATTR("maxlon");
      std::string y2 = GETATTR("maxlat");
      if (x1=="" || x2=="" || y1=="" || y2==""){
        std::cerr << "Warning: bad <bounds> tag\n";
        continue;
      }
      data.bbox = dRect(
        dPoint(str_to_type<double>(x1),str_to_type<double>(y1)),
        dPoint(str_to_type<double>(y1),str_to_type<double>(y2))
      );
      continue;
    }
    if (NAMECMP("node") && (type == TYPE_ELEM)){
      ret=read_nod(reader, data, opts);
      if (ret != 1) return ret;
      continue;
    }
    if (NAMECMP("way") && (type == TYPE_ELEM)){
      ret=read_way(reader, data, opts);
      if (ret != 1) return ret;
      continue;
    }
    if (NAMECMP("relation") && (type == TYPE_ELEM)){
      ret=read_rel(reader, data, opts);
      if (ret != 1) return ret;
      continue;
    }

    // end tag
    if (NAMECMP("osm") && (type == TYPE_ELEM_END)) break;

    std::cerr << "Warning: Unknown node \"" << name << "\" in osm (type: " << type << ")\n";
  }
  return 1;
}


void
read_osmxml(const std::string &filename, OSMXML & data, const Opt & opts) {

  LIBXML_TEST_VERSION
  xmlTextReaderPtr reader;
  int ret;

  reader = xmlReaderForFile(filename.c_str(), NULL, 0);
  if (reader == NULL)
    throw Err() << "Can't open file: " << filename;

  if (opts.get("verbose", false)) std::cerr <<
    "Reading OSMXML file: " << filename << std::endl;

  // parse file
  while (1){
    ret = xmlTextReaderRead(reader);
    if (ret!=1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);
    if (NAMECMP("osm") && (type == TYPE_ELEM))
      ret = read_osm_node(reader, data, opts);
    if (ret!=1) break;
  }

  // free resources
  xmlFreeTextReader(reader);

  if (ret != 0) throw Err() << "Can't parse OSMXML file: " << filename;

}
