///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "srtm.h"
#include "err/assert_err.h"

int
main(){
  try{

    Opt o;
    o.put("srtm_dir", "./test_srtm");
    SRTM S(o);

    int x0 = 29*1200;
    int y0 = 78*1200;

    assert_eq(S.get_val(x0 + 1300, y0, false), SRTM_VAL_NOFILE);
    assert_eq(S.get_val(-10, -20, false), SRTM_VAL_NOFILE);
    assert_eq(S.get_val(x0, y0-1, false), SRTM_VAL_NOFILE);

    assert_eq(S.get_val(x0+10, y0+10, false), 0);
    assert_eq(S.get_val(x0+700, y0+1100, false), 20);

    // wgs coordinates
    assert_eq(S.get_val_int4(dPoint(x0+10, y0+10)/1200.0), 0);
    assert_eq(S.get_val_int4(dPoint(x0+700, y0+1100)/1200.0), 20);

    // set value
    assert_eq(S.set_val(x0, y0-1, 100), SRTM_VAL_NOFILE);
    assert_eq(S.set_val(x0, y0, 100), 100);
    assert_eq(S.get_val(x0, y0, false), 100);

    Opt o1 = S.get_def_opt();
    assert_eq(o1.size(), 8);
    assert_eq(o1.exists("srtm_dir"), true);
    assert_eq(o1.get("srtm_draw_mode"), "shades");


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond