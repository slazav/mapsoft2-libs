///\cond HIDDEN (do not show this in Doxyden)

#include <unistd.h>
#include <cassert>
#include <iostream>
#include "err/assert_err.h"
#include "shp.h"

int
main(){
  try{
    dMultiLine l1("[[[0,1],[1,2],[3,4]],[[0,0],[1,1]]]");
    dMultiLine l2("[[[1,2]]]");
    {
      // create new file
      Shp M("a", 1, MAP_LINE);
      assert_eq(M.get_type(), MAP_LINE);
      assert_eq(M.get_num(), 0);

      int id1 = M.put(l1);
      int id2 = M.put(l2);
      assert_eq(M.get_num(), 2);

      dMultiLine l1a = M.get(id1);
      dMultiLine l2a = M.get(id2);

      assert_eq(l1,l1a);
      assert_eq(l2,l2a);
      assert_eq(id1,0);
      assert_eq(id2,1);
    }
    {
      // open existing file
      Shp M("a", 0, MAP_LINE);
      assert_eq(M.get_type(), MAP_LINE);
      assert_eq(M.get_num(), 2);
      dMultiLine l1a = M.get(0);
      dMultiLine l2a = M.get(1);
      assert_eq(l1,l1a);
      assert_eq(l2,l2a);
    }
    unlink("a.shp");
    unlink("a.shx");
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond