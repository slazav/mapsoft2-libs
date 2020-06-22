///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include <iomanip>
#include "image_cache.h"
#include "err/assert_err.h"

int
main(){
  try{

    ImageRCache icache(2);

    ImageR i1 = icache.get("test_data/test_fullc1.jpg");

    ImageR i2 = icache.get("test_data/test_fullc2.jpg");

    ImageR i3 = icache.get("test_data/test_fullc1.jpg");

    assert_eq(i1.size(), iPoint(320,240));
    assert_eq(i2.size(), iPoint(500,500));
    assert_eq(i1.data(), i3.data());

    assert_err(icache.get("test_data/missing.jpg"),
      "Can't open file: test_data/missing.jpg");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
