#ifndef IMAGE_MBTILES_H
#define IMAGE_MBTILES_H

#include <sqlite3.h>
#include <sstream>
#include "opt/opt.h"
#include "cache/cache.h"
#include "geom/rect.h"
#include "image/image.h"
#include "image/image_r.h"
#include "filename/filename.h"

// Tiled images in MBTILES databases.

class ImageMBTiles: public Image {
  std::shared_ptr<sqlite3> db;
  Opt img_opts;
  bool readonly;

  // precompiled commands
  std::shared_ptr<sqlite3_stmt>
    stmt_meta_sel, stmt_meta_del, stmt_meta_ins, // get, delete, insert metadata field
    stmt_tile_sel, stmt_tile_del, stmt_tile_ins, // get, delete, insert tile data
    stmt_tile_lst, // list tiles
    stmt_ntile, stmt_ntilez, // get number of tiles (total/at given z)
    stmt_minz, stmt_maxz,    // get min/max zoom level
    stmt_x1, stmt_x2, stmt_y1, stmt_y2; // x and y tile bounds at some zoom level

  public:

    // Open database; If readonly=0 and file does not exist then create it.
    ImageMBTiles(const std::string & file, bool readonly);

    // Get metadata value (MBTILES). If value does not exist empty string is returned
    std::string get_metadata(const std::string & key) const;

    // Set metadata value (MBTILES)
    void set_metadata(const std::string & key, const std::string & val);

    // Set image options (for packing/unpacking images)
    void set_img_opts(const Opt & opt) {img_opts = opt;}

    // Get x-y-z tile
    ImageR get_tile(const size_t x, const size_t y, const size_t z) const;

    // Same with iPoint key
    ImageR get_tile(const iPoint & p) const {return get_tile(p.x, p.y, p.z);}

    // Put x-y-z tile (PNG only)
    void put_tile(const size_t x, const size_t y, const size_t z, const ImageR img);

    // Same with iPoint key
    void put_tile(const iPoint & p, const ImageR img) {put_tile(p.x, p.y, p.z, img);}

    // list of all tiles at the zoom level z
    std::vector<iPoint> tile_list(const int z);

    // Count tiles at zoom level z. If z<0 count all tiles
    int tile_number(const int z = -1) const;

    // Minimum value of zoom_level field in the tiles database
    int min_zoom() const;

    // Maximum value of zoom_level field in the tiles database
    int max_zoom() const;

    // Get tile bounds for zoom level z
    iRect tile_bounds(const int z) const;

    // Get wgs bounds for zoom level z
    dRect wgs_bounds(const int z) const;

    // Get wgs bounds for all zoom levels
    dRect wgs_bounds() const;

    // Update minzoom, maxzoom, center and bounds value in the metadata table
    // Run this after adding tiles
    void update_bounds();

    /****************************************************************/
    // Image interface

    // Set zoom level
    int zoom = 0;
    void set_zoom(const int z) {zoom = z;}
    int tsize;

    mutable Cache<iPoint, ImageR> cache;

    // get point color
    uint32_t get_argb(const size_t x, const size_t y) const override {
      iPoint key(x/tsize, y/tsize, zoom);
      iPoint crd(x%tsize, y%tsize);
      if (!cache.contains(key))
        cache.add(key, get_tile(key.x, key.y, key.z));
      auto & img = cache.get(key);
      if (img.is_empty()) return 0;
      return img.get_argb(crd.x, crd.y);
    }

    // get an image
    ImageR get_image(const iRect & r) const;

    std::ostream & print (std::ostream & s) const override{
      s << "ImageMBTiles(" << ")";
      return s;
    }

  private:

  std::shared_ptr<sqlite3_stmt> sql_prepare(const char * cmd) const{
    sqlite3_stmt *stmt;
    // cmd memory should be managed in the program!
    int res = sqlite3_prepare(db.get(), cmd, -1, &stmt, 0);
    if (res!=SQLITE_OK) throw Err()
    << "can't prepare sql command: " << cmd << ": " << sqlite3_errstr(res);
    return std::shared_ptr<sqlite3_stmt>(stmt, sqlite3_finalize);
  }

  void sql_bind_str(sqlite3_stmt *stmt, const int n, const std::string & str) const{
    int res = sqlite3_bind_text(stmt, n, str.c_str(), str.size(), SQLITE_STATIC);
    if (res!=SQLITE_OK) throw Err()
      << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
      << ": " << sqlite3_errstr(res);
  }

  void sql_bind_blob(sqlite3_stmt *stmt, const int n, const std::string & str) const{
    int res = sqlite3_bind_blob(stmt, n, str.c_str(), str.size(), SQLITE_STATIC);
    if (res!=SQLITE_OK) throw Err()
      << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
      << ": " << sqlite3_errstr(res);
  }

  void sql_bind_int(sqlite3_stmt *stmt, const int n, const int v) const{
    int res = sqlite3_bind_int(stmt, n, v);
    if (res!=SQLITE_OK) throw Err()
      << "can't bind sql command: " << sqlite3_expanded_sql(stmt)
      << ": " << sqlite3_errstr(res);
  }

  void sql_run_simple(sqlite3_stmt *stmt) const{
    int res = sqlite3_step(stmt);
    if (res!=SQLITE_DONE) throw Err()
      << "can't run sql command: " << sqlite3_expanded_sql(stmt)
      << ": " << sqlite3_errstr(res);
  }

  void sql_cmd_simple(const char *cmd) const{
    auto stmt = sql_prepare(cmd);
    sql_run_simple(stmt.get());
  }

};

#endif
