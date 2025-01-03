#include <sstream>
#include "image_t_mbtiles.h"
#include "geo_tiles/geo_tiles.h"
#include "geom/poly_tools.h"
#include "filename/filename.h"
#include "image/io.h"
#include "image/io_png.h"

ImageMBTiles::ImageMBTiles(const std::string & file, bool readonly):
       readonly(readonly), ImageT(file, true, 256, 0, 16){

  int flags = readonly? SQLITE_OPEN_READONLY :
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

  if (readonly && !file_exists(file)) throw Err()
    << "can't open sqlite database: " << file << ": file does not exists";

  bool newdb = !file_exists(file);

  sqlite3 *pdb;
  int res;
  res = sqlite3_open_v2(file.c_str(), &pdb, flags, NULL);
  if (res != SQLITE_OK){
    sqlite3_close(pdb);
    throw Err() << "can't open sqlite database: "
                << file << ": " << sqlite3_errstr(res);
  }
  db = std::shared_ptr<sqlite3>(pdb, sqlite3_close);

  if (newdb){
    // create MBTILES database (according with spec version 1.3)
    sql_cmd_simple("CREATE TABLE tiles (tile_column int, tile_row int,"
                   "  zoom_level int, tile_data blob,"
                   "  PRIMARY KEY (tile_column, tile_row, zoom_level))");
    sql_cmd_simple("CREATE UNIQUE INDEX tile_index on tiles (zoom_level, tile_column, tile_row)");
    sql_cmd_simple("CREATE TABLE metadata (name text, value text)");

    //sql_cmd_simple("PRAGMA journal_mode = OFF");
    //sql_cmd_simple("PRAGMA synchronous = 0");

    //The metadata table MUST contain these two rows:
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('name', '')");
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('format', 'png')"); // pbf, jpg, png, webp ...

    // The metadata table SHOULD contain these four rows:
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('bounds', '-180.0,-85,180,85')");
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('center', '0,0')");
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('minzoom', '0')");
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('maxzoom', '16')");

    // The metadata table MAY contain these four rows:
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('attribution', '')");
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('description', '')");
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('type', 'overlay')"); // overlay or baselayer
    sql_cmd_simple("INSERT INTO metadata (name, value) VALUES ('version', '0')");
  }

  // set default image options
  opts["fmt"] = "png";

  // precompiled statements
  stmt_meta_sel = sql_prepare("SELECT value FROM metadata WHERE name = ?");
  stmt_meta_del = sql_prepare("DELETE FROM metadata WHERE name = ?");
  stmt_meta_ins = sql_prepare("INSERT INTO metadata (name, value) VALUES (?,?)");

  stmt_tile_sel = sql_prepare("SELECT tile_data FROM tiles "
                              "WHERE tile_column=? AND tile_row=? AND zoom_level=?");

  stmt_tile_ex = sql_prepare("SELECT 1 FROM tiles "
                              "WHERE tile_column=? AND tile_row=? AND zoom_level=?");

  stmt_tile_del = sql_prepare("DELETE FROM tiles "
                              "WHERE tile_column=? AND tile_row=? AND zoom_level=?");

  stmt_tile_ins = sql_prepare("INSERT INTO tiles "
                              "(tile_column, tile_row, zoom_level, tile_data) "
                              "VALUES (?,?,?,?)");

  stmt_tile_lst = sql_prepare("SELECT tile_column, tile_row, zoom_level FROM tiles "
                              "WHERE zoom_level=?");

  stmt_ntile  = sql_prepare("SELECT COUNT(1) FROM tiles");
  stmt_ntilez = sql_prepare("SELECT COUNT(1) FROM tiles WHERE zoom_level=?");

  stmt_minz = sql_prepare("SELECT MIN(zoom_level) FROM tiles");
  stmt_maxz = sql_prepare("SELECT MAX(zoom_level) FROM tiles");

  stmt_x1 = sql_prepare("SELECT MIN(tile_column) FROM tiles WHERE zoom_level=?");
  stmt_x2 = sql_prepare("SELECT MAX(tile_column) FROM tiles WHERE zoom_level=?");
  stmt_y1 = sql_prepare("SELECT MIN(tile_row) FROM tiles WHERE zoom_level=?");
  stmt_y2 = sql_prepare("SELECT MAX(tile_row) FROM tiles WHERE zoom_level=?");

}

