#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <math.h>

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include "opt/opt.h"
#include "err/err.h"
#include "time_fmt/time_fmt.h"

#include "geo_data.h"

using namespace std;

/* 
KML writing:
- waypoints, tracks, no maps.
- waypoint lists:
  * in separate folders
  * name -> <name> attribute
- waypoints:
  * name -> <name>
  * comm -> <description>
  * x,y,z -> <coordinates>
- tracks:
  * name -> <description>
  * open/closed track support
*/

void start_element(xmlTextWriterPtr & writer, const char *name){
  if (xmlTextWriterStartElement(writer, BAD_CAST name)<0)
    throw string("starting <") + name + "> element";
}

void end_element(xmlTextWriterPtr & writer, const char *name){
  if (xmlTextWriterEndElement(writer) < 0)
    throw string("closing <") + name + "> element";
}

void write_cdata_element(xmlTextWriterPtr & writer, const char *name, const string & value){
  if (name == NULL || strlen(name)==0 || value == "") return;
  start_element(writer, name);
  if (xmlTextWriterWriteFormatCDATA(writer, "%s", value.c_str())<0)
    throw string("writing <") + name + "> element";
  end_element(writer, name);
}


/*******************************/
void
write_kml (const string &filename, const GeoData & data, const Opt & opts){

  LIBXML_TEST_VERSION

  // set some parameters
  int indent = opts.get<int>("xml_indent", 1);
  char qchar = opts.get<char>("xml_qchar", '\'');
  std::string ind_str = opts.get("xml_ind_str", "  ");
  bool v = opts.get("verbose", false);
  if (v) cerr << "Writing KML file: " << filename << endl;

  // create XML writer
  xmlTextWriterPtr writer =
    xmlNewTextWriterFilename(filename.c_str(), opts.get<int>("xml_compr", 0));
  if (writer == NULL)
    throw Err() << "write_kml: can't write to file: " << filename;

  try {
    if (xmlTextWriterSetIndent(writer, indent)<0 ||
        xmlTextWriterSetIndentString(writer, BAD_CAST ind_str.c_str())<0 ||
        xmlTextWriterSetQuoteChar(writer, qchar)<0)
      throw "setting xml writer parameters";

    // start XML document
    if (xmlTextWriterStartDocument(writer, "1.0", "UTF-8", NULL)<0)
      throw "starting the xml document";

    // start KML element.
    // BAD_CAST converts (const char*) to xmlChar*.
    if (xmlTextWriterStartElement(writer, BAD_CAST "kml")<0 ||
        xmlTextWriterWriteAttribute(writer,
          BAD_CAST "xmlns", BAD_CAST "http://earth.google.com/kml/2.1")<0)
      throw "starting <kml> element";

    start_element(writer, "Document");

    // Writing waypoints:
    for (auto wpl: data.wpts) {

     if (v) cerr << "  Writing waypoints: " << wpl.name
                 << " (" << wpl.size() << " points)" << endl;

      start_element(writer, "Folder");
      write_cdata_element(writer, "name", wpl.name);
      write_cdata_element(writer, "description", wpl.comm);

      for (auto wp : wpl) {
        start_element(writer, "Placemark");
        write_cdata_element(writer, "name", wp.name);
        write_cdata_element(writer, "description", wp.comm);

        if (wp.t > 0){
          start_element(writer, "TimeStamp");
          if (xmlTextWriterWriteFormatElement(writer,
             BAD_CAST "when", "%s", write_fmt_time("%FT%T%fZ", wp.t).c_str())<0)
               throw "writing <when> element";
          end_element(writer, "TimeStamp");
        }

        start_element(writer, "Point");
        if (xmlTextWriterWriteFormatElement(writer,
           BAD_CAST "coordinates", "%.7f,%.7f,%.2f",
               wp.x, wp.y, wp.z)<0)
          throw "writing <coordinates> element";
        end_element(writer, "Point");
        end_element(writer, "Placemark");
      }
      end_element(writer, "Folder");
    }

    // Writing tracks:
    for (auto trk: data.trks) {

     if (v) cerr << "  Writing track: " << trk.name
                 << " (" << trk.npts() << " points)" << endl;

      start_element(writer, "Placemark");
      write_cdata_element(writer, "name", trk.name);
      write_cdata_element(writer, "description", trk.comm);
      start_element(writer, "MultiGeometry");
      const char *linename = trk.opts.get<string>("type")=="closed"? "Polygon":"LineString";

      for (auto seg : trk) {
        start_element(writer, linename);
        if (xmlTextWriterWriteFormatElement(writer,
           BAD_CAST "tessellate", "%d", 1)<0)
          throw "writing <tessellate> element";
        start_element(writer, "coordinates");

        for (auto tp : seg) {
          if (xmlTextWriterWriteFormatString(writer,
             " %.7f,%.7f,%.2f", tp.x, tp.y, tp.z)<0)
            throw "writing <coordinates> element";
        }
        end_element(writer, "coordinates");
        end_element(writer, linename);
      }
      end_element(writer, "MultiGeometry");
      end_element(writer, "Placemark");
    }
    end_element(writer, "Document");
    end_element(writer, "kml");
    if (xmlTextWriterEndDocument(writer) < 0) throw "closing xml document";

  }
  catch (const char *c){
    xmlFreeTextWriter(writer);
    throw Err() << "write_gpx: error in " << c;
  }

  // free resources
  xmlFreeTextWriter(writer);

  return;
}

