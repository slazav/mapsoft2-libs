///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "line_walker.h"

int
main(){
  try{

    dLine l1("[[0,0,0],[2,0,1],[2,2,2],[3,2],[4,3],[5,4]]");
    LineWalker lw(l1);

    // we are at the beginning
    assert_feq(lw.length(), 5.0+2*sqrt(2.0), 1e-6);
    assert_feq(lw.dist(), 0.0, 1e-6);
    assert_deq(lw.norm(), dPoint(0,1), 1e-6);
    assert_deq(lw.tang(), dPoint(1,0), 1e-6);
    assert_feq(lw.ang(), 0.0, 1e-6);
    assert_deq(lw.pt(), dPoint(0,0), 1e-6);

    // start moving alone the line, check dist, norm, tang, pt
    lw.move_frw(0.5);
    assert_feq(lw.dist(), 0.5, 1e-6);
    assert_deq(lw.norm(), dPoint(0,1), 1e-6);
    assert_deq(lw.tang(), dPoint(1,0), 1e-6);
    assert_feq(lw.ang(), 0.0, 1e-6);
    assert_deq(lw.pt(), dPoint(0.5,0), 1e-6);

    lw.move_frw(1.5);
    assert_feq(lw.dist(), 2, 1e-6);
    assert_deq(lw.norm(), dPoint(0,1), 1e-6);
    assert_deq(lw.tang(), dPoint(1,0), 1e-6);
    assert_feq(lw.ang(), 0.0, 1e-6);
    assert_deq(lw.pt(), dPoint(2,0), 1e-6);

    lw.move_frw(1);
    assert_feq(lw.dist(), 3, 1e-6);
    assert_deq(lw.norm(), dPoint(-1,0), 1e-6);
    assert_deq(lw.tang(), dPoint(0,1), 1e-6);
    assert_feq(lw.ang(), M_PI/2.0, 1e-6);
    assert_deq(lw.pt(), dPoint(2,1), 1e-6);

    lw.move_bck(0.5);
    assert_eq(lw.is_begin(), false);
    assert_eq(lw.is_end(), false);
    assert_feq(lw.dist(), 2.5, 1e-6);
    assert_deq(lw.norm(), dPoint(-1,0), 1e-6);
    assert_deq(lw.tang(), dPoint(0,1), 1e-6);
    assert_feq(lw.ang(), M_PI/2.0, 1e-6);
    assert_deq(lw.pt(), dPoint(2,0.5), 1e-6);

    // move to the beginning
    lw.move_begin();
    assert_eq(lw.is_begin(), true);
    assert_eq(lw.is_end(), false);
    assert_feq(lw.dist(), 0, 1e-6);
    assert_deq(lw.pt(), dPoint(0,0), 1e-6);

    // moving back from the beginning does not change anything
    lw.move_bck_to_node();
    assert_feq(lw.dist(), 0, 1e-6);
    assert_deq(lw.pt(), dPoint(0,0), 1e-6);

    lw.move_bck(1);
    assert_feq(lw.dist(), 0, 1e-6);
    assert_deq(lw.pt(), dPoint(0,0), 1e-6);

    // move to the end
    lw.move_end();
    assert_eq(lw.is_begin(), false);
    assert_eq(lw.is_end(), true);
    assert_feq(lw.dist(), lw.length(), 1e-6);
    assert_deq(lw.pt(), dPoint(5,4), 1e-6);
    assert_deq(lw.norm(), rotate2d(dPoint(0,1),dPoint(),M_PI/4), 1e-6);
    assert_deq(lw.tang(), rotate2d(dPoint(1,0),dPoint(),M_PI/4), 1e-6);
    assert_feq(lw.ang(), M_PI/4.0, 1e-6);

    // moving forward does not change anything
    lw.move_frw_to_node();
    assert_feq(lw.dist(), lw.length(), 1e-6);
    assert_deq(lw.pt(), dPoint(5,4), 1e-6);

    lw.move_frw(1);
    assert_feq(lw.dist(), lw.length(), 1e-6);
    assert_deq(lw.pt(), dPoint(5,4), 1e-6);

    // check move_bck_to_node, move_frw_to_node
    lw.move_bck_to_node();
    assert_feq(lw.dist(), lw.length()-sqrt(2), 1e-6);
    assert_deq(lw.pt(), dPoint(4,3), 1e-6);

    lw.move_frw_to_node();
    assert_feq(lw.dist(), lw.length(), 1e-6);
    assert_deq(lw.pt(), dPoint(5,4), 1e-6);

    // check get_points()
    assert_deq(lw.get_points(1), dLine("[[5,4]]"), 1e-6);
    assert_deq(lw.pt(), dPoint(5,4), 1e-6);

    lw.move_begin();
    assert_deq(lw.get_points(1), dLine("[[0,0],[1,0]]"), 1e-6);
    assert_deq(lw.pt(), dPoint(1,0), 1e-6);

    assert_deq(lw.get_points(2), dLine("[[1,0],[2,0],[2,1]]"), 1e-6);
    assert_deq(lw.pt(), dPoint(2,1), 1e-6);

    assert_deq(lw.get_points(6), dLine("[[2,1],[2,2],[3,2],[4,3],[5,4]]"), 1e-6);
    assert_eq(lw.is_end(), true);

    // Empty line
    dLine el;
    assert_err(LineWalker elw(el), "LineWalker: empty line");

    { // 1 point
      LineWalker lw(dLine("[[1,1]]"));
      assert_eq(lw.is_begin(), 1);
      assert_eq(lw.is_end(), 1);
      assert_eq(lw.length(), 0.0);
      assert_eq(lw.dist(), 0.0);
      assert_deq(lw.norm(), dPoint(0,1), 1e-6);
      assert_deq(lw.tang(), dPoint(1,0), 1e-6);
      assert_eq(lw.ang(), 0.0);
      assert_deq(lw.pt(), dPoint(1,1), 1e-6);
    }

    { // 2 same point
      LineWalker lw(dLine("[[1,1],[1,1]]"));
      assert_eq(lw.is_begin(), 1);
      assert_eq(lw.is_end(), 0);
      assert_eq(lw.length(), 0.0);
      assert_eq(lw.dist(), 0.0);
      assert_deq(lw.norm(), dPoint(0,1), 1e-6);
      assert_deq(lw.tang(), dPoint(1,0), 1e-6);
      assert_eq(lw.ang(), 0.0);
      assert_deq(lw.pt(), dPoint(1,1), 1e-6);
    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
