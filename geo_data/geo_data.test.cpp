///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "geo_data.h"

int
main(){
  try{

    { // GeoWpt

      // constructor, clear/set alt
      GeoWpt p1;
      assert_eq(p1.x, 0);
      assert_eq(p1.y, 0);
      assert_eq(p1.t, 0);
      assert(std::isnan(p1.z));
      assert(std::isnan(p1.z));
      assert_eq(p1.have_alt(), false);
      p1.z = 0;
      assert(!std::isnan(p1.z));
      assert_eq(p1.have_alt(), true);
      p1.clear_alt();
      assert_eq(p1.have_alt(), false);

      p1 = dPoint(10,10,0); // z is undef!
      assert_eq(p1.x, 10);
      assert_eq(p1.y, 10);
      assert_eq(p1.t, 0);
      assert_eq(p1.have_alt(), false);
      p1.opts.put<double>("test", 10.0);
      assert_eq(p1.opts.get<double>("test", 0.0), 10);

      p1 = GeoWpt(11,11);
      assert_eq(p1.x, 11);
      assert_eq(p1.y, 11);
      assert_eq(p1.t, 0);
      assert_eq(p1.have_alt(), false);
    }

    { // GeoTpt

      // constructor, clear/set alt
      GeoTpt p1;
      assert_eq(p1.x, 0);
      assert_eq(p1.y, 0);
      assert_eq(p1.t, 0);
      assert_eq(p1.start, false);
      assert(std::isnan(p1.z));
      assert_eq(p1.have_alt(), false);
      p1.z = 0;
      assert_eq(p1.have_alt(), true);
      assert_eq(p1.z, 0);
      p1.clear_alt();
      assert_eq(p1.have_alt(), false);

      p1 = dPoint(10,10,0);
      assert_eq(p1.x, 10);
      assert_eq(p1.y, 10);
      assert_eq(p1.t, 0);
      assert_eq(p1.have_alt(), false);
    }

    { // GeoWptList
      GeoWptList l1;
      l1.push_back(GeoWpt(37.403169, 55.803693, 210));
      l1.push_back(GeoWpt(24.803224, 60.174925, 20));
      assert_eq(l1.size(), 2);
      assert_eq(iRect(l1.bbox()*10.0), iRect(248,558,125,43));
      assert_eq(l1[0].z, 210);
      assert_eq(l1[1].z, 20);
      l1.clear_alt();
      assert(std::isnan(l1[0].z));
      assert(std::isnan(l1[1].z));
    }

    { // GeoTrk
      GeoTrk l1;
      l1.push_back(GeoTpt(37.403169, 55.803693, 210));
      l1.push_back(GeoTpt(24.803224, 60.174925, 20,0));
      assert_eq(l1.size(), 2);
      assert_feq(l1.length(), 886625, 1);
      assert_eq(iRect(l1.bbox()*10.0), iRect(248,558,125,43));
      assert_eq(l1[0].z, 210);
      assert_eq(l1[1].z, 20);
      l1.clear_alt();
      assert(std::isnan(l1[0].z));
      assert(std::isnan(l1[1].z));

      GeoTrk l2;
      l2.emplace_back(1,2,3,0);
      l2.emplace_back(2,3,4,0);
      l2.emplace_back(3,4,5,1);
      l2.emplace_back(4,5,6,0);
      l2.emplace_back(5,6,7,0);
      l2.emplace_back(6,7,8,1);

      // convert to dLine/dMultiline
      assert_eq(dMultiLine(l2),
        dMultiLine("[ [[1,2,3],[2,3,4]], [[3,4,5],[4,5,6],[5,6,7]], [[6,7,8]]]"));
      assert_eq(dLine(l2),
        dLine("[[1,2,3],[2,3,4],[3,4,5],[4,5,6],[5,6,7],[6,7,8]]"));

      // constructed from Line/MultiLine
      l2 = GeoTrk(dMultiLine("[ [[1,2,3],[2,3,4]], [[3,4,5],[4,5,6],[5,6,7]], [[6,7,8]]]"));
      assert_eq(l2.size(), 6);
      assert_eq(l2[0].start, 1);
      assert_eq(l2[0].z, 3);
      assert_eq(l2[0].t, 0);
      assert_eq(l2[1].start, 0);
      assert_eq(dMultiLine(l2),
        dMultiLine("[ [[1,2,3],[2,3,4]], [[3,4,5],[4,5,6],[5,6,7]], [[6,7,8]]]"));

      l2 = GeoTrk(dLine("[[1,2,3],[2,3,4],[3,4,5],[4,5,6],[5,6,7],[6,7,8]]"));
      assert_eq(l2.size(), 6);
      assert_eq(l2[0].start, 1);
      assert_eq(l2[0].z, 3);
      assert_eq(l2[0].t, 0);
      assert_eq(l2[1].start, 0);
      assert_eq(dMultiLine(l2),
        dMultiLine("[[[1,2,3],[2,3,4],[3,4,5],[4,5,6],[5,6,7],[6,7,8]]]"));

    }

    { // GeoMap
      GeoMap m1,m2;
      assert(m1==m2);
      assert(m1<=m2);
      assert(m1>=m2);
      assert(m1.empty());

      assert_eq(m1.image_dpi, 300);
      assert_eq(m1.tile_size, 256);
      assert_eq(m1.tile_swapy, false);

      assert_eq(m1.ref.size(),0);
      m1.add_ref(1,2,30,40);
      m1.add_ref(dPoint("[2,3]"),dPoint("[41,51]"));
      m1.add_ref(dLine("[[3,4],[4,5]]"),dLine("[[52,62],[63,73]]"));
      assert(!m1.empty());

      std::ostringstream ss;
      for (auto & r:m1.ref) ss << r.first << " " << r.second;
      assert_eq(ss.str(),
        "[1,2] [30,40]"
        "[2,3] [41,51]"
        "[3,4] [52,62]"
        "[4,5] [63,73]");
      assert_eq((iRect)m1.bbox(), iRect(1,2,3,3));

      m1.border = dMultiLine("[[-1,-1],[10,0],[10,1]]");
      assert_eq((iRect)m1.bbox(), iRect(-1,-1,11,6));
      m1.image_size = dPoint(5,5);
      assert_eq((iRect)m1.bbox(), iRect(0,0,5,5));

    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
