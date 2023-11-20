#include <iostream>
#include "err/err.h"
#include "vxi.h"

int
main(int argc, char ** argv) {
  try {
    VXI dev("gpib", "gpib0,13");
    dev.clear();
    dev.write("*IDN?\n");
    std::cerr << "> " << dev.read() << "\n";
  }
  catch (Err e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
