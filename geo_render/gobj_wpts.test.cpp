///\cond HIDDEN (do not show this in Doxyden)

#include "gobj_wpts.h"
#include "err/assert_err.h"

int
main(){
  try{

    GeoWptList wpts;
    wpts.emplace_back(0,0,0); // x,y,z
    wpts.emplace_back(1,1,0);
    wpts.emplace_back(2,1,0);
    wpts.emplace_back(3,1,0);
    wpts.emplace_back(3,0,0);
    wpts.emplace_back(1,0,0);

    GObjWpts wpts_obj(wpts);
    Opt o;
    o.put("wpt_draw_size", 0.5);
    o.put("wpt_text_size", 0);
    wpts_obj.set_opt(o);

    auto v1 = wpts_obj.find_points(dPoint(1.1,1.1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], 1);

    v1 = wpts_obj.find_points(dPoint(3,1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], 3);

    o.put("wpt_draw_size", 1.5);
    o.put("wpt_text_size", 0);
    wpts_obj.set_opt(o);

    v1 = wpts_obj.find_points(dPoint(0,0));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 3);
    assert_eq(v1[0], 0);
    assert_eq(v1[1], 1);
    assert_eq(v1[2], 5);

    v1 = wpts_obj.find_points(dRect(1.5,0.5,2,1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 2);
    assert_eq(v1[0], 2);
    assert_eq(v1[1], 3);

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond