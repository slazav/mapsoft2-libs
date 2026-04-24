///\cond HIDDEN (do not show this in Doxyden)

#include "err/assert_err.h"
#include "actions.h"

using namespace std;

int
main(){
  try{

    VMap2 vmap;
    VMap2obj o1("line:0x10"), o2("point:0x10"), o3("area:0x10"), o4("text:0x10"), o5("none");
    vmap.add()


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
