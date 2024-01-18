#include <iostream>
#include "ocad.h"
#include "err/err.h"

using namespace std;

int
main(int argc, char **argv){

  try {

    if (argc!=3)
      throw Err() << "usage: " << argv[0] << " <file> <file>";

    ocad O;
    O.read(argv[1], 2);
    O.update_extents();
    O.write(argv[2]);

  }
  catch (const Err & e){
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
