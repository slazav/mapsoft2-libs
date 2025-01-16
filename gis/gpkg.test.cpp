///\cond HIDDEN (do not show this in Doxyden)

#include "gpkg.h"
#include "err/assert_err.h"

int
main(){
  try{

/*
    GPKG db("naturvard_ln25.gpkg");
    for (const auto & tt: db.get_tables()){
      std::cout << ">>> " << tt.first << ": "
                << tt.second.type << ", "
                << tt.second.gcol << ", "
                << tt.second.gtype << ", "
                << tt.second.srs << "\n";
      if (tt.second.type!="features") continue;
      db.read_start(tt.first);
      while (1){
        auto o = db.read_next();
        if (o.type==-1) break;
        std::cout << o << "\n\n";
      }
    }
*/
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
