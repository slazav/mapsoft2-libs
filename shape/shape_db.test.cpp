///\cond HIDDEN (do not show this in Doxyden)

#include <unistd.h>
#include <cassert>
#include <iostream>
#include "err/assert_err.h"
#include "shape_db.h"

int
main(){
  try{
    dMultiLine l1("[[[1,2],[3,4]],[[0,0],[1,1]]]");
    dMultiLine l2("[[[1,2]]]");

      assert_err(ShapeDB("missing", 0, ShapeDB::LINE), "ShapeDB: can't open shp file: missing.shp");
      assert_err(ShapeDB("missing.shp", 0, ShapeDB::LINE), "ShapeDB: can't open shp file: missing.shp");
      assert_err(ShapeDB("missing.zip", 0, ShapeDB::LINE), "ShapeDB: can't open zip file: missing.zip");

    // create new file (second pass - overwrite existing one)
    for (int i = 0; i<2; ++i){

      ShapeDB M("a", 1, ShapeDB::LINE);

      assert_eq(M.shp_type(), ShapeDB::LINE);
      assert_eq(M.shp_num(), 0);
      assert_eq(M.shp_bbox(), dRect());

      int id1 = M.add(l1);
      assert_eq(id1,0);

      assert_eq(M.shp_type(), ShapeDB::LINE);
      assert_eq(M.shp_num(), 1);
      assert_eq(M.shp_bbox(), l1.bbox());

      int id2 = M.add(l2);
      assert_eq(id2,1);
      assert_eq(M.shp_num(), 2);

      assert_eq(l1,M.get(id1));
      assert_eq(l2,M.get(id2));

      // dbf
      assert_eq(M.dbf_num(), 0);
      assert_eq(M.dbf_field_num(), 0);
      assert_err(M.dbf_field_type(0), "ShapeDB: field number out of range: 0");
      assert_err(M.dbf_field_width(0), "ShapeDB: field number out of range: 0");
      assert_err(M.dbf_field_decimals(0), "ShapeDB: field number out of range: 0");
      assert_eq(M.dbf_field_find("aaa"), -1);

      assert_eq(M.dbf_field_add_str("f1", 6), 0);

      assert_eq(M.dbf_field_num(), 1);
      assert_eq(M.dbf_field_type(0), 'C');
      assert_eq(M.dbf_field_width(0), 6);
      assert_eq(M.dbf_field_decimals(0), 0);
      assert_eq(M.dbf_field_find("f1"), 0);

      M.dbf_put_str(0,0, "abcdefgh");
      assert_eq(M.dbf_get_str(0,0), "abcdef"); // truncated to width

//      M.dbf_put_str(1,0, "abcdefgh");
//      assert_eq(M.dbf_get_str(0,0), "abcdef"); // truncated to width

//      assert_eq(M.dbf_field_add_str("f2", 6), 1);


    }

    // same with zip file
    for (int i = 0; i<2; ++i){

      ShapeDB M("a.zip", 1, ShapeDB::LINE);

      assert_eq(M.shp_type(), ShapeDB::LINE);
      assert_eq(M.shp_num(), 0);
      assert_eq(M.shp_bbox(), dRect());

      int id1 = M.add(l1);
      assert_eq(id1,0);

      assert_eq(M.shp_type(), ShapeDB::LINE);
      assert_eq(M.shp_num(), 1);
      assert_eq(M.shp_bbox(), l1.bbox());

      int id2 = M.add(l2);
      assert_eq(id2,1);
      assert_eq(M.shp_num(), 2);

      assert_eq(l1,M.get(id1));
      assert_eq(l2,M.get(id2));
    }

    {
      // open existing file
      ShapeDB M("a", 0, ShapeDB::POINT); // type will be ignored
      assert_eq(M.shp_type(), ShapeDB::LINE);
      assert_eq(M.shp_num(), 2);
      assert_eq(M.shp_bbox(), l1.bbox());
      assert_eq(l1,M.get(0));
      assert_eq(l2,M.get(1));

      assert_eq(M.dbf_num(), 1);
      assert_eq(M.dbf_field_num(), 1);
      assert_eq(M.dbf_field_type(0), 'C');
      assert_eq(M.dbf_field_width(0), 6);
      assert_eq(M.dbf_field_decimals(0), 0);
      assert_eq(M.dbf_field_find("f1"), 0);

      assert_eq(M.dbf_get_str(0,0), "abcdef"); // truncated to width

    }

    {
      // same with zip
      ShapeDB M("a.zip", 0, ShapeDB::POINT); // type will be ignored
      assert_eq(M.shp_type(), ShapeDB::LINE);
      assert_eq(M.shp_num(), 2);
      assert_eq(M.shp_bbox(), l1.bbox());
      assert_eq(l1,M.get(0));
      assert_eq(l2,M.get(1));
    }

    unlink("a.shp");
    unlink("a.shx");
    unlink("a.dbf");
    unlink("a.zip");
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond