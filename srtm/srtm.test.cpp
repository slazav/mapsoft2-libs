///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "srtm.h"
#include "err/assert_err.h"

int
main(){
  try{

    Opt o;
    o.put("srtm_dir", "./test_srtm");
    o.put("srtm_interp", "nearest");
    SRTM S(o);

    assert_eq(S.get_h(dPoint(30.2,78.0)), SRTM_VAL_NOFILE);
    assert_eq(S.get_h(dPoint(-1.0,-1.0)), SRTM_VAL_NOFILE);
    assert_eq(S.get_h(dPoint(29.0,77.0)), SRTM_VAL_NOFILE);

    assert_feq(S.get_h(dPoint(29.1, 78.1)), 0, 1e-3);
    assert_feq(S.get_h(dPoint(29.6, 78.9)), 58, 1e-3);

    o.put("srtm_interp", "linear");
    S.set_opt(o);

    assert_feq(S.get_h(dPoint(29.1, 78.1)), 0, 1e-3);
    assert_feq(S.get_h(dPoint(29.6, 78.9)), 58, 1e-3);

    o.put("srtm_interp", "cubic");
    S.set_opt(o);

    assert_feq(S.get_h(dPoint(29.1, 78.1)), 0, 1e-3);
    assert_feq(S.get_h(dPoint(29.6, 78.9)), 58, 1e-3);

    Opt o1 = S.get_def_opt();
    assert_eq(o1.exists("srtm_dir"), true);
    assert_eq(o1.get("srtm_interp_holes"), "1");
    assert_eq(o1.get("srtm_interp"), "linear");


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond