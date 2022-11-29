#include "osmxml.h"
#include <libxml/xmlreader.h>


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

// read <node> tag
int
read_nod(xmlTextReaderPtr reader, OSMXML & data, const Opt & opts){
  osm_id_t id  = str_to_type<osm_id_t>(GETATTR("id"));
  dPoint p(str_to_type<double>(GETATTR("lon")),
           str_to_type<double>(GETATTR("lat")));

  data.nodes.emplace(id, p);
  if (xmlTextReaderIsEmptyElement(reader)) return 1;

  // if node is not empty (have tags) create point object
  OSMXML::OSM_Point pt;
  pt.id = id;

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
  data.points.push_back(pt);
  return 1;
}

// read <way> tag
int
read_way(xmlTextReaderPtr reader, OSMXML & data, const Opt & opts){
  OSMXML::OSM_Way way;
  way.id  = str_to_type<osm_id_t>(GETATTR("id"));

  while(1){
    int ret =xmlTextReaderRead(reader);
    if (ret != 1) return ret;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    if (type == TYPE_ELEM && NAMECMP("nd")) {
      way.nds.emplace_back(str_to_type<osm_id_t>(GETATTR("ref")));
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
  data.ways.push_back(way);
  return 1;
}

// read <relation> tag
int
read_rel(xmlTextReaderPtr reader, OSMXML & data, const Opt & opts){
  OSMXML::OSM_Rel rel;
  rel.id  = str_to_type<osm_id_t>(GETATTR("id"));

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
  data.relations.push_back(rel);
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
