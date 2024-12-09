/* Test program! */

#include "err/assert_err.h"
#include "jnx.h"



int
main(){
  try{

    assert_eq(sizeof(jnx_header_t),     JNX_HEADER_SIZE);
    assert_eq(sizeof(jnx_level_info_t), JNX_LEVEL_INFO_SIZE);
    assert_eq(sizeof(jnx_tile_info_t),  JNX_TILE_INFO_SIZE);

    assert_eq(rint(deg2jnx(0)),     0);
    assert_eq(rint(deg2jnx(180)),   0x7fffffff);
    assert_eq(rint(deg2jnx(-180)), -0x7fffffff);

    assert_eq(jnx2deg(0),     0.0);
    assert_feq(jnx2deg(0x7fffffff),   180.0, 1e-6);
    assert_feq(jnx2deg(-0x7fffffff), -180.0, 1e-6);

    assert_eq(deg2jnx(jnx2deg(0x12345678)), 0x12345678);

  }
  catch(Err & e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;

};

