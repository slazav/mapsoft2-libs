///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include <unistd.h>
#include "image_t_mbtiles.h"
#include "err/assert_err.h"

int
main(){
  try{

    // create new MBTILES database:
    {
      unlink("db_test_new.mbtiles");
      ImageMBTiles db("db_test_new.mbtiles", 0);

      // read default metadata fields
      assert_eq(db.get_metadata("maxzoom"), "16");
      assert_eq(db.get_metadata("bounds"), "-180.0,-85,180,85");
      assert_eq(db.get_metadata("non-std"), "");

      // add/replace metadata fields
      db.set_metadata("maxzoom", "14");
      db.set_metadata("non-std", "new value");

      // check updated fields
      assert_eq(db.get_metadata("maxzoom"), "14");
      assert_eq(db.get_metadata("non-std"), "new value");

      assert_eq(db.tile_number(), 0);
      assert_eq(db.min_zoom(), -1);
      assert_eq(db.max_zoom(), -1);

      // put an image
      ImageR img(256,256, IMAGE_32ARGB);
      img.fill32(0xFFFF0000);
      img.set32(128,128,0xFF00FF00);

      db.tile_write(iPoint(5741,5035,13), img);
      assert_eq(db.tile_exists(iPoint(5741,5035,13)), true);
      assert_eq(db.tile_exists(iPoint(5741,5035,12)), false);

      assert_feq(db.tile_mtime(iPoint(5741,5035,13)), time(NULL), 2);
      assert_eq(db.tile_mtime(iPoint(5741,5035,12)), -1);

      ImageR img1 = db.tile_read(iPoint(1001,1000,10));
      assert_eq(img1.is_empty(), true);

      ImageR img2 = db.tile_read(iPoint(5741,5035,13));
      assert_eq(img2.is_empty(), false);
      assert_eq(img2.get32(0,0), 0xFFFF0000);
      assert_eq(img2.get32(128,128), 0xFF00FF00);

      // get_argb
      db.set_zoom(13);
      assert_eq(db.get_argb(5741*256, 5035*256), 0xFFFF0000);
      assert_eq(db.get_argb(5741*256+128, 5035*256+128), 0xFF00FF00);
      assert_eq(db.get_argb(5740*256, 5035*256), 0);
      assert_eq(db.get_argb(0,0), 0);


      // tile number
      assert_eq(db.tile_number(), 1);
      assert_eq(db.tile_number(-1), 1);
      assert_eq(db.tile_number(13), 1);
      assert_eq(db.tile_number(9),  0);

      // min/max zoom
      assert_eq(db.min_zoom(), 13);
      assert_eq(db.max_zoom(), 13);

      assert_eq(db.tile_bounds(9), iRect());
      assert_eq(db.tile_bounds(13), iRect(5741,5035,1,1));
      assert_deq(db.wgs_bounds(), dRect(72.2900391,38.0999826,0.0439453125,0.0345739297), 1e-6);

      // update bounds
      db.update_bounds();
      assert_eq(db.get_metadata("bounds"), "72.29,38.1,72.334,38.1346");
      assert_eq(db.get_metadata("center"), "72.312,38.1173");
      assert_eq(db.get_metadata("maxzoom"), "13");
      assert_eq(db.get_metadata("minzoom"), "13");

      // tile list
      auto lst10 = db.tile_list(10);
      assert_eq(lst10.size(), 0);

      auto lst13 = db.tile_list(13);
      assert_eq(lst13.size(), 1);
      assert_eq(lst13[0], iPoint(5741,5035,13));

    }

    // re-open database
    {
      ImageMBTiles db("db_test_new.mbtiles", 0);

      // check updated fields
      assert_eq(db.get_metadata("maxzoom"), "13");
      assert_eq(db.get_metadata("non-std"), "new value");

      // replace metadata
      db.set_metadata("maxzoom", "15");
      assert_eq(db.get_metadata("maxzoom"), "15");

      // number of tiles
      assert_eq(db.tile_number(-1), 1);
      assert_eq(db.tile_number(13), 1);
      assert_eq(db.tile_number(9),  0);
    }

    // re-open database read-only
    {
      ImageMBTiles db("db_test_new.mbtiles", 1);

      // try to change information
      assert_err(db.set_metadata("maxzoom", "18"), "can't write to read-only database");
      assert_err(db.tile_write(iPoint(5741,5035,13),ImageR()), "can't write to read-only database");

      // check updated fields
      assert_eq(db.get_metadata("maxzoom"), "15");
      assert_eq(db.get_metadata("non-std"), "new value");

      // number of tiles
      assert_eq(db.tile_number(-1), 1);
      assert_eq(db.tile_number(13), 1);
      assert_eq(db.tile_number(9),  0);
    }

    // try to open non-existing database read-only
    {
      unlink("db_test_new.mbtiles");
      assert_err(ImageMBTiles db("db_test_new.mbtiles", 1),
        "can't open sqlite database: db_test_new.mbtiles: file does not exists");
    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond