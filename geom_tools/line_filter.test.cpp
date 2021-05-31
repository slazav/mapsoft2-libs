///\cond HIDDEN (do not show this in Doxyden)

#include "line_utils.h"
#include "opt/opt.h"
#include "err/assert_err.h"

int
main(){
  try{

    iLine l1("[[0,0],[1,1],[2,2],[5,3],[8,3],[10,4]]"), l2;

    l2=l1; line_filter_v1(l2, -1, 5);
    assert_eq(l2, iLine("[[0,0],[2,2],[5,3],[8,3],[10,4]]"));

    l2=l1; line_filter_v1(l2, 0.01, -1);
    assert_eq(l2, iLine("[[0,0],[2,2],[5,3],[8,3],[10,4]]"));

    l2=l1; line_filter_v1(l2, -1, 4);
    assert_eq(l2, iLine("[[0,0],[2,2],[8,3],[10,4]]"));

    l2=l1; line_filter_v1(l2, 1000, 4);
    assert_eq(l2, iLine("[[0,0],[10,4]]"));

    l2=l1; line_filter_v1(l2, -1, 2);
    assert_eq(l2, iLine("[[0,0],[10,4]]"));

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
