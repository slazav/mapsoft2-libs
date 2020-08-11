#include <cassert>
#include <iostream>
#include "quadkey.h"
#include "err/assert_err.h"
int
main(){
  try{

    int x,y,z;
    quadkey_to_tile("", x,y,z);
    assert_eq(x, 0);
    assert_eq(y, 0);
    assert_eq(z, 0);

    quadkey_to_tile("31", x,y,z);
    assert_eq(x, 3);
    assert_eq(y, 2);
    assert_eq(z, 2);

    quadkey_to_tile("31021", x,y,z);
    assert_eq(x, 25);
    assert_eq(y, 18);
    assert_eq(z, 5);

    assert_eq(tile_to_quadkey(0,0,0), "");
    assert_eq(tile_to_quadkey(3,2,2), "31");
    assert_eq(tile_to_quadkey(25,18,5), "31021");

  }
  catch (Err & e){
    std::cerr << "Error:" << e.str() << "\n";
  }
  return 0;
}
