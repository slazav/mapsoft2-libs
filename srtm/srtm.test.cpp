///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "srtm.h"
#include "err/assert_err.h"

int
main(){
  try{


    // tiles
    {
      // empty tile (ALOS by default)
      SRTMTile T("./test_srtm", iPoint(30.2, 78.0));
      assert_eq(T.is_empty(), true);
      assert_eq(T.srtm, false);
      assert_eq(T.w, 0);
      assert_eq(T.h, 0);
      assert_eq(T.key, iPoint(30,78));
      assert_eq(T.step, dPoint());
    }
    {
      // non-empty SRTM tile
      SRTMTile T("./test_srtm", iPoint(29.1, 78.2));
      assert_eq(T.is_empty(), false);
      assert_eq(T.srtm, true);
      assert_eq(T.w, 1201);
      assert_eq(T.h, 1201);
      assert_eq(T.key, iPoint(29,78));
      assert_deq(T.step, dPoint(1.0,1.0)/1200.0, 1e-8);
    }

    // coordinate conversions
    {
      iPoint key(10,10);
      SRTMTile T("./test_srtm", key);
      assert_eq(T.is_empty(), true);
      assert_eq(T.key, key);

      // check SRTM
      T.srtm = 1;
      T.w = T.h = 1201;
      T.step = dPoint(1.0,1.0)/1200.0;

      // check all 4 corners
      assert_deq(T.px2ll(dPoint(0,0)),       dPoint(0,1) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(1200,0)),    dPoint(1,1) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(0,1200)),    dPoint(0,0) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(1200,1200)), dPoint(1,0) + key, 1e-8);

      // check step
      assert_deq(T.px2ll(dPoint(1,1)),       dPoint(T.step.x, 1-T.step.y) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(1199,1199)), dPoint(1-T.step.x, T.step.y) + key, 1e-8);

      // switch to ALOS
      T.srtm = 0;
      T.w = T.h = 1200;
      T.step = dPoint(1.0,1.0)/1200.0;

      // check all 4 corners
      double dx = 0.5*T.step.x, dy = 0.5*T.step.y;
      assert_deq(T.px2ll(dPoint(0,0)),       dPoint(  dx, 1-dy) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(1199,0)),    dPoint(1-dx, 1-dy) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(0,1199)),    dPoint(  dx,   dy) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(1199,1199)), dPoint(1-dx,   dy) + key, 1e-8);

      // check step
      assert_deq(T.px2ll(dPoint(1,1)),       dPoint(  3*dx, 1-3*dy) + key, 1e-8);
      assert_deq(T.px2ll(dPoint(1198,1198)), dPoint(1-3*dx,   3*dy) + key, 1e-8);

    }

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

    Opt o1 = S.get_def_opt();
    assert_eq(o1.exists("srtm_dir"), true);
    assert_eq(o1.get("srtm_use_overlay"), "1");
    assert_eq(o1.get("srtm_interp"), "linear");


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond