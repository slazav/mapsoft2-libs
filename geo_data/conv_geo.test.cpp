///\cond HIDDEN (do not show this in Doxyden)

#include <proj.h>
#include <cassert>
#include "err/assert_err.h"
#include "conv_geo.h"
#include "geo_utils.h"
int
main(){
  try{

    assert_eq(expand_proj_aliases("WGS"), "+datum=WGS84 +proj=lonlat +type=crs");
    assert_eq(expand_proj_aliases("WEB"), "+proj=webmerc +datum=WGS84 +type=crs");
    assert_eq(expand_proj_aliases("FI"),  "EPSG:2393");
    assert_eq(expand_proj_aliases("CH"),  "EPSG:21781");

    assert_eq(expand_proj_aliases("SU-3"),
      "+ellps=krass +towgs84=+28,-130,-95 "
      "+proj=tmerc +lon_0=-3 +x_0=60500000 +type=crs");

    assert_eq(expand_proj_aliases("SU3"),
      "+ellps=krass +towgs84=+28,-130,-95 "
      "+proj=tmerc +lon_0=3 +x_0=1500000 +type=crs");

    assert_err(expand_proj_aliases("SU0"),
      "Bad central meridian for SU0 system. Should have 3+n*6 form.");

    assert_eq(expand_proj_aliases("SU-3N"),
      "+ellps=krass +towgs84=+28,-130,-95 "
      "+proj=tmerc +lon_0=-3 +x_0=500000 +type=crs");

    assert_eq(expand_proj_aliases("SU3N"),
      "+ellps=krass +towgs84=+28,-130,-95 "
      "+proj=tmerc +lon_0=3 +x_0=500000 +type=crs");

    assert_err(expand_proj_aliases("SU0N"),
      "Bad central meridian for SU0N system. Should have 3+n*6 form.");


    std::string proj_wgs = "WGS";
    std::string proj_krass = "SU27";

    ConvGeo cnv1(proj_wgs,proj_wgs, false);   // wgs -> wgs, 3D
    ConvGeo cnv2(proj_krass);                 // krass -> wgs, 2D
    ConvGeo cnv3(proj_wgs, proj_krass, true); // wgs -> krass, 2D

    assert_eq(cnv1.get_2d(), false);
    assert_eq(cnv2.get_2d(), true);
    assert_eq(cnv3.get_2d(), true);

    cnv3.set_2d(false);
    assert_eq(cnv3.get_2d(), false);
    cnv3.set_2d();
    assert_eq(cnv3.get_2d(), true);

    dPoint p1(25.651054, 60.976941, 0);
    dPoint p1a(5427091, 6763808, -11);
    dPoint p1z(5427091, 6763808);
    dPoint p1n(5427091, 6763808, nan(""));

    // trivial
    dPoint p2(p1);
    cnv1.frw(p2);
    assert_eq(p1,p2);
    cnv1.bck(p2);
    assert_eq(p1,p2);


    // wgs -> pulkovo -> wgs
    cnv2.set_2d(false);
    cnv3.set_2d(false);
    cnv2.bck(p2);
    assert_eq(iPoint(p2), p1a);
    cnv2.frw(p2);
    assert_deq(p1, p2, 2e-7);

    cnv3.frw(p2);
    assert_eq(iPoint(p2), p1a);
    cnv3.bck(p2);
    assert_deq(p1, p2, 2e-7);

    // same, 2D
    cnv2.set_2d();
    cnv3.set_2d();
    p2 = p1;
    cnv2.bck(p2);
    assert_eq(iPoint(p2), p1z);
    cnv2.frw(p2);
    assert_deq(p1, p2, 2e-7);

    cnv3.frw(p2);
    assert_eq(iPoint(p2), p1z);
    cnv3.bck(p2);
    assert_deq(p1, p2, 2e-7);

    // nan altitude (2d conversion)
    cnv2.set_2d(false);
    cnv3.set_2d(false);
    p2 = p1;
    p2.z = nan("");
    cnv2.bck(p2);
    assert(dist2d(p2,p1n) < 1e-1);
    cnv2.frw(p2);
    assert(dist2d(p1,p2) < 2e-7);

    cnv3.frw(p2);
    assert(dist2d(p2,p1n) < 1e-1);
    cnv3.bck(p2);
    assert(dist2d(p1,p2) < 2e-7);

    // line
    dLine l1;
    iLine l1a;
    for (int i=0; i<3; i++) {l1.push_back(p1); l1a.push_back(p1a);}
    cnv3.frw(l1);
    assert_eq(iLine(l1), l1a);

    // rescale_dst (only x,y is affected!)
    cnv3.rescale_dst(2);
    p2=p1;
    cnv3.frw(p2);
    p2.z*=2;
    assert(dist2d(p2,p1a*2) < 1);
    p2.z/=2;
    cnv3.bck(p2);
    assert(dist2d(p1,p2) < 2e-7);

    // rescale_src
    cnv3.rescale_src(2);
    cnv3.rescale_dst(0.5);
    p2=p1/2; p2.z *= 2;
    cnv3.frw(p2);
    assert(dist2d(p2,p1a) < 1);
    cnv3.bck(p2);
    p2.z/=2;
    assert(dist2d(p1,p2*2) < 2e-7);

    // adding coordinate prefix does not change result
    {
      std::string proj1 = "SU99";
      std::string proj2 = "+ellps=krass +towgs84=+28,-130,-95 +proj=tmerc"
                          " +lon_0=99 +x_0=500000";
;

      ConvGeo cnv1(proj1);
      ConvGeo cnv2(proj2);

      dPoint p1(98.651054, 60.976941), p2(p1), p3(p1);
      cnv1.bck(p1);
      cnv2.bck(p2);
      p2.x += 17000000;
      assert_deq(p1, p2, 1e-2); // m
    }

    // no datum
    {
      std::string proj_ll = "+ellps=krass +proj=lonlat";
      std::string proj_tmerc = "+ellps=krass +proj=tmerc +x_0=500000 +lon_0=27";

      ConvGeo cnv1(proj_ll, proj_tmerc);
      dPoint p1(25.651054, 60.976941, nan("")), p2(p1);
      dPoint p1a(426961.39, 6763794.09, nan(""));
      cnv1.frw(p2);
      assert(dist2d(p1a,p2) <1e-2);
      cnv1.bck(p2);
      assert(dist2d(p1,p2) < 1e-7);
    }

    // no datum, no ellipsoid
    {
      std::string proj_ll = "+proj=lonlat";
      std::string proj_tmerc = "+proj=tmerc +x_0=500000 +lon_0=27";

      ConvGeo cnv1(proj_ll, proj_tmerc);
      dPoint p1(25.651054, 60.976941, nan("")), p2(p1);
      dPoint p1a(426962.60, 6763675.97, nan(""));
      cnv1.frw(p2);
      assert(dist2d(p1a,p2) <1e-2);
      cnv1.bck(p2);
      assert(dist2d(p1,p2) < 1e-7);
    }

    // lonlat = latlon !
    {
      ConvGeo cnv1("+proj=lonlat", "+proj=latlon");
      dPoint p1(25.651054, 60.976941);
      dPoint p2(p1);
      cnv1.frw(p2);
      assert(dist2d(p1,p2) < 1e-7);
      cnv1.bck(p2);
      assert(dist2d(p1,p2) < 1e-7);
    }

    // bad coordinates (without datum conversion)
    {
      std::string proj_ll = "+proj=lonlat";
      std::string proj_tmerc = "+proj=tmerc +x_0=500000 +lon_0=27";

      ConvGeo cnv1(proj_ll, proj_tmerc);
      dPoint p1(25.651054, 160.976941);
      assert_err(cnv1.frw(p1), "Can't convert coordinates: Invalid coordinate");

      // too large y
      p1 = dPoint(426963,16763676);
      cnv1.bck(p1);
      // assert_eq(p1, dPoint(27,90)); // strange PROJ feature (different in proj 5.0 and 6.2)

      p1 = dPoint(nan(""), 60.976941);
// proj 9.1.0 -> 9.2.1 returns [nan, nan] instead of "Point outside of projection domain"
#if PROJ_AT_LEAST_VERSION(9, 2, 0)
      cnv1.frw(p1);
      assert_eq(std::isnan(p1.x) && std::isnan(p1.y), true);
#else
      assert_err(cnv1.frw(p1), "Can't convert coordinates: Point outside of projection domain");
#endif
    }

    // bad coordinates (with datum conversion)
    {
      std::string proj_wgs = "WGS";
      std::string proj_krass = "SU27";

      ConvGeo cnv1(proj_wgs, proj_krass);
      dPoint p1(25.651054, 160.976941);
      assert_err(cnv1.frw(p1), "Can't convert coordinates: Invalid coordinate");

      p1 = dPoint(nan(""), 60.976941);
// proj 9.1.0 -> 9.2.1 returns [nan, nan] instead of "Point outside of projection domain"
#if PROJ_AT_LEAST_VERSION(9, 2, 0)
      cnv1.frw(p1);
      assert_eq(std::isnan(p1.x) && std::isnan(p1.y), true);
#else
      assert_err(cnv1.frw(p1), "Can't convert coordinates: Point outside of projection domain");
#endif
    }

    {
      ConvGeo cnv1("SU33");
      dPoint p1(32.780603, 56.221141);
      dPoint p2(6486513.76, 6233327.52);

      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);
    }

    // FI - KKJ
    {
      ConvGeo cnv1("+proj=tmerc +lon_0=27 +x_0=3500000 +ellps=intl +towgs84=-90.7,-106.1,-119.2,4.09,0.218,-1.05,1.37");
      dPoint p1(20.54873,69.05995);
      dPoint p2(3243163.86,7677778.02);
      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv2("FI");
      assert_deq(cnv2.bck_pts(p1), p2, 0.1);
      assert_deq(cnv2.frw_pts(p2), p1, 1e-6);
    }

    // FI - ETRS-TM35FIN
    {
      ConvGeo cnv1("+proj=utm +zone=35 +ellps=GRS80 +units=m +no_defs");
      dPoint p1(20.54873,69.05995);
      dPoint p2(243094.408,7674572.84);
      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv2("ETRS-TM35FIN");
      assert_deq(cnv2.bck_pts(p1), p2, 0.1);
      assert_deq(cnv2.frw_pts(p2), p1, 1e-6);
    }

    // NO33
    {
      ConvGeo cnv1("+proj=utm +zone=33 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs +type=crs");
      dPoint p1(20.54873,69.05995);
      dPoint p2(721054.955,7671053.62);
      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv2("NO33");
      assert_deq(cnv2.bck_pts(p1), p2, 0.1);
      assert_deq(cnv2.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv3("EPSG:25833");
      assert_deq(cnv3.bck_pts(p1), p2, 0.1);
      assert_deq(cnv3.frw_pts(p2), p1, 1e-6);
    }

    // SE
    {
      ConvGeo cnv1("+proj=utm +zone=33 +ellps=GRS80 +units=m +no_defs");
      dPoint p1(20.54873,69.05995);
      dPoint p2(721054.955,7671053.62);
      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv2("SE");
      assert_deq(cnv2.bck_pts(p1), p2, 0.1);
      assert_deq(cnv2.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv3("EPSG:3006");
      assert_deq(cnv3.bck_pts(p1), p2, 0.1);
      assert_deq(cnv3.frw_pts(p2), p1, 1e-6);
    }

    // UK
    {
      ConvGeo cnv1("+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717"
                   " +x_0=400000 +y_0=-100000 +datum=OSGB36 +units=m +no_defs");
      dPoint p1(-2.79692, 54.03539);
      dPoint p2(347903.209,460228.132);
      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv2("GB");
      assert_deq(cnv2.bck_pts(p1), p2, 0.1);
      assert_deq(cnv2.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv3("EPSG:27700");
      assert_deq(cnv3.bck_pts(p1), p2, 0.1);
      assert_deq(cnv3.frw_pts(p2), p1, 1e-6);
    }

    // CH
    {
      ConvGeo cnv1("+proj=somerc +lat_0=46.95240555555556"
      " +lon_0=7.439583333333333 +x_0=600000 +y_0=200000"
      " +ellps=bessel +towgs84=674.374,15.056,405.346,0,0,0,0"
      " +units=m +no_defs");

      dPoint p1(7.93260, 46.49816);
      dPoint p2(637920.134,149768.714);
      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv2("CH");
      assert_deq(cnv2.bck_pts(p1), p2, 0.1);
      assert_deq(cnv2.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv3("EPSG:21781");
      assert_deq(cnv3.bck_pts(p1), p2, 0.1);
      assert_deq(cnv3.frw_pts(p2), p1, 1e-6);
    }


    // SU conversion with automatic zones
/*    {
      ConvGeo cnv1("SU", "WGS");
      dPoint p1(32.780603, 56.221141);
      dPoint p2(6486513.76, 6233327.52);

      assert_deq(cnv1.bck_pts(p1), p2, 0.1);
      assert_deq(cnv1.frw_pts(p2), p1, 1e-6);

      ConvGeo cnv2("WGS", "SU");
      assert_deq(cnv2.bck_pts(p2), p1, 1e-6);
      assert_deq(cnv2.frw_pts(p1), p2, 0.1);

      ConvGeo cnv3("SU", "SU33");
      assert_deq(cnv3.bck_pts(p2), p2, 0.1);
      assert_deq(cnv3.frw_pts(p2), p2, 0.1);

      ConvGeo cnv4("SU", "SU33");
      assert_deq(cnv4.bck_pts(p2), p2, 0.1);
      assert_deq(cnv4.frw_pts(p2), p2, 0.1);
    }
*/

    // CnvMap
    {
       GeoMap m;
       // ref: image points -> wgs84
       m.add_ref(159.0,386.0,   35.998051,55.999946);
       m.add_ref(2370.0,386.0,  36.248054,55.999950);
       m.add_ref(2371.0,3007.0, 36.248063,55.833280);
       m.add_ref(151.0,3010.0,  35.998059,55.833276);
       m.border = dMultiLine("[[[159.1,386.8],[1264.1,386.4],[2369.9,385.3],"
                        "[2371.2,3007.6],[1260.7,3008.9],[150.9,3009.3],[159.1,386.8]]]");
       m.proj = "+datum=WGS84 +proj=tmerc +lon_0=39 +x_0=500000";
       ConvMap cnv1(m, "SU39");
       dPoint p1(1333, 867, 1100);
       dPoint p2(7321000, 6209000, 1100);
       cnv1.frw(p1);
       assert(dist(p1,p2) < 15); // 15m accuracy (~2px)

       ConvMap cnv2(m, "SU_LL");
       dPoint p3(159,386, 2500);
       dPoint p4(36.00,56.00, 2500);
       cnv2.bck(p4);
       assert(dist(p3,p4) < 1); // 1px accuracy

       assert_eq(iRect(m.bbox_ref_img()), iRect("[151,386,2220,2624]"));

       dRect r = m.bbox_ref_wgs();
       assert(dist(r.tlc(), dPoint(35.998051,55.833276)) < 1e-6);
       assert_feq(r.w, 1/4.0, 1e-4);
       assert_feq(r.h, 1/6.0, 1e-4);

       cnv2.rescale_src(2);
       p4 = dPoint(36.00,56.00, 2500/2);
       cnv2.bck(p4);
       assert(dist(p3/2,p4) < 1);

       cnv2.rescale_src(0.5);
       cnv2.rescale_dst(2);
       p4 = dPoint(36.00*2,56.00*2, 2500);
       cnv2.bck(p4);
       assert(dist(p3,p4) < 1);

    }
  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
