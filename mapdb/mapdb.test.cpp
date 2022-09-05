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

    // MapDB test
    {
      MapDB::delete_db("tmp.db");
      MapDB m("tmp.db", 1);

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
    MapDB::delete_db("tmp.db");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
