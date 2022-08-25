///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "read_words.h"
#include "err/assert_err.h"

int
main(int argc, const char* argv[]){
  try {

    std::string s("\"a b\" \\\'c");
    assert_eq(unquote_words(s), "a b \'c");
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
