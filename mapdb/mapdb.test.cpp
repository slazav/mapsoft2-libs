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

    }

    // MapDBObj structure
    {
      MapDBObj o1,o2;
      // defaults
      assert_eq(o1.get_class(), MAPDB_POINT);
      assert_eq(o1.get_tnum(),  0);
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
      o1.name = "object name\nsecond line";
      o1.comm = "object comment\nsecond line";
      o1.tags.insert("object source\nsecond line");
      o1.set_coords("[[[0,0],[1,1]],[[1,1],[2,2]]]");
      assert_err(o1.set_coords("[0,0]"), "can't parse multisegment line: \"[0,0]\": a JSON array expected");
      assert_eq(dMultiLine(o1), dMultiLine("[[[0,0],[1,1]],[[1,1],[2,2]]]"));
      std::string pack = o1.pack();
      o2.unpack(pack);
      assert_eq(o1,o2);

      o1.set_type(MAPDB_POINT, 0x12);
      o1.angle  = 0;
      o1.name = "";
      o1.comm = "";
      o1.tags.clear();
      pack = o1.pack();
      o2.unpack(pack);
      assert_eq(o1,o2);

      o1.set_type(MAPDB_POINT, 0x2342);
      o1.set_coords("[1,1]");
      assert_err(o1.set_coords("[[0,0],[1,1]]"), "can't parse point: \"[[0,0],[1,1]]\": can't extract a number from array index 0");
      assert_eq(dMultiLine(o1), dMultiLine("[[[1,1]]]"));

    }


    // MapDB test
    {
      if (system("rm -rf tmp.db")!=0) throw Err() << "Can't delete tmp.db";
      MapDB m("tmp.db", 1);

      // mapinfo.db test

      // get/set name
      assert_eq(m.get_map_name(), "");
      m.set_map_name("");
      assert_eq(m.get_map_name(), "");
      m.set_map_name("Test");
      assert_eq(m.get_map_name(), "Test");

      // get/set border
      dMultiLine brd("[[[1,1],[2,2],[3,3]],[[4,4],[5,5]]]");
      assert_eq(m.get_map_brd(), dMultiLine());
      m.set_map_brd(brd);
      assert_eq(m.get_map_brd(), brd);

      // get version
      assert_eq(m.get_map_version(), 0);


      // get/set object
      MapDBObj o1;
      o1.set_type(MAPDB_LINE, 0x2342);
      o1.angle  = 60;
      o1.name = "object name\nsecond line";
      o1.comm = "object comment\nsecond line";
      o1.tags.insert("object source\nsecond line");

      assert_err(m.put(0,o1), "MapDB::put: object does not exists: 0");
      assert_err(m.del(0), "MapDB::del: object does not exists: 0");
      assert_err(m.get(0), "MapDB::get: object does not exists: 0");

      assert_eq(m.get_types().size(), 0);

      assert_err(m.add(o1), "MapDB::add: empty object");
      o1.dMultiLine::operator=(dMultiLine("[[0,0],[1,1]]"));

      // add/get object
      uint32_t id = m.add(o1);
      assert_eq(id, 0);
      assert_eq(o1, m.get(id));

      assert_eq(m.get_types().size(), 1);

      assert_err(m.put(1,o1), "MapDB::put: object does not exists: 1");
      assert_err(m.del(1), "MapDB::del: object does not exists: 1");
      assert_err(m.get(1), "MapDB::get: object does not exists: 1");

      // update object
      o1.set_type(MAPDB_LINE, 0x2342);
      o1.dMultiLine::operator=(dMultiLine());
      assert_err(m.put(id, o1), "MapDB::put: empty object");

      o1.dMultiLine::operator=(dMultiLine("[[[0,0],[1,1]],[[1,1],[2,2]]]"));
      m.put(id,o1);
      assert_eq(o1, m.get(id));

      // delete object
      m.del(id);
      assert_err(m.get(0), "MapDB::get: object does not exists: 0");

      // find
      id = m.add(o1);
      assert_eq(o1, m.get(id));
      assert_eq(m.find(o1.get_class(), o1.get_tnum()+1, dRect("[1,1,1,1]")).size(), 0);
      assert_eq(m.find(MAPDB_POINT, o1.get_tnum(), dRect("[1,1,1,1]")).size(), 0);
      assert_eq(m.find(o1.get_class(), o1.get_tnum(), dRect("[10,1,1,1]")).size(), 0);
      std::set<uint32_t> ii = m.find(o1.get_class(), o1.get_tnum(), dRect("[1,1,1,1]"));
      assert_eq(ii.size(),1);

      assert_eq(m.get_types().size(), 1);


    }
    if (system("rm -rf tmp.db")!=0) throw Err() << "Can't delete tmp.db";

  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
