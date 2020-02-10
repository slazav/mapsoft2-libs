#ifndef MAPDB_H
#define MAPDB_H

#include <string>
#include <list>
#include <map>
#include <set>

#include "db_simple.h"
#include "db_geohash.h"
#include "geom/multiline.h"

//#include "fig/fig.h"

// current MapDB version (integer)
#define MAPDB_VERSION 0

// Class for vector map (without label information!)
// All coordinates are lat,lon in WGS84 datum.

/*********************************************************************/
// enums

typedef enum{
  MAPDB_POINT    = 0,
  MAPDB_LINE     = 1,
  MAPDB_POLYGON  = 2
} MapDBObjClass;

/*********************************************************************/
// MapDBObj -- a single map object

struct MapDBObj: public dMultiLine {
  MapDBObjClass   cl;      // object class: MAPDB_POINT, MAPDB_LINE, MAPDB_POLYGON
  uint16_t        type;    // = MP type
  float           angle;   // object angle, deg
  std::string     name;    // object name (to be printed on map labels)
  std::string     comm;    // object comment
  std::set<std::string> tags;    // object tags

  // defaults
  MapDBObj() {cl=MAPDB_POINT; type=0; angle=0;}

  // pack object to a string (for DB storage)
  std::string pack() const;

  // unpack object from a string (for DB storage)
  void unpack(const std::string & s);

  /***********************************************/
  // operators <=>
  /// Less then operator.
  bool operator< (const MapDBObj & o) const {
    if (cl!=o.cl)       return cl<o.cl;
    if (type!=o.type)   return type<o.type;
    if (angle!=o.angle) return angle<o.angle;
    if (name!=o.name)   return name<o.name;
    if (comm!=o.comm)   return comm<o.comm;
    if (tags!=o.tags)   return tags<o.tags;
    return dMultiLine::operator<(o);
  }

  /// Equal opertator.
  bool operator== (const MapDBObj & o) const {
    return cl==o.cl && type==o.type && angle==o.angle &&
        name==o.name && comm==o.comm && tags==o.tags &&
        dMultiLine::operator==(o);
  }
  // derived operators:
  bool operator!= (const MapDBObj & other) const { return !(*this==other); } ///< operator!=
  bool operator>= (const MapDBObj & other) const { return !(*this<other);  } ///< operator>=
  bool operator<= (const MapDBObj & other) const { return *this<other || *this==other; } ///< operator<=
  bool operator>  (const MapDBObj & other) const { return !(*this<=other); } ///< operator>

};

/*********************************************************************/
// MapDBLabel -- label structure. Contains everything for drawing
// the label:
// - coordinates (wgs84) of the text
// - text horizontal alignment (LEFT, CENTER, RIGHT)
// - text vertical alignment (BOTTOM, CENTER, TOP)
// - text angle (degrees)
// - text
// - type (used in drawing rules)
// - scale (relative to drawing rule size)
// Each label is attache to an object, but there is no reference
// to this object from the label.
struct MapDBLabel: public dPoint {
  typedef enum{
    LEFT    = 0,
    CENTER  = 1,
    RIGHT   = 2,
  } HAlign;
  HAlign halign;

  typedef enum{
    BOTTOM  = 0,
    MIDDLE  = 1,
    TOP     = 2,
  } VAlign;
  VAlign valign;

  float angle; // degrees
  float scale; // scale
  std::string text;
  uint32_t type;

  // defaults
  MapDBLabel(): halign(LEFT), valign(BOTTOM), angle(0), scale(1), type(0) {}
};

/*********************************************************************/
// MapDB -- a storage for map objects

class MapDB {
private:

  // class for checking/making database folder
  class FolderMaker{ public: FolderMaker(std::string name, bool create); };

  FolderMaker folder; // Making folder for the databases.
                      // This line should appear before all database members.

  DBSimple   mapinfo; // map information
  DBSimple   objects; // object data
  DBSimple   labels;  // label data
  GeoHashDB  geohash; // geohashes for spatial indexing

  int map_version; // set in the constructor

public:

  // Constructor. Open all databases, check map version.
  MapDB(std::string name, bool create = false);

  ///////////////
  /* Function for working with map information (mapinfo.db) */

  /// Get map version.
  uint32_t get_map_version() const {return map_version;}

  /// Get map name. If the field is not set return empty string without an error.
  std::string get_map_name();

  /// Set map name
  void set_map_name(const std::string & name);

  /// Get map border. If the field is not set return empty dMultiLine without an error
  dMultiLine get_map_brd();

  /// Set map border
  void set_map_brd(const dMultiLine & b);


  ///////////////
  /* Functions for working with map objects */

  /// Add object to the map, return object ID.
  uint32_t add(const MapDBObj & o);

  /// Rewrite existing object, update geohashes.
  /// If object does not exist it will be added anyway.
  void put(const uint32_t id, const MapDBObj & o);

  /// Read an object.
  MapDBObj get(const uint32_t id);

  /// Delete an object.
  /// If the object does not exist throw an error.
  void del(const uint32_t id);

  /// Find objects with given type and range
  std::set<uint32_t> find(MapDBObjClass cl, uint16_t type, const dRect & range){
    return geohash.get((cl  << 16) | type, range); }

  /// Find objects with given type and range
  std::set<uint32_t> find(uint32_t etype, const dRect & range){
    return geohash.get(etype, range); }

  /// get all object types in the database
  std::set<uint32_t> get_etypes() {
    return geohash.get_types();}



  ///////////////
  /* Import/export */
  public:

  /// Import objects from MP file.
  void import_mp(
    const std::string & mp_file,
    const Opt & opts);

  /// Export objects to MP file.
  void export_mp(
    const std::string & mp_file,
    const Opt & opts);

  /// Import objects from VMAP file.
  void import_vmap(
    const std::string & vmap_file,
    const Opt & opts);

  /// Export objects to VMAP file.
  void export_vmap(
    const std::string & vmap_file,
    const Opt & opts);

};

// add option groups:
//   MAPDB_MP_IMP, MAPDB_MP_EXP, MAPDB_VMAP_IMP, MAPDB_VMAP_EXP

#include "getopt/getopt.h"

void ms2opt_add_mapdb_mp_imp(GetOptSet & opts);
void ms2opt_add_mapdb_mp_exp(GetOptSet & opts);

void ms2opt_add_mapdb_vmap_imp(GetOptSet & opts);
void ms2opt_add_mapdb_vmap_exp(GetOptSet & opts);

#endif
