///\cond HIDDEN (do not show this in Doxyden)

#include "gobj_trk.h"
#include "err/assert_err.h"

std::ostream & operator<< (std::ostream & s, const GObjTrk::idx_t & idx){
  s << "[" << idx.sn << "," << idx.pn << "]";
  return s;
}


int
main(){
  try{

    GeoTrk trk;
    trk.add_point(GeoTpt(0,0,0,0)); // x,y,z,t
    trk.add_point(GeoTpt(1,1,0,0));
    trk.add_point(GeoTpt(2,1,0,0));
    trk.add_segment();
    trk.add_point(GeoTpt(3,1,0,0));
    trk.add_point(GeoTpt(3,0,0,0));
    trk.add_point(GeoTpt(1,0,0,0));

    // find_* functions use line_width and dot_w parameters
    trk.opts.put("thickness", 0.5);

    GObjTrk trk_obj(trk);
    Opt o;
    o.put("dot_w", 0.0);
    o.put("trk_draw_width", 1.0);
    trk_obj.set_opt(o);

    auto v1 = trk_obj.find_segments(dPoint(1.1,1.1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 2);
    assert_eq(v1[0], GObjTrk::idx_t(0,2));
    assert_eq(v1[1], GObjTrk::idx_t(0,1));

    v1 = trk_obj.find_segments(dPoint(3,1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], GObjTrk::idx_t(1,1));

    trk.opts.put("thickness", 1.5);
    trk_obj.set_opt(o);

    v1 = trk_obj.find_segments(dPoint(0,0));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 3);
    assert_eq(v1[0], GObjTrk::idx_t(0,1));
    assert_eq(v1[1], GObjTrk::idx_t(1,2));
    assert_eq(v1[2], GObjTrk::idx_t(0,2));


    trk.opts.put("thickness", 0.5);
    trk_obj.set_opt(o);

    v1 = trk_obj.find_points(dPoint(1.1,1.1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], GObjTrk::idx_t(0,1));

    v1 = trk_obj.find_points(dPoint(3,1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 1);
    assert_eq(v1[0], GObjTrk::idx_t(1,0));

    trk.opts.put("thickness", 1.0);
    trk_obj.set_opt(o);

    v1 = trk_obj.find_points(dPoint(0,0));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 3);
    assert_eq(v1[0], GObjTrk::idx_t(0,0));
    assert_eq(v1[1], GObjTrk::idx_t(1,2));
    assert_eq(v1[2], GObjTrk::idx_t(0,1));

    v1 = trk_obj.find_points(dRect(1.5,0.5,2,1));
    // for (const auto & n: v1) std::cerr << "> " << n << "\n";
    assert_eq(v1.size(), 2);
    assert_eq(v1[0], GObjTrk::idx_t(0,2));
    assert_eq(v1[1], GObjTrk::idx_t(1,0));

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
