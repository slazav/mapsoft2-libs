#ifndef GPKG_H
#define GPKG_H

#include <string>
#include <map>
#include <memory>
#include <sqlite3.h>
#include "geom/multiline.h"
#include "geo_data/conv_geo.h"
#include "vmap2/vmap2obj.h"


/*
Read gpkg database
https://wiki.openstreetmap.org/wiki/GeoPackage
https://www.opengeospatial.org/standards/geopackage
https://www.geopackage.org/spec140/index.html

*/

class GPKG {
  public:
  struct table_t {
    std::string type; // data type (features, tiles, etc.)
    std::string gcol; // name of geometry column
    std::string gtype; // geometry type (POINT etc.)
    std::string srs; // srs (ESPG:4326, etc.)
  };


  private:
    std::shared_ptr<sqlite3> db;
    std::map<std::string, table_t> tables;

    // main select statement
    std::shared_ptr<sqlite3_stmt> stmt_read;
    // current table
    table_t tab;
    // current type: "point:1", "line:1", or "area:1"
    std::string vmap_type;

  public:

    // open gpkg file (read only)
    GPKG(const std::string & fname);

    // SELECT table_name FROM gpkg_contents
    std::map<std::string, table_t> get_tables() const {return tables;}

    // start reading a table
    void read_start(const std::string & table);

    // Get next object from the current table (empty object at the end).
    // No coordinate conversion is done - I need a low level interface
    // to read objects one-by-one and convert/crop them somewhere else.
    // If data is finished object with type==-1 ("none") is returned.
    VMap2obj read_next();

  private:
    // decode geometry blob
    dMultiLine decode_geom(std::string & data);

    // some functions are copies from image_t_mbtiles.h
    std::shared_ptr<sqlite3_stmt> sql_prepare(const char * cmd) const;

    void sql_bind_str(sqlite3_stmt *stmt, const int n, const std::string & str) const;
    void sql_bind_int(sqlite3_stmt *stmt, const int n, const int v) const;

    std::string sql_column_text(sqlite3_stmt *stmt, int n) const;
    std::string sql_column_name(sqlite3_stmt *stmt, int n) const;
};


#endif
