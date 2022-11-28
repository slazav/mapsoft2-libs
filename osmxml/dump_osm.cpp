///\cond HIDDEN (do not show this in Doxyden)

#include "osmxml.h"

int
main(int argc, char* argv[]){
  try {

    if (argc<2) {
      std::cerr << "Usage: dump_osm <in_file1> ... > <out_file>\n";
      return 1;
    }

    OSMXML osm;

    for (int i=1; i<argc; i++){
      std::cerr << "Reading " << argv[i] << "\n";
      read_osmxml(argv[i], osm);
    }

    std::cout << "nodes (id -> coords):\n";
    for (const auto & n:osm.nodes)
      std::cout << "  " << n.first << " " << n.second << "\n";

    std::cout << "\n";
    std::cout << "points (id -> tags):\n";
    for (const auto & n:osm.points){
      std::cout << "  " << n.id << " " << n << "\n";
    }

    std::cout << "\n";
    std::cout << "ways (ids -> tags):\n";
    for (const auto & w:osm.ways){
      for (const auto & i:w.nds)
        std::cout << "  " << i;
      std::cout << "\n  " << w << "\n";
    }

    std::cout << "\n";
    std::cout << "relations (id -> tags), members (ref,type,role):\n";
    for (const auto & r:osm.relations){
      std::cout << r.id << "  " << r << "\n";
      for (const auto & m:r.members)
        std::cout << "    " << m.ref << " " << m.type << " " << m.role << "\n";
    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
  }
  return 0;
}

///\endcond
