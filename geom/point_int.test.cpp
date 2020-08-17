///\cond HIDDEN (do not show this in Doxyden)

#include "err/assert_err.h"
#include "point_int.h"

int
main(){
  try{

    // adjacent
    assert_eq(adjacent(iPoint(10,10),0), iPoint( 9, 9) );
    assert_eq(adjacent(iPoint(10,10),1), iPoint(10, 9) );
    assert_eq(adjacent(iPoint(10,10),2), iPoint(11, 9) );
    assert_eq(adjacent(iPoint(10,10),3), iPoint(11,10) );
    assert_eq(adjacent(iPoint(10,10),4), iPoint(11,11) );
    assert_eq(adjacent(iPoint(10,10),5), iPoint(10,11) );
    assert_eq(adjacent(iPoint(10,10),6), iPoint( 9,11) );
    assert_eq(adjacent(iPoint(10,10),7), iPoint( 9,10) );
    assert_eq(adjacent(iPoint(10,10),8), iPoint( 9, 9) );
    assert_eq(adjacent(iPoint(10,10),-1), iPoint(9,9) );

    // is_adjacent
    assert_eq(is_adjacent(iPoint(10,10), iPoint( 9, 9)), 0 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint(10, 9)), 1 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint(11, 9)), 2 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint(11,10)), 3 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint(11,11)), 4 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint(10,11)), 5 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint( 9,11)), 6 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint( 9,10)), 7 );
    assert_eq(is_adjacent(iPoint(10,10), iPoint(10,10)), -1);
    assert_eq(is_adjacent(iPoint(10,10), iPoint(10,12)), -1);
    assert_eq(is_adjacent(iPoint(10,10), iPoint(12,10)), -1);

    // border
    {
      std::set<iPoint> s, b;
      assert_eq(border_set(s).size(), 0);
      s.insert(iPoint(10,10));
      b = border_set(s);
      assert_eq(b.size(), 8);
      for (int d=0; d<8; ++d){
        auto p = adjacent(iPoint(10,10),d);
        assert_eq(b.count(p), 1);
      }
      s.insert(iPoint(10,11));
      b = border_set(s);
      assert_eq(b.size(), 10);
      s.insert(iPoint(11,12));
      b = border_set(s);
      assert_eq(b.size(), 14);
      assert_eq(b.count(iPoint(10,11)), 0);
      assert_eq(b.count(iPoint(12,13)), 1);
      s.insert(iPoint(20,20));
      b = border_set(s);
      assert_eq(b.size(), 14+8);
    }

    // same with add_set_and_border
    {
      std::set<iPoint> s, b;
      assert_eq(border_set(s).size(), 0);
      add_set_and_border(iPoint(10,10), s, b);

      assert_eq(b.size(), 8);
      for (int d=0; d<8; ++d){
        auto p = adjacent(iPoint(10,10),d);
        assert_eq(b.count(p), 1);
      }
      add_set_and_border(iPoint(10,11), s, b);
      assert_eq(b.size(), 10);
      add_set_and_border(iPoint(11,12), s, b);
      assert_eq(b.size(), 14);
      assert_eq(b.count(iPoint(10,11)), 0);
      assert_eq(b.count(iPoint(12,13)), 1);
      add_set_and_border(iPoint(20,20), s, b);
      assert_eq(b.size(), 14+8);
      assert_eq(add_set_and_border(iPoint(20,20), s, b), false);
      assert_eq(add_set_and_border(iPoint(21,20), s, b), true);
    }

    { // border_line
      std::set<iPoint> s;
      auto l = border_line(s);
      assert_eq(l.size(), 0);
      s.insert(iPoint(10,10));
      l = border_line(s);
      assert_eq(l, iMultiLine("[[[11,10],[11,11],[10,11],[10,10],[11,10]]]"));

      s.insert(iPoint(10,11));
      l = border_line(s);
      assert_eq(l, iMultiLine("[[[11,12],[10,12],[10,10],[11,10],[11,12]]]"));

      s.insert(iPoint(11,12));
      l = border_line(s);
      assert_eq(l, iMultiLine("[[[12,12],[12,13],[11,13],[11,12],[10,12],[10,10],[11,10],[11,12],[12,12]]]"));

      s.insert(iPoint(20,20));
      l = border_line(s);
      assert_eq(l, iMultiLine("[[[12,12],[12,13],[11,13],[11,12],[10,12],[10,10],[11,10],[11,12],[12,12]],"
                              "[[21,20],[21,21],[20,21],[20,20],[21,20]]]"));
      // std::cerr << "> " << l.size() << ": " << l << "\n";

    }

    { // bres_line
      auto s = bres_line(iPoint(0,0), iPoint(3,2), 0,0);
      assert_eq(s.size(), 4);
      assert_eq(s.count(iPoint(0,0)), 1);
      assert_eq(s.count(iPoint(1,1)), 1);
      assert_eq(s.count(iPoint(2,1)), 1);
      assert_eq(s.count(iPoint(3,2)), 1);
      // more tests needed? other directions, thickness, shift...
    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
