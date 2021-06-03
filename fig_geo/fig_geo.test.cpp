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
          "# \\lon0=69\n"
          "# \\map_proj=tmerc\n"
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
          "2 3 0 1 5 7 41 -1 -1 0.000 0 0 7 0 0 7\n"
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

  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
