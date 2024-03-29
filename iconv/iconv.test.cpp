///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include "iconv.h"
#include "err/assert_err.h"

int
main(){
  try{

    // convert from utf8 to koi8-r:
    IConv C1("UTF8", "KOI8-R");
    assert_eq( C1("привет!"), "������!");

    // non-utf characters are not modified:
    assert_eq( C1("������!"), "������!");

    // trivial conversion:
    IConv C2;
    assert_eq( C2("привет!"), "привет!");

    // same encodings - no conversion:
    C2 = IConv("UTF8", "UTF8");
    assert_eq( C2("привет!"), "привет!");

    // same unknown encodings - no conversion:
    C2=IConv("AAA", "AAA");
    assert_eq( C2("привет!"), "привет!");

    // unknown charset:
    assert_err(IConv C3("UTF8", "AAA"),
      "can't do iconv conversion from UTF8 to AAA");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond