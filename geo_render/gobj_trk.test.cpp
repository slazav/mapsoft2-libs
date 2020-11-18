///\cond HIDDEN (do not show this in Doxyden)

#include "gobj_trk.h"
#include "err/assert_err.h"

int
main(){
  try{

    GeoTrk trk;
    trk.emplace_back(0,0,0,1,0); // x,y,z,start,t
    trk.emplace_back(1,1,0,0,0);
    trk.emplace_back(2,1,0,0,0);
    trk.emplace_back(3,1,0,1,0);
    trk.emplace_back(3,0,0,0,0);
    trk.emplace_back(1,0,0,0,0);

    // find_* functions use line_width and dot_w parameters
    trk.opts.put("thickness", 0.5);

    GObjTrk trk_obj(trk);
    Opt o;
    o.put("dot_w", 0.0);
    trk_obj.set_opt(o);

    auto v1 = trk_obj.find_segments(dPoint(1.1,1.1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 2);
    assert_eq(v1[0], 1);
    assert_eq(v1[1], 0);

    v1 = trk_obj.find_segments(dPoint(3,1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], 3);

    trk.opts.put("thickness", 1.5);
    trk_obj.set_opt(o);

    v1 = trk_obj.find_segments(dPoint(0,0));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 3);
    assert_eq(v1[0], 0);
    assert_eq(v1[1], 4);
    assert_eq(v1[2], 1);


    trk.opts.put("thickness", 0.5);
    trk_obj.set_opt(o);

    v1 = trk_obj.find_points(dPoint(1.1,1.1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], 1);

    v1 = trk_obj.find_points(dPoint(3,1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], 3);

    trk.opts.put("thickness", 1.0);
    trk_obj.set_opt(o);

    v1 = trk_obj.find_points(dPoint(0,0));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 3);
    assert_eq(v1[0], 0);
    assert_eq(v1[1], 5);
    assert_eq(v1[2], 1);

    v1 = trk_obj.find_points(dRect(1.5,0.5,2,1));
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