///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "read_words.h"
#include "err/err.h"
#include "err/assert_err.h"

int
main(int argc, const char* argv[]){
  try {
    bool lc=false, raw=false;

    for (int i=1; i<argc; i++) {
      if (std::string(argv[i])=="-l") lc = true;
      if (std::string(argv[i])=="-r") raw = true;
    }

    int N[2] = {0,0}; // line counter for read_words
    read_words_defs defs;
    while (1){
      std::vector<std::string> vs = read_words(std::cin, N, lc, raw);
      if (vs.size()<1) break;

      // definitions
      defs.apply(vs);
      if (vs[0] == "define"){
        if (vs.size()!=3) throw Err() << "bad definition in line " << N[0]
          << ": expected: define <key> <value>";
        defs.define(vs[1], vs[2]);
      }

      if (raw) {
        if (vs.size()) std::cout << "> " << vs[0];
        continue;
      }

      std::cout << N[0] << "-" << N[1] << ":";
      for (auto const & i:vs) std::cout << " [" << i << "]";
      std::cout << "\n";

      // test join_words
      std::string str = join_words(vs);
      std::istringstream ss(str);

      std::vector<std::string> vs1 = read_words(ss, NULL, false, false);
      std::vector<std::string> vs2 = read_words(ss, NULL, false, false);
      assert_eq(vs2.size(), 0 );
      assert_eq(vs1.size(), vs.size());
      for (size_t i = 0; i<vs1.size(); i++){
        assert_eq(vs[i], vs1[i]);
      }

    }
    return 0;
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
