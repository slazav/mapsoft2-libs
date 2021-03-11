///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "read_words.h"
#include "err/err.h"

int
main(int argc, const char* argv[]){
  try {
    bool lc=false, raw=false;

    for (int i=1; i<argc; i++) {
      if (std::string(argv[i])=="-l") lc = true;
      if (std::string(argv[i])=="-r") raw = true;
    }

    int N[2] = {0,0};
    while (1){
      std::vector<std::string> vs = read_words(std::cin, N, lc, raw);
      if (vs.size()<1) break;

      if (raw) {
        if (vs.size()) std::cout << "> " << vs[0];
      }
      else {
        std::cout << N[0] << "-" << N[1] << ":";
        for (auto const & i:vs) std::cout << " [" << i << "]";
        std::cout << "\n";
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
