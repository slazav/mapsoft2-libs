///\cond HIDDEN (do not show this in Doxyden)

#include "ocad_types.h"
#include "err/assert_err.h"

int
main(){
  try{
    assert_eq(sizeof(ocad_long),  4);
    assert_eq(sizeof(ocad_int),   4);
    assert_eq(sizeof(ocad_small), 2);
    assert_eq(sizeof(ocad_word),  2);
    assert_eq(sizeof(ocad_bool),  2);
    assert_eq(sizeof(ocad_byte),  1);
    assert_eq(sizeof(ocad_double), 8);

    assert_eq(sizeof(ocad_coord),     8);
    assert_eq(sizeof(ocad_cmyk_),     4);
    assert_eq(sizeof(ocad_freq_ang_), 4);
    assert_eq(sizeof(ocad_color_),   72);
    assert_eq(sizeof(ocad_colsep_),  24);

    assert_eq(sizeof(ocad_header_),        48);
    assert_eq(sizeof(ocad8_colinfo_),   19224);
    assert_eq(sizeof(ocad_index_block_<int>), 4+256*sizeof(int));
    assert_eq(sizeof(ocad_parstr_index_),  16);
    assert_eq(sizeof(ocad8_object_index_), 24);
    assert_eq(sizeof(ocad8_object_),       32);
    assert_eq(sizeof(ocad9_object_index_), 40);
    assert_eq(sizeof(ocad9_object_),       40);
    assert_eq(sizeof(ocad8_base_symb_),   348);
    assert_eq(sizeof(ocad9_base_symb_),   572);

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
