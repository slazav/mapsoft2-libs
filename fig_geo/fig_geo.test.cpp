///\cond HIDDEN (do not show this in Doxyden)

#include "fig_geo.h"
#include "err/assert_err.h"

using namespace std;

int
main(){
  try {


     std::string str(
          "#FIG 3.2\n"
          "Portrait\n"
          "Center\n"
          "Metric\n"
          "A4\n"
          "100.00\n"
          "Single\n"
          "-2\n"
          "# \\map_proj=+datum=WGS84 +proj=tmerc +lon0=69\n"
          "# \\mp_id=0\n"
          "# \\name=j42-043\n"
          "# \\rscale=100000\n"
          "# \\style=hr\n"
          "1200 2\n"
          "# REF 68.99916 38.99999\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	0 53\n"
          "# REF 68.999164 38.666649\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	0 16703\n"
          "# REF 69.499172 38.999993\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	19488 0\n"
          "# REF 69.499176 38.666652\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	19580 16650\n"
          "# BRD j42-043\n"
          "2 3 0 1 5 7 2 -1 -1 0.000 0 0 7 0 0 7\n"
          "	0 53 11267 35 19488 0 19580 16650 8260 16694 0 16703\n"
          "	0 53\n"
     );

     std::istringstream si(str);

     Fig F;
     read_fig(si, F);

     auto map = fig_get_ref(F);
     assert_eq(map.proj, "+datum=WGS84 +proj=tmerc +lon0=69");

     {
       fig_del_ref(F);

       std::ostringstream so;
       write_fig(so, F);
       assert_eq(so.str(),
          "#FIG 3.2\n"
          "Portrait\n"
          "Center\n"
          "Metric\n"
          "A4\n"
          "100.00\n"
          "Single\n"
          "-2\n"
          "# \\mp_id=0\n"
          "# \\rscale=100000\n"
          "# \\style=hr\n"
          "1200 2\n"
       );
     }

     {
       fig_add_ref(F, map);
       std::ostringstream so;
       write_fig(so, F);
       assert_eq(so.str(), str);
    }

    GeoData D;

    GeoTrk trk(dLine("[[69,39], [69.5,39.5]]"));
    trk.name = "trk1";
    D.trks.push_back(trk);

    GeoWptList wpts;
    GeoWpt wpt(dPoint("[69.2,39.2]"));
    wpt.name = "wpt1";
    wpts.push_back(wpt);
    D.wpts.push_back(wpts);

    fig_add_wpts(F, map, D);
    fig_add_trks(F, map, D);

     std::string str1(
          "#FIG 3.2\n"
          "Portrait\n"
          "Center\n"
          "Metric\n"
          "A4\n"
          "100.00\n"
          "Single\n"
          "-2\n"
          "# \\map_proj=+datum=WGS84 +proj=tmerc +lon0=69\n"
          "# \\mp_id=0\n"
          "# \\name=j42-043\n"
          "# \\rscale=100000\n"
          "# \\style=hr\n"
          "1200 2\n"
          "# REF 68.99916 38.99999\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	0 53\n"
          "# REF 68.999164 38.666649\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	0 16703\n"
          "# REF 69.499172 38.999993\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	19488 0\n"
          "# REF 69.499176 38.666652\n"
          "2 1 0 4 4 7 1 -1 -1 0.000 0 1 -1 0 0 1\n"
          "	19580 16650\n"
          "# BRD j42-043\n"
          "2 3 0 1 5 7 2 -1 -1 0.000 0 0 7 0 0 7\n"
          "	0 53 11267 35 19488 0 19580 16650 8260 16694 0 16703\n"
          "	0 53\n"
          "# WPT wpt1\n"
          "2 1 0 2 0 7 6 0 -1 1.000 1 1 -1 0 0 1\n"
          "	7783 -9883\n"
          "# WPL\n"
          "4 0 8 5 -1 18 6.000 0.0000 4 0 0 7813 -9853 wpt1\\001\n"
          "# TRK trk1\n"
          "2 1 0 1 1 7 7 0 -1 1.000 1 1 -1 0 0 2\n"
          "	58 67 19185 -24872\n"
     );
     {
       std::ostringstream so;
       write_fig(so, F);
       assert_eq(so.str(), str1);
     }

     {
       GeoData D1;
       fig_get_wpts(F, map, D1);
       fig_get_trks(F, map, D1);
       assert_eq(D1.trks.size(), 1);
       auto t = *D1.trks.begin();
       assert_eq(t.size(), 2);
       assert_eq(t.name, "trk1");
       assert_deq(t[0], dPoint(69,39), 1e-4);
       assert_deq(t[1], dPoint(69.5,39.5), 1e-4);
       assert_eq(D1.wpts.size(), 1);
       auto w = *D1.wpts.begin();
       assert_eq(w.size(), 1);
       assert_eq(w[0].name, "wpt1");
       assert_deq(w[0], dPoint(69.2,39.2), 1e-4);
     }

     {
       fig_del_wpts(F);
       fig_del_trks(F);
       std::ostringstream so;
       write_fig(so, F);
       assert_eq(so.str(), str);
     }

  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