#define TYPE_ELEM      1
#define TYPE_TEXT      3
#define TYPE_CDATA     4
#define TYPE_COMM      8
#define TYPE_SWS      14
#define TYPE_ELEM_END 15

#define NAMECMP(x) (xmlStrcasecmp(name,(const xmlChar *)x)==0)
#define GETATTR(x) (const char *)xmlTextReaderGetAttribute(reader, (const xmlChar *)x)
#define GETVAL     (const char *)xmlTextReaderConstValue(reader)

int
read_text_node(xmlTextReaderPtr reader, const char * nn, string & str){
  int ret=1;
  str.clear();
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (type == TYPE_TEXT || type == TYPE_CDATA) str += GETVAL;
    else if (NAMECMP(nn) && (type == TYPE_ELEM_END)) break;
    else cerr << "Skipping node\"" << name << "\" in text node (type: " << type << ")\n";
  }
  return ret;
}

int
read_timestamp_node(xmlTextReaderPtr reader, const char * nn, string & str){
  int ret=1;
  str.clear();
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (NAMECMP("when") && (type == TYPE_ELEM))
      ret=read_text_node(reader, "when", str);
    else if (NAMECMP(nn) && (type == TYPE_ELEM_END)) break;
    else cerr << "Skipping node\"" << name << "\" in TimeStamp node (type: " << type << ")\n";
  }
  return ret;
}

int
read_point_node(xmlTextReaderPtr reader, GeoWpt & ww){
  int ret=1;
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;

    else if (NAMECMP("coordinates") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "coordinates", str);
      if (ret != 1) break;
      char s1,s2;
      istringstream s(str);
      s >> ws >> ww.x >> ws >> s1 >> ws >> ww.y;
      if (!s.eof()) s >> ws;
      if (s.fail() || s1!=','){
        cerr << "Warning: Coord error (" << str << ")\n";
        ret=0;
        break;
      }
      if (!s.eof()) {
         s >> s2 >> ws >> ww.z;
        if (s.fail() || s2!=',') {
          cerr << "Warning: Coord error (" << str << ")\n";
          ret=0;
          break;
        }
      }
    }
    else if (NAMECMP("Point") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      cerr << "Skipping node\"" << name << "\" in Point (type: " << type << ")\n";
    }
  }
  return ret;
}

int
read_linestring_node(xmlTextReaderPtr reader, GeoTrk & T){
  int ret=1;
  T.add_segment();
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;

    else if (NAMECMP("tessellate") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "tessellate", str);
      if (ret != 1) break;
    }
    else if (NAMECMP("coordinates") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "coordinates", str);
      if (ret != 1) break;
      char s1,s2;
      istringstream s(str);
      GeoTpt tp;
      while (!s.eof()){
        s >> ws >> tp.x >> ws >> s1 >>
             ws >> tp.y >> ws >> s2 >>
             ws >> tp.z >> ws;
        if (s1!=',' || s2!=','){
          cerr << "Warning: Coord error\n";
          break;
        }
        T.add_point(tp);
      }
    }
    else if (NAMECMP("LineString") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      cerr << "Skipping node\"" << name << "\" in LineString (type: " << type << ")\n";
    }
  }
  return ret;
}

/* same as Linestring, but for closed lines */
int
read_polygon_node(xmlTextReaderPtr reader, GeoTrk & T){
  int ret=1;
  T.opts.put("type", "closed");
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (NAMECMP("tessellate") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "tessellate", str);
      if (ret != 1) break;
    }
    else if (NAMECMP("coordinates") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "coordinates", str);
      if (ret != 1) break;
      char s1,s2;
      istringstream s(str);
      GeoTpt tp;
      T.add_segment();
      while (!s.eof()){
        s >> ws >> tp.x >> ws >> s1 >>
             ws >> tp.y >> ws >> s2 >>
             ws >> tp.z >> ws;
        if (s1!=',' || s2!=','){
          cerr << "Warning: Coord error\n";
          break;
        }
        T.add_point(tp);
      }
    }
    else if (NAMECMP("Polygon") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      cerr << "Skipping node\"" << name << "\" in Polygon (type: " << type << ")\n";
    }
  }
  return ret;
}