std::string
ImageMBTiles::get_metadata(const std::string & key) const {
  auto cmd = stmt_meta_sel.get();
  sqlite3_reset(cmd);
  sql_bind_str(cmd, 1, key);

  // Return a single string from row=1,column=1, or empty string
  // is data is missing.
  if (sqlite3_step(cmd)!=SQLITE_ROW) return std::string();
  return (const char *)sqlite3_column_text(cmd, 0);

}

void
ImageMBTiles::set_metadata(const std::string & key, const std::string & val){
  if (readonly) throw Err() << "can't write to read-only database";

  auto stmt1 = stmt_meta_del.get();
  auto stmt2 = stmt_meta_ins.get();
  sqlite3_reset(stmt1);
  sqlite3_reset(stmt2);

  sql_bind_str(stmt1, 1, key);
  sql_bind_str(stmt2, 1, key);
  sql_bind_str(stmt2, 2, val);

  sql_run_simple(stmt1);
  sql_run_simple(stmt2);
}

void
ImageMBTiles::tile_write(const iPoint & key, const ImageR & img){
  if (readonly) throw Err() << "can't write to read-only database";

  std::string data;
  {
    std::ostringstream str;
    image_save_png(img, str, opts);
    data = str.str(); // save data
  }

  auto stmt1 = stmt_tile_del.get();
  auto stmt2 = stmt_tile_ins.get();
  sqlite3_reset(stmt1);
  sqlite3_reset(stmt2);

  sql_bind_int(stmt1, 1, key.x);
  sql_bind_int(stmt1, 2, key.y);
  sql_bind_int(stmt1, 3, key.z);

  sql_bind_int(stmt2, 1, key.x);
  sql_bind_int(stmt2, 2, key.y);
  sql_bind_int(stmt2, 3, key.z);
  sql_bind_blob(stmt2, 4, data);

  sql_run_simple(stmt1);
  sql_run_simple(stmt2);
}

ImageR
ImageMBTiles::tile_read(const iPoint & key) const {

  auto stmt = stmt_tile_sel.get();
  sqlite3_reset(stmt);

  sql_bind_int(stmt, 1, key.x);
  sql_bind_int(stmt, 2, key.y);
  sql_bind_int(stmt, 3, key.z);

  // no data - return empty image
  if (sqlite3_step(stmt)!=SQLITE_ROW) return ImageR();

  // wrong data type
  if (sqlite3_column_type(stmt, 0) != SQLITE_BLOB)
    throw Err() << "blob data expected: " << sqlite3_expanded_sql(stmt);

  // obtain data
  std::string data((const char *)sqlite3_column_blob(stmt, 0),
                                 sqlite3_column_bytes(stmt, 0));

  std::istringstream str(data);
  return image_load(str);
}

bool
ImageMBTiles::tile_exists(const iPoint & key) const {

  auto stmt = stmt_tile_ex.get();
  sqlite3_reset(stmt);
  sql_bind_int(stmt, 1, key.x);
  sql_bind_int(stmt, 2, key.y);
  sql_bind_int(stmt, 3, key.z);

  // no data - return empty image
  return sqlite3_step(stmt)==SQLITE_ROW;
}

void
ImageMBTiles::tile_delete(const iPoint & key){
  if (readonly) throw Err() << "can't write to read-only database";

  auto stmt = stmt_tile_del.get();
  sqlite3_reset(stmt);
  sql_bind_int(stmt, 1, key.x);
  sql_bind_int(stmt, 2, key.y);
  sql_bind_int(stmt, 3, key.z);
  sql_run_simple(stmt);
}

