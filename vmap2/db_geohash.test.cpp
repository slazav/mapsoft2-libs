///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <unistd.h>
#include <iostream>
#include "err/assert_err.h"
#include "db_geohash.h"

int
main(){
  try{
    {
      // create new file
      unlink("a.dbh");
      GeoHashDB db("a.dbh", NULL, 1);

      assert_eq(db.get_types().size(), 0);
      assert_eq(db.bbox(), dRect());

      db.put(1, dRect(-0.01,-0.01, 0.02,0.02), 0);
      db.put(2, dRect(1,-0.01,     0.02,0.02), 0);
      db.put(3, dRect(-0.01,1,     0.02,0.02), 0);
      db.put(4, dRect(1,1,         0.01,0.01), 0);
      assert_eq(type_to_str(db.bbox()), "[-0.0439453125,-0.0439453125,1.09863281,1.09863281]");
      db.put(5, dRect(36,57,       0.01,0.01), 0);
      assert_eq(type_to_str(db.bbox()), "[-0.0439453125,-0.0439453125,36.0791016,57.0849609]");
      db.put(6, dRect(36,57,       0.11,0.11), 0);
      db.put(7, dRect(35,57,       0.11,0.11), 0);
      db.put(8, dRect(35,57,       0.10,0.10), 0);
      db.put(9, dRect(-100,-50,    220,100), 0);
      db.put(10, dRect(), 0); // empty range - do nothing
      db.put(11, dRect(36,57,0,0), 0);
      assert_eq(type_to_str(db.bbox()), "[-180,-90,360,180]");

      db.put(12, dRect(0,0,    0.1,0.1), 1);
      db.put(13, dRect(1,1,    0.1,0.1), 1);
      db.put(14, dRect(2,2,    0.1,0.1), 2);

      std::set<uint32_t> v1 = db.get(dRect(),0);
      assert_eq(v1.size(),0);

      v1 = db.get_types();
      assert_eq(v1.size(), 3);
      assert_eq(v1.count(0),1);
      assert_eq(v1.count(1),1);
      assert_eq(v1.count(2),1);
      assert_eq(v1.count(3),0);

      v1 = db.get(dRect(-0.01,-0.01, 1.12,1.12), 0);
      //for (auto i:v1) std::cerr << "> " << i << "\n";
      assert_eq(v1.size(),5);
      assert_eq(v1.count(1),1);
      assert_eq(v1.count(2),1);
      assert_eq(v1.count(3),1);
      assert_eq(v1.count(4),1);
      assert_eq(v1.count(9),1);

      v1 = db.get(dRect(-0.01,-0.01, 1.12,1.12),1);
      assert_eq(v1.size(),2);

      v1 = db.get(dRect(-0.01,-0.01, 2.12,2.12),2);
      assert_eq(v1.size(),1);

      v1 = db.get(dRect(-0.01,-0.01, 1.12,1.12),3);
      assert_eq(v1.size(),0);

      v1 = db.get(0);
      assert_eq(v1.size(),10);

      v1 = db.get(1);
      assert_eq(v1.size(),2);

      v1 = db.get(2);
      assert_eq(v1.size(),1);

      v1 = db.get(3);
      assert_eq(v1.size(),0);


      v1 = db.get(dRect(36,57, 0.001,0.001), 0);
      //for (auto i:v1) std::cerr << "> " << i << "\n";
      assert_eq(v1.size(),4);
      assert_eq(v1.count(5),1);
      assert_eq(v1.count(6),1);
      assert_eq(v1.count(9),1);
      assert_eq(v1.count(11),1);

      v1 = db.get(dRect(-180,-90, 360,180), 0);
      //for (auto i:v1) std::cerr << "> " << i << "\n";
      assert_eq(v1.size(),10);
    }
    {
      // open existing file
      GeoHashDB db("a.dbh", NULL, 0);
      std::set<uint32_t> v1 = db.get(dRect(36,57, 0.001,0.001), 0);
      //for (auto i:v1) std::cerr << "> " << i << "\n";
      assert_eq(v1.size(),4);
      assert_eq(v1.count(5),1);
      assert_eq(v1.count(6),1);
      assert_eq(v1.count(9),1);
      assert_eq(v1.count(11),1);

      db.del(9, dRect(-100,-50, 220,100), 0);
      db.del(5, dRect(-100,-50, 220,100), 1);
      db.put(9, dRect(-100,-50, 220,100), 1);
      //for (auto i:v1) std::cerr << "> " << i << "\n";
      v1 = db.get(dRect(36,57, 0.001,0.001), 0);
      assert_eq(v1.size(),3);
      assert_eq(v1.count(5),1);
      assert_eq(v1.count(6),1);
      assert_eq(v1.count(11),1);

      v1 = db.get(dRect(36,57, 0.001,0.001), 1);
      assert_eq(v1.size(),1);
      assert_eq(v1.count(9),1);

      db.del(5, dRect(36,57,0.01,0.01), 0);
      v1 = db.get(dRect(36,57, 0.001,0.001), 0);
      assert_eq(v1.size(),2);
      assert_eq(v1.count(6),1);
      assert_eq(v1.count(11),1);

      // non-existing id-type-range combinations!
      db.del(6, dRect(36,57,0.01,0.01), 0);
      db.del(6, dRect(36,57,0.11,0.11), 1);
      v1 = db.get(dRect(36,57, 0.001,0.001), 0);
      assert_eq(v1.size(),2);
      assert_eq(v1.count(6),1);
      assert_eq(v1.count(11),1);

      v1 = db.get_types();
      assert_eq(v1.size(), 3);
      assert_eq(v1.count(0),1);
      assert_eq(v1.count(1),1);
      assert_eq(v1.count(2),1);
    }
    unlink("a.dbh");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
