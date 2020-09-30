#include "read_conf.h"
#include "read_words.h"
#include <fstream>

Opt
read_conf(const std::string & fname,
          std::list<std::string> known,
          bool should_exist){

  std::ifstream ss(fname);
  if (!ss){
    if (should_exist) throw Err() << fname
      << ": can't read file";
    else return Opt();
  }

  int N[2] = {0,0};
  Opt ret;
  while (1){
    std::vector<std::string> vs = read_words(ss, N, false);
    if (vs.size()<1) break;

    // allow only two-word lines
    if (vs.size()!=2) throw Err() << fname << ":" << N[0]
      << ": two words expected";

    bool found = false;
    for (auto const & k: known){
      if (vs[0] == k) {
        ret.put(vs[0], vs[1]);
        found = true;
        break;
      }
    }

    if (!found)
      throw Err() << fname << ":" << N[0]
        << ": unknown parameter: " << vs[0];
  }
  return ret;
}
