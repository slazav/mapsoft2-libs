///\cond HIDDEN (do not show this in Doxyden)

// More complicated script: preprocessing iostream (if, define, include) commands

#include <iostream>
#include "read_words.h"
#include "err/assert_err.h"




int
main(){

  std::string fname="stdin";
//  std::ifstream ff(fname);
//  if (!ff) throw Err() << "can't open file: " << fname;
//  auto path = file_get_prefix(fname);

  int line_num[2] = {0,0}; // line counter for read_words
  std::deque<bool> ifs;    // for if/endif commands
  read_words_defs defs;

  while (1){
    auto vs = read_words(std::cin, line_num, false);
    if (vs.size()==0) break;

    try{
      if (read_words_stdcmds(vs, defs, ifs)) continue;
      for (const auto & v:vs) std::cout << " " << v;
      std::cout << "\n";

/*
      // include command
      if (vs[0] == "include"){
        if (vs.size()!=2) throw Err() << "include: filename expected";
        auto fn = vs[1];
        // should not happend, but lets check before accessing fn[0]:
        if (fn.size() == 0) throw Err() << "include: empty filename";

        if (fn[0] != '/') fn = path + fn;
        load(fn, defs);
        type=-1;
        continue;
      }
*/

    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }
  }

}

///\endcond
