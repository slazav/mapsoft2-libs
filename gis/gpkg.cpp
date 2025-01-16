#include <cstdint>
#include "gpkg.h"
#include "ewkb.h"
#include "filename/filename.h"

GPKG::GPKG(const std::string & fname){

  // open sqlite database readonly:
  if (!file_exists(fname)) throw Err()
    << "can't open gpkg database: " << fname << ": file does not exists";

  sqlite3 *pdb;
  int res;
  res = sqlite3_open_v2(fname.c_str(), &pdb, SQLITE_OPEN_READONLY, NULL);
  if (res != SQLITE_OK){
    sqlite3_close(pdb);
    throw Err() << "can't open sqlite database: " << fname<< ": " << sqlite3_errstr(res);
  }
  db = std::shared_ptr<sqlite3>(pdb, sqlite3_close);

  // read gpkg_contents table, fill tables structure:
  auto stmt = sql_prepare("SELECT table_name, data_type FROM gpkg_contents");
  while ((res = sqlite3_step(stmt.get())) != SQLITE_DONE){
    if (res!=SQLITE_ROW) throw Err()
      << sqlite3_expanded_sql(stmt.get()) << ": " << sqlite3_errstr(res);
    auto name = sql_column_text(stmt.get(), 0);
    table_t table;
    table.type = sql_column_text(stmt.get(), 1);
    tables.emplace(name, table);
  }

  // read gpkg_geometry_columns table, fill tables structure:
  stmt = sql_prepare(
    "SELECT table_name, column_name, geometry_type_name, srs_id FROM gpkg_geometry_columns");
  while ((res = sqlite3_step(stmt.get())) != SQLITE_DONE){
    if (res!=SQLITE_ROW) throw Err()
      << sqlite3_expanded_sql(stmt.get()) << ": " << sqlite3_errstr(res);
    auto name = sql_column_text(stmt.get(), 0);
    if (!tables.count(name) || tables[name].type != "features") continue;
    tables[name].gcol  = sql_column_text(stmt.get(), 1);
    tables[name].gtype = sql_column_text(stmt.get(), 2);
    int srs_id = sqlite3_column_int(stmt.get(), 3);

    // convert srs_id to name:
    auto stmt1 = sql_prepare(
      "SELECT organization,organization_coordsys_id FROM gpkg_spatial_ref_sys WHERE srs_id=?");
    sql_bind_int(stmt1.get(), 1, srs_id);
    res = sqlite3_step(stmt1.get());
    if (res == SQLITE_DONE) continue; // keep srs empty
    if (res != SQLITE_ROW) throw Err()
      << sqlite3_expanded_sql(stmt1.get()) << ": " << sqlite3_errstr(res);

    tables[name].srs = sql_column_text(stmt1.get(), 0) + ":"
                     + sql_column_text(stmt1.get(), 1);
  }

}

/**********************************************************/

void
GPKG::read_start(const std::string & table){

  if (tables.count(table)!=1) throw Err()
    << "gpkg_contents does not have table: " << table;

  if (tables[table].type!="features") throw Err()
    << "unsupported data_type for table: " << table << ": " << tables[table].type;

  // it will be no coordinates
  if (tables[table].gcol=="")
    std::cerr << "no geometry columnun for table: " << table << "\n";

  // prepare main select command
  stmt_read = sql_prepare(("SELECT * from " + table).c_str());
  tab = tables[table]; // save current table

  // set ibject type
  if (tab.gtype=="POINT") vmap_type = "point:1";
  else if (tab.gtype=="MULTILINESTRING") vmap_type = "line:1";
  else if (tab.gtype=="MULTIPOLYGON") vmap_type = "area:1";
  else throw Err() << "unknown geometry type: " << tab.gtype;

}