std::vector<iPoint>
ImageMBTiles::tile_list(const int z){
  auto stmt = stmt_tile_lst.get();
  sqlite3_reset(stmt);
  sql_bind_int(stmt, 1, z);

  std::vector<iPoint> ret;
  while (1){
    int res = sqlite3_step(stmt);
    if (res==SQLITE_DONE) break;

    if (res!=SQLITE_ROW)
      throw Err() << "can't read tile list: " << sqlite3_errstr(res);

    ret.emplace_back(
      sqlite3_column_int(stmt, 0),
      sqlite3_column_int(stmt, 1),
      sqlite3_column_int(stmt, 2)
   );
  }
  return ret;
}

int
ImageMBTiles::tile_number(const int z) const {

  auto stmt = (z>=0) ? stmt_ntilez.get() : stmt_ntile.get();
  sqlite3_reset(stmt);

  if (z>=0) sql_bind_int(stmt, 1, z);

  int res = sqlite3_step(stmt);
  if (res!=SQLITE_ROW)
    throw Err() << "can't count tiles: " << sqlite3_errstr(res);

  return sqlite3_column_int(stmt, 0);
}

int
ImageMBTiles::min_zoom() const {
  auto stmt = stmt_minz.get();
  sqlite3_reset(stmt);

  int res = sqlite3_step(stmt);
  if (res!=SQLITE_ROW)
    throw Err() << "can't get min zoom: " << sqlite3_errstr(res);
  if (sqlite3_column_type(stmt, 0) != SQLITE_INTEGER)
    return -1;

  return sqlite3_column_int(stmt, 0);
}

int
ImageMBTiles::max_zoom() const {
  auto stmt = stmt_maxz.get();
  sqlite3_reset(stmt);

  int res = sqlite3_step(stmt);
  if (res!=SQLITE_ROW)
    throw Err() << "can't get max zoom: " << sqlite3_errstr(res);
  if (sqlite3_column_type(stmt, 0) != SQLITE_INTEGER)
    return -1;

  return sqlite3_column_int(stmt, 0);
}


iRect
ImageMBTiles::tile_bounds(const int z) const {
  auto x1 = stmt_x1.get(); sqlite3_reset(x1);
  auto x2 = stmt_x2.get(); sqlite3_reset(x2);
  auto y1 = stmt_y1.get(); sqlite3_reset(y1);
  auto y2 = stmt_y2.get(); sqlite3_reset(y2);

  sql_bind_int(x1, 1, z);
  sql_bind_int(x2, 1, z);
  sql_bind_int(y1, 1, z);
  sql_bind_int(y2, 1, z);

  if (sqlite3_step(x1)!=SQLITE_ROW) return iRect();
  if (sqlite3_column_type(x1, 0) != SQLITE_INTEGER) return iRect();
  int x1v = sqlite3_column_int(x1, 0);

  if (sqlite3_step(x2)!=SQLITE_ROW) return iRect();
  if (sqlite3_column_type(x2, 0) != SQLITE_INTEGER) return iRect();
  int x2v = sqlite3_column_int(x2, 0);

  if (sqlite3_step(y1)!=SQLITE_ROW) return iRect();
  if (sqlite3_column_type(y1, 0) != SQLITE_INTEGER) return iRect();
  int y1v = sqlite3_column_int(y1, 0);

  if (sqlite3_step(y2)!=SQLITE_ROW) return iRect();
  if (sqlite3_column_type(y2, 0) != SQLITE_INTEGER) return iRect();
  int y2v = sqlite3_column_int(y2, 0);

  return iRect(iPoint(x1v,y1v), iPoint(x2v+1, y2v+1));
}

dRect
ImageMBTiles::wgs_bounds(const int z) const {
  const int tsize = 256;
  const GeoTiles tcalc(tsize);
  iRect r = tile_bounds(z);
  iPoint p1 = r.tlc()*tsize;
  iPoint p2 = r.brc()*tsize;
  return dRect(
    tcalc.m_to_ll(tcalc.px_to_m(p1, z)),
    tcalc.m_to_ll(tcalc.px_to_m(p2, z))
  );
}

