///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "geo_utils.h"
#include "geom/poly_tools.h"

int
main(){
  try {

    {  //geo_dist_2d

      // large distance:
      // nakarte: 885370
      // formula: 886625
      double d1 = 886625;
      assert_feq(geo_dist_2d(
        dPoint(37.403169, 55.803693),
        dPoint(24.803224, 60.174925)), d1, 1.0 );

      // nakarte: 7960
      // map:     8000
      // formula: 7973.47
      double d2 = 7973.47;
      assert_feq(geo_dist_2d(
        dPoint(37.340126, 55.803018),
        dPoint(37.467499, 55.804658)), d2, 1.0);

      // nakarte: 63870
      // map:     64000
      // formula: 63961.6
      double d3 = 63961.6;
      assert_feq(geo_dist_2d(
        dPoint(37.589979, 55.932279),
        dPoint(37.568607, 56.506565)), d3, 1.0);

      // pole to pole
      double d4 = 6380e3*M_PI;
      assert_feq(geo_dist_2d(
        dPoint(12, -90),
        dPoint(22,  90)), d4, 1.0);

      // pole to equator
      double d5 = 6380e3*M_PI/2;
      assert_feq(geo_dist_2d(
        dPoint(12, -90),
        dPoint(22,   0)), d5, 1.0);

      // geo_bearing_2d
      assert_deq(geo_bearing_2d(
        dPoint(0,0), 0, 6380e3), dPoint(0,57.296), 1e-3);

      assert_deq(geo_bearing_2d(
        dPoint(0,0), 90, 6380e3), dPoint(57.296,0), 1e-3);

      assert_deq(geo_bearing_2d(
        dPoint(0,0), 180, 6380e3), dPoint(0,-57.296), 1e-3);

      assert_deq(geo_bearing_2d(
        dPoint(0,0), 270, 6380e3), dPoint(-57.296,0), 1e-3);

      assert_deq(geo_bearing_2d(
        dPoint(0,0), -90, 6380e3), dPoint(-57.296,0), 1e-3);

      assert_deq(geo_bearing_2d(
        dPoint(-2.809, 54.055), 45, 1950), dPoint(-2.788,54.067), 1e-3);


      dLine       l1("[[0,0],[2,1],[4,2]]");
      dMultiLine ml1("[[],[[0,0],[2,1],[4,2]]]");
      dMultiLine ml0("[[],[]]");

      assert_feq(geo_length_2d(l1),  497900, 1.0);
      assert_feq(geo_length_2d(ml0), 0, 1.0);
      assert_feq(geo_length_2d(ml1), 497900, 1.0);

      dPoint p0;
      assert_feq(nearest_vertex(l1, dPoint(1,2), &p0,
                 (double (*)(const dPoint&, const dPoint&))geo_dist_2d), 157448, 1);  // ~111km*sqrt(2)
      assert_eq(p0, dPoint(2,1));
      assert_feq(nearest_vertex(ml1, dPoint(1,2), &p0,
                 (double (*)(const dPoint&, const dPoint&))geo_dist_2d), 157448, 1);
      assert_eq(p0, dPoint(2,1));

      assert_feq(nearest_vertex(ml1, dPoint(1,2), &p0,
                 (double (*)(const dPoint&, const dPoint&))geo_dist_2d), 157448, 1);
      assert_err(nearest_vertex(ml0, dPoint(1,2), &p0,
                 (double (*)(const dPoint&, const dPoint&))geo_dist_2d), "Can't find nearest point: empty line");

      assert_feq(geo_nearest_vertex_2d(l1, dPoint(1,2), &p0), 157448, 1);  // ~111km*sqrt(2)
      assert_eq(p0, dPoint(2,1));
      assert_feq(geo_nearest_vertex_2d(ml1, dPoint(1,2), &p0), 157448, 1);
      assert_eq(p0, dPoint(2,1));

      assert_feq(geo_nearest_vertex_2d(ml1, dPoint(1,2), &p0), 157448, 1);
      assert_eq(std::isinf(geo_nearest_vertex_2d(ml0, dPoint(1,2), &p0)), true);


    }
  /****************************/

  assert_eq(figure_geo_line("[1,1,1,1]"), dMultiLine("[[[1,1],[2,1],[2,2],[1,2],[1,1]]]"));
  assert_eq(figure_geo_line("[1,1]"),     dMultiLine("[[1,1]]"));
  assert_eq(figure_geo_line("[[1,1],[2,2]]"), dMultiLine("[[1,1],[2,2]]"));
  assert_err(figure_geo_line("no file"), "can't read figure: no file");

  /****************************/
  // lon2lon0
  assert_eq(lon2lon0(0.1), 3);
  assert_eq(lon2lon0(-0.1), -3);
  assert_eq(lon2lon0(179), 177);
  assert_eq(lon2lon0(-179), -177);
  assert_eq(lon2lon0(37+360), 39);
  assert_eq(lon2lon0(37-360), 39);

  // boundary cases
  assert_eq(lon2lon0(6-1e-10), 3);
  assert_eq(lon2lon0(6), 9);
  assert_eq(lon2lon0(0-1e-10), -3);
  assert_eq(lon2lon0(0), 3);

  // lon2pref
  assert_eq(lon2pref(37), 7);
  assert_eq(lon2pref(34), 6);
  assert_eq(lon2pref(-73.5), 48);
  assert_eq(lon2pref(34+720), 6);
  assert_eq(lon2pref(34-720), 6);

  // boundary cases
  assert_eq(lon2pref(6-1e-10), 1);
  assert_eq(lon2pref(6), 2);
  assert_eq(lon2pref(0-1e-10), 60);
  assert_eq(lon2pref(0), 1);

  // crdx2lon0
  assert_eq(crdx2lon0(7800000), 39);
  assert_eq(crdx2lon0(48800000), -75);
  assert_eq(crdx2lon0(1800000), 3);

  assert_err(crdx2lon0(800000), "zero coordinate prefix");

  // crdx2nonpref
  assert_eq(crdx2nonpref(7800000), 800000);
  assert_eq(crdx2nonpref(800000), 800000);
  assert_eq(crdx2nonpref(60800000), 800000);

  // no prefix
  assert_eq(crdx2nonpref(800000), 800000);

  assert_eq(GEO_PROJ_SU(31), "SU33");
  assert_eq(GEO_PROJ_SU(30), "SU33");
  assert_eq(GEO_PROJ_SU(5), "SU3");
  assert_eq(GEO_PROJ_SU(-10), "SU-9");

  // OS references
  assert_err(os_to_pt(""), "bad OS reference length: ");
  assert_err(os_to_pt("NN123"), "bad OS reference length: NN123");
  assert_err(os_to_pt("AA1212"), "unknown OS letter code: AA1212");
  assert_eq((iPoint)os_to_pt("NN166712"), iPoint(216600,771200));

  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