int
read_gx_track_node(xmlTextReaderPtr reader, GeoTrk & T){
  int ret=1;
  T = GeoTrk();
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (NAMECMP("when") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "when", str);
      if (ret != 1) break;
    }
    else if (NAMECMP("gx:coord") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "gx:coord", str);
      if (ret != 1) break;
      istringstream s(str);
      GeoTpt tp;
      s >> ws >> tp.x >> ws
        >> ws >> tp.y >> ws
        >> ws >> tp.z >> ws;
      T.add_point(tp);
    }
    else if (NAMECMP("gx:Track") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      cerr << "Skipping node\"" << name << "\" in gx:Track (type: " << type << ")\n";
    }
  }
  return ret;
}

int
read_multigeometry_node(xmlTextReaderPtr reader, GeoTrk & T){
  int ret=1;
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (NAMECMP("LineString") && (type == TYPE_ELEM)){
      ret=read_linestring_node(reader, T);
      if (ret != 1) break;
    }
    else if (NAMECMP("Polygon") && (type == TYPE_ELEM)){
      ret=read_polygon_node(reader, T);
      if (ret != 1) break;
    }
    else if (NAMECMP("MultiGeometry") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      cerr << "Skipping node\"" << name << "\" in MultiGeometry (type: " << type << ")\n";
    }
  }
  return ret;
}

int
read_placemark_node(xmlTextReaderPtr reader,
     GeoWptList & W, GeoTrk & T, GeoMapList & M){

  GeoWpt ww;
  GeoMap mm;
  T = GeoTrk();

  int ot=-1;
  string skip_el="";
  int ret=1;

  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (skip_el != ""){
      if (skip_el == (char*)name && type == TYPE_ELEM_END) skip_el="";
      continue;
    }
    else if (NAMECMP("name") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "name", str);
      if (ret != 1) break;
      ww.name = str;
       T.name = str;
      mm.name = str;
    }

    else if (NAMECMP("description") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "description", str);
      if (ret != 1) break;
      ww.comm = str;
       T.comm = str;
      mm.comm = str;
    }
    else if (NAMECMP("TimeStamp") && (type == TYPE_ELEM)){
      string str;
      ret=read_timestamp_node(reader, "TimeStamp", str);
      if (ret != 1) break;
      if (str!="") ww.t = parse_utc_time(str);
    }
    else if (NAMECMP("Point") && (type == TYPE_ELEM)){
      ot=0;
      ret=read_point_node(reader, ww);
      if (ret != 1) break;
    }
    else if (NAMECMP("LineString") && (type == TYPE_ELEM)){
      ot=1;
      ret=read_linestring_node(reader, T);
      if (ret != 1) break;
    }
    else if (NAMECMP("Polygon") && (type == TYPE_ELEM)){
      ot=1;
      ret=read_polygon_node(reader, T);
      if (ret != 1) break;
    }
    else if (NAMECMP("MultiGeometry") && (type == TYPE_ELEM)){
      ot=1;
      ret=read_multigeometry_node(reader, T);
      if (ret != 1) break;
    }
    else if (NAMECMP("gx:Track") && (type == TYPE_ELEM)){
      ot=1;
      ret=read_gx_track_node(reader, T);
      if (ret != 1) break;
    }
    else if (NAMECMP("Placemark") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      if (type == TYPE_ELEM && !xmlTextReaderIsEmptyElement(reader)) skip_el=(char*)name;
      cerr << "Skipping node\"" << name << "\" in Placemark (type: " << type << ")\n";
    }
  }
  if (ot==0) W.push_back(ww);
  return ret;
}