dRect
ImageMBTiles::wgs_bounds() const {
  dRect bnds;
  int minz = min_zoom();
  int maxz = max_zoom();
  for (int z=minz; z<=maxz; ++z) bnds.expand(wgs_bounds(z));
  return bnds;
}

void
ImageMBTiles::update_bounds() {
  int minz = min_zoom();
  int maxz = max_zoom();

  if (minz<0 || maxz<0)
    throw Err() << "can't update bounds (no tiles?)";

  int tsize = 256;
  GeoTiles tcalc(tsize);

  dRect bnds;
  for (int z=minz; z<=maxz; ++z) bnds.expand(wgs_bounds(z));

  if (bnds.is_empty())
    throw Err() << "can't update bounds (no tiles?)";

  std::ostringstream s1, s2;
  s1 << bnds.x << "," << bnds.y << ","
     << bnds.x+bnds.w << "," << bnds.y+bnds.h;
  s2 << bnds.cnt().x << "," << bnds.cnt().y;

  set_metadata("bounds", s1.str());
  set_metadata("center", s2.str());
  set_metadata("minzoom", type_to_str(minz));
  set_metadata("maxzoom", type_to_str(maxz));
}

void
ImageMBTiles::crop(const dMultiLine & brd) {
  const GeoTiles tcalc(tsize);
  for (int z=min_zoom(); z<=max_zoom(); ++z){
    for (auto tile:tile_list(z)){
      dRect trange = tcalc.tile_to_range(tile, z);
      if (brd.size() && rect_in_polygon(trange, brd) == 0) tile_delete(tile);
    }
  }
}

void
ImageMBTiles::vacuum() {
  auto stmt = sql_prepare("VACUUM");
  sql_run_simple(stmt.get());
}

/**********************************************************/
// Private functions

std::shared_ptr<sqlite3_stmt>
ImageMBTiles::sql_prepare(const char * cmd) const{
  sqlite3_stmt *stmt;
  // cmd memory should be managed in the program!
  int res = sqlite3_prepare(db.get(), cmd, -1, &stmt, 0);
  if (res!=SQLITE_OK) throw Err()
  << "can't prepare sql command: " << cmd << ": " << sqlite3_errstr(res);
  return std::shared_ptr<sqlite3_stmt>(stmt, sqlite3_finalize);
}

void
ImageMBTiles::sql_bind_str(sqlite3_stmt *stmt, const int n, const std::string & str) const{
  int res = sqlite3_bind_text(stmt, n, str.c_str(), str.size(), SQLITE_STATIC);
  if (res!=SQLITE_OK) throw Err()
    << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
    << ": " << sqlite3_errstr(res);
}

void
ImageMBTiles::sql_bind_blob(sqlite3_stmt *stmt, const int n, const std::string & str) const{
  int res = sqlite3_bind_blob(stmt, n, str.c_str(), str.size(), SQLITE_STATIC);
  if (res!=SQLITE_OK) throw Err()
    << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
    << ": " << sqlite3_errstr(res);
}

void
ImageMBTiles::sql_bind_int(sqlite3_stmt *stmt, const int n, const int v) const{
  int res = sqlite3_bind_int(stmt, n, v);
  if (res!=SQLITE_OK) throw Err()
    << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
    << ": " << sqlite3_errstr(res);
}

void
ImageMBTiles::sql_run_simple(sqlite3_stmt *stmt) const{
  int res = sqlite3_step(stmt);
  if (res!=SQLITE_DONE) throw Err()
    << "can't run sql command: " << sqlite3_expanded_sql(stmt)
    << ": " << sqlite3_errstr(res);
}

void
ImageMBTiles::sql_cmd_simple(const char *cmd) const{
  auto stmt = sql_prepare(cmd);
  sql_run_simple(stmt.get());
}