VMap2obj
GPKG::read_next(){
  auto s = stmt_read.get();
  auto res = sqlite3_step(s);
  if (res == SQLITE_DONE) return VMap2obj("none");
  if (res != SQLITE_ROW) throw Err()
    << sqlite3_expanded_sql(s) << ": " << sqlite3_errstr(res);

  VMap2obj ret(vmap_type);

  auto n = sqlite3_column_count(s);
  for (int i=0; i<n; ++i){
    auto col = sql_column_name(s,i);
    // non-geometry column
    if (col != tab.gcol){
      ret.opts.emplace(col, sql_column_text(s,i)); 
      continue;
    }

    // geometry column
    if (sqlite3_column_type(s, i) != SQLITE_BLOB)
      throw Err() << sqlite3_expanded_sql(s)
                  << ": blob data expected for geometry column";
    // obtain data
    std::string data((const char *)sqlite3_column_blob(s, i),
                                   sqlite3_column_bytes(s, i));

    ret.set_coords(decode_geom(data));
  }
  return ret;
}

/**********************************************************/
// see https://www.geopackage.org/spec140/index.html#gpb_format
dMultiLine
GPKG::decode_geom(std::string & data){
  size_t pos = 0;

  // Decode header
  if (pos+8>=data.size())
    throw Err() << "decode_geom: not enough data";
  if ((data[0]<<8) + data[1] != 0x4750)
    throw Err() << "decode_geom: wrong magic number";
  uint8_t version = data[2];
  uint8_t flags = data[3];

  bool extended = flags & (1<<5);
  bool empty = flags & (1<<4);
  int env_type = (flags>>1) & 7;
  bool horder = flags & 1; // only for header

  int srs_id = horder?
    (data[7]<<24) + (data[6]<<16) + (data[5]<<8) + data[4] :
    (data[4]<<24) + (data[5]<<16) + (data[6]<<8) + data[7];

  pos+=8;
  // skip envelope
  switch (env_type){
    case 0: break;
    case 1: pos+=32; break;
    case 2:
    case 3: pos+=48; break;
    case 4: pos+=64; break;
    default: throw Err() << "decode_geom: bad envelope type: " << env_type;
  }
  if (empty) {
    if (pos!=data.size()) throw Err() << "decode_geom: extra data in empty geom";
    return dMultiLine();
  }

  // Decode geom (no conversion, binary)
  auto ret = ewkb_decode(data, false, true, pos);
  if (pos!=data.size()) throw Err() << "decode_geom: extra data in geom";

  return ret;
}

/**********************************************************/

std::shared_ptr<sqlite3_stmt>
GPKG::sql_prepare(const char * cmd) const{
  sqlite3_stmt *stmt;
  // cmd memory should be managed in the program!
  int res = sqlite3_prepare(db.get(), cmd, -1, &stmt, 0);
  if (res!=SQLITE_OK) throw Err()
  << "can't prepare sql command: " << cmd << ": " << sqlite3_errstr(res);
  return std::shared_ptr<sqlite3_stmt>(stmt, sqlite3_finalize);
}

void
GPKG::sql_bind_str(sqlite3_stmt *stmt, const int n, const std::string & str) const{
  int res = sqlite3_bind_text(stmt, n, str.c_str(), str.size(), SQLITE_STATIC);
  if (res!=SQLITE_OK) throw Err()
    << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
    << ": " << sqlite3_errstr(res);
}

void
GPKG::sql_bind_int(sqlite3_stmt *stmt, const int n, const int v) const{
  int res = sqlite3_bind_int(stmt, n, v);
  if (res!=SQLITE_OK) throw Err()
    << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
    << ": " << sqlite3_errstr(res);
}

std::string
GPKG::sql_column_text(sqlite3_stmt *stmt, int n) const{
  auto v = sqlite3_column_text(stmt, n);
  if (v == NULL) return std::string();
  return std::string((const char*)v);
}

std::string
GPKG::sql_column_name(sqlite3_stmt *stmt, int n) const{
  auto v = sqlite3_column_name(stmt, n);
//  if (v == NULL) throw Err()
//      << "can't get column name";
  if (v == NULL) return std::string();
  return std::string(v);
}