int
read_folder_node(xmlTextReaderPtr reader, GeoData & data, const bool v){ // similar to document node
  GeoWptList W;
  GeoTrk     T;
  GeoMapList M;
  string skip_el="";
  int ret=1;
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (skip_el != ""){
      if (skip_el == (char*)name && type == TYPE_ELEM_END) skip_el="";
      continue;
    }
    else if (NAMECMP("name") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "name", str);
      if (ret != 1) break;
      W.name = str;
      T.name = str;
      M.name = str;
    }
    else if (NAMECMP("description") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "description", str);
      if (ret != 1) break;
      W.comm = str;
      T.comm = str;
      M.comm = str;
    }
    else if (NAMECMP("Placemark") && (type == TYPE_ELEM)){
      ret=read_placemark_node(reader, W, T, M);
      if (T.size()) data.trks.push_back(T);
      if (ret != 1) break;
    }
    else if (NAMECMP("Folder") && (type == TYPE_ELEM)){
      ret=read_folder_node(reader, data, v);
      if (ret != 1) break;
    }
    else if (NAMECMP("Folder") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      if (type == TYPE_ELEM && !xmlTextReaderIsEmptyElement(reader)) skip_el=(char*)name;
      cerr << "Skipping node\"" << name << "\" in Folder (type: " << type << ")\n";
    }
  }
  if (W.size()){
    if (v) cerr << "  Reading waypoints: " << W.name
                << " (" << W.size() << " points)" << endl;
    data.wpts.push_back(W);
  }
  if (M.size()){ // not supported yet
    if (v) cerr << "  Reading maps: " << M.name
                << " (" << M.size() << " maps)" << endl;
    data.maps.push_back(M);
  }
  return ret;
}

int
read_document_node(xmlTextReaderPtr reader, GeoData & data, const bool v){
  string skip_el="";
  GeoWptList W;
  GeoTrk     T;
  GeoMapList M;
  int ret=1;
  while(1){
    ret =xmlTextReaderRead(reader);
    if (ret != 1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;
    else if (skip_el != ""){
      if (skip_el == (char*)name && type == TYPE_ELEM_END) skip_el="";
      continue;
    }
    else if (NAMECMP("name") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "name", str);
      if (ret != 1) break;
      W.name = str;
      T.name = str;
      M.name = str;
    }
    else if (NAMECMP("description") && (type == TYPE_ELEM)){
      string str;
      ret=read_text_node(reader, "description", str);
      if (ret != 1) break;
      W.comm = str;
      T.comm = str;
      M.comm = str;
    }
    else if (NAMECMP("Placemark") && (type == TYPE_ELEM)){
      ret=read_placemark_node(reader, W, T, M);
      if (ret != 1) break;
    }
    else if (NAMECMP("Folder") && (type == TYPE_ELEM)){
      ret=read_folder_node(reader, data, v);
      if (ret != 1) break;
    }
    else if (NAMECMP("Document") && (type == TYPE_ELEM_END)){
      break;
    }
    else {
      if (type == TYPE_ELEM && !xmlTextReaderIsEmptyElement(reader)) skip_el=(char*)name;
      cerr << "Skipping node\"" << name << "\" in Document (type: " << type << ")\n";
    }
  }
  if (W.size()){
    if (v) cerr << "  Reading waypoints: " << W.name
                << " (" << W.size() << " points)" << endl;
    data.wpts.push_back(W);
  }
  if (T.size()){
    if (v) cerr << "  Reading track: " << T.name
                << " (" << T.npts() << " points)" << endl;
    data.trks.push_back(T);
  }
  if (M.size()){ // not supported yet
    if (v) cerr << "  Reading maps: " << M.name
                << " (" << M.size() << " maps)" << endl;
    data.maps.push_back(M);
  }
  return ret;
}

int
read_kml_node(xmlTextReaderPtr reader, GeoData & data, const bool v){
  while(1){
    int ret =xmlTextReaderRead(reader);
    if (ret != 1) return ret;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);

    if (type == TYPE_SWS || type == TYPE_COMM) continue;

    else if (NAMECMP("Document") && (type == TYPE_ELEM)){
      ret=read_document_node(reader, data, v);
      if (ret != 1) return ret;
    }

    else if (NAMECMP("kml") && (type == TYPE_ELEM_END)){
      break;
    }

    else {
      cerr << "Skipping node\"" << name << "\" in kml (type: " << type << ")\n";
    }
  }
  return 1;
}


void
read_kml(const string &filename, GeoData & data, const Opt & opts) {

  LIBXML_TEST_VERSION

  xmlTextReaderPtr reader;
  int ret;

  reader = xmlReaderForFile(filename.c_str(), NULL, 0);
  if (reader == NULL)
    throw Err() << "Can't open KML file: " << filename;

  bool v = opts.get("verbose", false);
  if (v) cerr << "Reading KML file: " << filename << endl;

  // parse file
  while (1){
    ret = xmlTextReaderRead(reader);
    if (ret!=1) break;

    const xmlChar *name = xmlTextReaderConstName(reader);
    int type = xmlTextReaderNodeType(reader);
    if (NAMECMP("kml") && (type == TYPE_ELEM))
      ret = read_kml_node(reader, data, v);
    if (ret!=1) break;
  }

  // free resources
  xmlFreeTextReader(reader);
  xmlCleanupParser();
  xmlMemoryDump();

  if (ret != 0) throw Err() << "Can't parse KML file: " << filename;
}
