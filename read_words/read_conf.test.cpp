///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "read_conf.h"
#include "err/assert_err.h"

int
main(int argc, const char* argv[]){
  try {

    assert_eq(read_conf("test_data/missing", {}), Opt());

    assert_err(read_conf("test_data/missing", {}, true),
              "test_data/missing: can't read file");

    Opt o;
    o.put("key1","val1");
    o.put("key2","val 2");
    o.put("key3","val3a\n      val3b");
    assert_eq(read_conf("test_data/conf1", {"key1","key2","key3"}), o);

    assert_err(read_conf("test_data/conf1", {"key1","key3"}, true),
               "test_data/conf1:5: unknown parameter: key2");

    assert_err(read_conf("test_data/conf2", {"key1","key2"}, true),
               "test_data/conf2:4: two words expected");
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
