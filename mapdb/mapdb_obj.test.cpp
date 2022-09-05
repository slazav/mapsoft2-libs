///\cond HIDDEN (do not show this in Doxyden)

#include <unistd.h>
#include <cassert>
#include <iostream>
#include "err/assert_err.h"
#include "mapdb.h"

using namespace std;

int
main(){
  try{

    // MapDBObj types
    {
      assert_eq(MapDBObj::make_type(MAPDB_POINT,   0xFFFF), 0xFFFF);
      assert_eq(MapDBObj::make_type(MAPDB_LINE,    0xFFFF), 0x100FFFF);
      assert_eq(MapDBObj::make_type(MAPDB_POLYGON, 0xFFFF), 0x200FFFF);

      assert_eq(MapDBObj::make_type("point:0xFFFF"), 0xFFFF);
      assert_eq(MapDBObj::make_type("line:0xFFFF"),  0x100FFFF);
      assert_eq(MapDBObj::make_type("area:0xFFFF"),  0x200FFFF);

      assert_eq(MapDBObj::make_type("point:123"), 123);

      std::string err("bad MapDB object type, (point|line|area):<number> expected: ");
      assert_err(MapDBObj::make_type(""), "can't parse MapDB object type: empty string");
      assert_err(MapDBObj::make_type("a"), "can't parse MapDB object type \"a\": ':' separator not found");
      assert_err(MapDBObj::make_type("point:"), "can't parse MapDB object type \"point:\": can't parse value: \"\"");
      assert_err(MapDBObj::make_type("point:a"), "can't parse MapDB object type \"point:a\": can't parse value: \"a\"");
      assert_err(MapDBObj::make_type("point:0xFFFFFF"), "can't parse MapDB object type \"point:0xFFFFFF\": too large number");
      assert_err(MapDBObj::make_type("abc:0xFFFF"), "can't parse MapDB object type \"abc:0xFFFF\": point, line, area, or text word expected");

      assert_eq(MapDBObj::print_type(0x123), "point:0x123");
      assert_eq(MapDBObj::print_type(0x100FFFF), "line:0xffff");
      assert_eq(MapDBObj::print_type(0x200FFFF), "area:0xffff");
      assert_eq(MapDBObj::print_type(0x300FFFF), "text:0xffff");
      assert_eq(MapDBObj::print_type(0x400FFFF), "unknown:0xffff");

      assert_eq(MapDBObj::print_type(0x312FFFF), "text:0xffff");
      assert_eq(MapDBObj::print_type(0x412FFFF), "unknown:0xffff");


    }

    // MapDBObj structure
    {
      MapDBObj o1,o2;
      // defaults
      assert_eq(o1.get_class(), MAPDB_POINT);
      assert_eq(o1.get_tnum(),  0);
      assert_eq(o1.scale,       1.0);
      assert_eq(o1.align,       MAPDB_ALIGN_SW);
      assert_eq(isnan(o1.angle), true);
      assert_eq(o1.name, "");
      assert_eq(o1.comm, "");
      assert_eq(o1.tags.size(), 0);
      assert_eq(o1.size(), 0);

      // <=> operators
      assert_eq(o1, o2);
      assert(o1 >= o2);
      assert(o1 <= o2);
      o2.set_type(MAPDB_LINE, 0);
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o2.set_type(MAPDB_LINE, 1);
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1; o2.angle = 10;
      assert(o1 != o2);
      assert(o1 < o2); // nan < then any number
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o1.name = "a";
      o2.name = "b";
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o1.comm = "a";
      o2.comm = "b";
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o1.tags.insert("a");
      o2.tags.insert("b");
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2.tags.insert("a");
      o1.tags.insert("b");
      assert_eq(o1, o2);
      assert(o1 <= o2);
      assert(o2 >= o1);

    }

    // packing and unpacking of MapDBObj
    // for OBJ database
    {
      MapDBObj o1,o2;
      o1.set_type(MAPDB_LINE, 0x2342);
      o1.angle  = 60;
      o1.scale  = 2.5;
      o1.align  = MAPDB_ALIGN_C;
      o1.name = "object name\nsecond line";
      o1.comm = "object comment\nsecond line";
      o1.tags.insert("object source\nsecond line");
      o1.children.insert(123);
      o1.children.insert(234);
      o1.set_coords("[[[0,0],[1,1]],[[1,1],[2,2]]]");
      assert_err(o1.set_coords("[0,0]"), "can't parse multisegment line: \"[0,0]\": a JSON array expected");
      assert_eq(dMultiLine(o1), dMultiLine("[[[0,0],[1,1]],[[1,1],[2,2]]]"));
      std::string pack = o1.pack();
      o2.unpack(pack);
      assert_eq(o1,o2);

      o1.set_type(MAPDB_POINT, 0x12);
      o1.angle  = 0;
      o1.scale  = 1.0;
      o1.align  = MAPDB_ALIGN_NW;
      o1.name = "";
      o1.comm = "";
      o1.tags.clear();
      o1.children.clear();
      pack = o1.pack();
      o2.unpack(pack);
      assert_eq(o1,o2);

      o1.set_type(MAPDB_POINT, 0x2342);
      o1.set_coords("[1,1]");
      assert_err(o1.set_coords("[[0,0],[1,1]]"), "can't parse point: \"[[0,0],[1,1]]\": can't extract a number from array index 0");
      assert_eq(dMultiLine(o1), dMultiLine("[[[1,1]]]"));

    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
