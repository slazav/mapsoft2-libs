#ifndef IMAGE_MBTILES_H
#define IMAGE_MBTILES_H

#include <sqlite3.h>
#include <sstream>
#include "opt/opt.h"
#include "geom/rect.h"
#include "geom/multiline.h"
#include "filename/filename.h"
#include "image_t.h"

// Tiled images in MBTILES databases.

class ImageMBTiles: public ImageT {
  std::shared_ptr<sqlite3> db;
  Opt opts;
  bool readonly;

  // precompiled commands
  std::shared_ptr<sqlite3_stmt>
    stmt_meta_sel, stmt_meta_del, stmt_meta_ins, // get, delete, insert metadata field
    stmt_tile_sel, stmt_tile_del, stmt_tile_ins, // get, delete, insert tile data
    stmt_tile_ex, stmt_tile_lst, // does tile exist?, list tiles
    stmt_ntile, stmt_ntilez, // get number of tiles (total/at given z)
    stmt_minz, stmt_maxz,    // get min/max zoom level
    stmt_x1, stmt_x2, stmt_y1, stmt_y2; // x and y tile bounds at some zoom level

  public:

    // Open database; If readonly=0 and file does not exist then create it.
    ImageMBTiles(const std::string & file, bool readonly);

    /*******************************************************/
    // ImageT interface

    // Set options
    void set_opt(const Opt & opt) override { ImageT::set_opt(opt); opts = opt; }

    // get a tile (without using cache)
    ImageR tile_read(const iPoint & key) const override;

    // check if tile exists
    bool tile_exists(const iPoint & key) const override;

    // get timestamp (or -1 if unavalable)
    virtual time_t tile_mtime(const iPoint & key) const override;

    // check if tile1 newer then tile2 OR tile2 does not exist
    bool tile_newer(const iPoint & key1, const iPoint & key2) const override;

    // write a tile
    void tile_write(const iPoint & key, const ImageR & img) override;

    // delete a tile
    void tile_delete(const iPoint & p) override;


    /*******************************************************/

    // Get metadata value (MBTILES). If value does not exist empty string is returned
    std::string get_metadata(const std::string & key) const;

    // Set metadata value (MBTILES)
    void set_metadata(const std::string & key, const std::string & val);


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

    // remove tiles outside border
    void crop(const dMultiLine & brd);

    // remove all empty space from database (useful after crop())
    void vacuum();

  private:

    std::shared_ptr<sqlite3_stmt> sql_prepare(const char * cmd) const;
    void sql_bind_str(sqlite3_stmt *stmt, const int n, const std::string & str) const;
    void sql_bind_blob(sqlite3_stmt *stmt, const int n, const std::string & str) const;
    void sql_bind_int(sqlite3_stmt *stmt, const int n, const int v) const;
    void sql_run_simple(sqlite3_stmt *stmt) const;
    void sql_cmd_simple(const char *cmd) const;

};

#endif
