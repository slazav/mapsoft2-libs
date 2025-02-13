///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <sstream>
#include "geo_mkref.h"
#include "geo_io.h"
#include "geo_utils.h"
#include "conv_geo.h"
#include "geo_nom/geo_nom.h"
#include "err/assert_err.h"

int
main(){
  try {

    Opt o;
    GeoMap map;

    /**** nom ****/

    { // nomenclature map with margins
      Opt o = Opt("{\"mkref\":\"nom\", \"name\":\"n37-001\", \"dpi\":\"200\"}");

      GeoMap map = geo_mkref_opts(o);
      assert_eq(map.name, "n37-001");
      assert_eq(map.proj, "SU39");
      assert_eq(map.image_dpi, 200);
      assert_eq(map.image_size, iPoint(2583,3020));
      assert_deq(map.border, dMultiLine(
            "[[0.351908927,2921.09993],[1238.55916,2972.44913],"
            "[2476.86091,3019.3302],[2582.39403,98.0292248],"
            "[1354.65253,51.3640529],[127.007511,0.251437777]]"), 1e-3);
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,2922],[35.9980041,55.6665025],"
        "[127,1],[35.9980557,55.9998604],"
        "[2476,3020],[36.497906,55.6665348],"
        "[2582,99],[36.4979852,55.9998413]]"), 1e-6);
    }

    { // nomenclature finnish map with margins
      Opt o = Opt("{\"mkref\":\"nom_fi\", \"name\":\"v52\", \"dpi\":\"200\"}");

      GeoMap map = geo_mkref_opts(o);
      assert_eq(map.name, "v52");
      assert_eq(map.proj, "ETRS-TM35FIN");
      assert_eq(map.image_dpi, 200);
      assert_eq(map.image_size, iPoint(7559,3779));  // 96x48cm / 2.54 * dpi
      assert_eq((iMultiLine)map.border, iMultiLine(
        "[[[0,3779],[7559,3779],[7559,0],[0,0]]]"));
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0.0787401575,-0.440944882],[27,68.7456375],"
        "[0.0787401575,3779.08661],[27,68.3151375],"
        "[7559.13386,-0.440944882],[29.371684,68.7290289],"
        "[7559.13386,3779.08661],[29.3268932,68.2988899]]"), 1e-5);
    }

    { // nomenclature map with margins
      Opt o = Opt("{\"mkref\":\"nom\", \"name\":\"n37-001\", \"dpi\":\"100\","
                  "\"margins\": \"100\", \"top_margin\": \"200\" }");
      GeoMap map = geo_mkref_opts(o);
      assert_eq(map.name, "n37-001");
      assert_eq(map.proj, "SU39");
      assert_eq(map.image_dpi, 100);
      assert_eq(map.image_size, iPoint(1492,1810));
      assert_deq(map.border, dMultiLine(
          "[[[100.175954,1660.54996],[719.27958,1686.22457],"
          "[1338.43045,1709.6651],[1391.19701,249.014612],"
          "[777.326265,225.682026],[163.503755,200.125719]]]"), 1e-3);

      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[100,1661],[35.9980041,55.6665025],"
        "[163,201],[35.9978612,55.9997415],"
        "[1338,1710],[36.497906,55.6665348],"
        "[1391,250],[36.4979926,55.9997274]]"), 1e-6);
    }

    { // nomenclature map with margins -- write map for manual test
      Opt o = Opt("{\"mkref\":\"nom\", \"name\":\"m47-022\", \"dpi\":\"50\","
                  "\"margins\": \"0\", \"top_margin\": \"0\" }");
      o.put("top_margin",16);
      o.put("left_margin",18);
      o.put("bottom_margin",775-16-747);
      o.put("right_margin",733-18-702);
      GeoMap map = geo_mkref_opts(o);
      assert_eq(map.name, "m47-022");
      assert_eq(map.proj, "SU99");
      assert_eq(map.image_dpi, 50);
      assert_eq(map.image_size, iPoint(732,775));
      assert_deq(map.border, dMultiLine(
          "[[[33.0508163,762.471297],[376.006149,754.874646],"
          "[718.951483,746.108754],[699.002452,16.1578048],"
          "[358.551855,24.9002932],[18.0908163,32.4766734]]]"), 1e-3);
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[18,33],[100.49987,51.6668599],"
        "[33,763],[100.4999,51.3335156],"
        "[699,17],[100.999943,51.6667138],"
        "[718,747],[100.999251,51.3333618]]"), 1e-6);
      map.image = "m47-022.jpg";
      write_ozi_map("test_data/m47-022.map", map, Opt());
    }

    /**** tms/google tiles ****/

    { // single TMS tile
      GeoMap map = geo_mkref_opts(Opt("{\"mkref\": \"tms_tile\", \"tiles\": \"[1,1,10]\"}"));
      GeoMap map1 = geo_mkref_opts(Opt("{\"mkref\": \"tms_tile\", \"tiles\": \"[1,1]\", \"zindex\":\"10\"}"));
      assert(map1 == map);

      assert_eq(map.name, "[1,1,1,1]");
      assert_eq(map.proj, "WEB");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(256,256));
      assert_eq(type_to_str(map.border), "[[[0,256],[256,256],[256,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[-179.648438,-84.9901002],"
        "[0,256],[-179.648438,-85.0207077],"
        "[256,0],[-179.296875,-84.9901002],"
        "[256,256],[-179.296875,-85.0207077]]"), 1e-6);
    }

    { // 2x3 TMS tile range
      GeoMap map = geo_mkref_opts(Opt("{\"mkref\": \"tms_tile\", \"tiles\": \"[1,1,2,3]\", \"zindex\":\"3\"}"));
      assert_eq(map.name, "[1,1,2,3]");
      assert_eq(map.proj, "WEB");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(256*2,256*3));
      assert_eq(type_to_str(map.border), "[[[0,768],[512,768],[512,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[-135,0],"
        "[0,768],[-135,-79.1713346],"
        "[512,0],[-45,0],"
        "[512,768],[-45,-79.1713346]]"), 1e-6);
    }

    { // single TMS tile covering a given point
      GeoMap map = geo_mkref_opts(Opt("{\"mkref\": \"tms_tile\", \"coords_wgs\": \"[64.0,32.0]\", \"zindex\":\"3\"}"));
      assert_eq(map.name, "[5,4,1,1]");
      assert_eq(map.proj, "WEB");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(256,256));
      assert_eq(type_to_str(map.border), "[[[0,256],[256,256],[256,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[45,40.9798981],"
        "[0,256],[45,0],"
        "[256,0],[90,40.9798981],"
        "[256,256],[90,0]]"), 1e-6);
    }

    { // single google tile covering a given point
      GeoMap map = geo_mkref_opts(Opt("{\"mkref\": \"google_tile\", \"coords_wgs\": \"[64.0,32.0]\", \"zindex\":\"3\"}"));
      assert_eq(map.name, "[5,3,1,1]");
      assert_eq(map.proj, "WEB");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(256,256));
      assert_eq(type_to_str(map.border), "[[[0,256],[256,256],[256,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[45,40.9798981],"
        "[0,256],[45,0],"
        "[256,0],[90,40.9798981],"
        "[256,256],[90,0]]"), 1e-6);
    }

    { // tms tiles covering a triangular area.
      Opt o("{"
        "\"mkref\": \"tms_tile\", "
        "\"border_wgs\": \"[[64,32],[65,31],[63,29]]\", "
        "\"coords_wgs\": \"[[64,32],[65,31],[63,29]]\", "
        "\"zindex\":\"7\"}");
      GeoMap map = geo_mkref_opts(o);
      geo_mkref_brd(map, o);
      assert_eq(map.name, "[86,74,2,3]");
      assert_eq(map.proj, "WEB");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(256*2,256*3));
      assert_eq((iMultiLine)map.border, iMultiLine("[[[193,250],[284,357],[193,463],[102,567]]]"));
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[61.875,34.3071439],"
        "[0,768],[61.875,27.0591258],"
        "[512,0],[67.5,34.3071439],"
        "[512,768],[67.5,27.0591258]]"), 1e-6);
    }

    { // single google tile covering a given point -- write map for manual test
      GeoMap map = geo_mkref_opts(Opt("{"
        "\"mkref\": \"google_tile\", "
        "\"coords_wgs\": \"[26.77188,61.33552]\", "
        "\"zindex\":\"14\"}"));
      assert_eq(map.name, "[9410,4633,1,1]");
      assert_eq(map.proj, "WEB");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(256,256));
      assert_eq(type_to_str(map.border), "[[[0,256],[256,256],[256,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[26.7626953,61.3440784],"
        "[0,256],[26.7626953,61.3335397],"
        "[256,0],[26.784668,61.3440784],"
        "[256,256],[26.784668,61.3335397]]"), 1e-6);
      map.image = "9410_4633_14.png";
      write_ozi_map("test_data/9410_4633_14.map", map, Opt());

    }


    /**** proj ****/

    { // 2x2 km map, Gauss-Kruger projection, 1:100'000, 300dpi
      Opt o;
      o.put("mkref", "proj");
      o.put("proj", "SU39");
      o.put("coords", "[7376000,6208000,2000,2000]");
      o.put("scale", 1000);
      GeoMap map = geo_mkref_opts(o);

      assert_eq(map.name, "");
      assert_eq(map.proj, "SU39");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(237,237));
      assert_eq(type_to_str(map.border), "[[[0,237],[237,237],[237,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[37.0107346,55.9959095],"
        "[0,237],[37.0116594,55.9778984],"
        "[237,0],[37.0428731,55.9964239],"
        "[237,237],[37.043783,55.9784124]]"), 1e-6);
    }

    { // L-shaped map, Gauss-Kruger projection, 1:100'000, 300dpi
      dLine L("[[7376000,6208000],[7380000,6208000],[7380000,6212000],[7378000,6212000],[7378000,6210000],[7376000,6210000]]");
      Opt o;
      o.put("mkref", "proj");
      o.put("proj", "SU39");
      o.put("scale", 1000);
      o.put("coords", L);
      o.put("border", L);
      GeoMap map = geo_mkref_opts(o);
      assert_eq(map.name, "");
      assert_eq(map.proj, "SU39");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(473,473));
      assert_deq(map.border, dMultiLine(
        "[[[0,473],[473,473],[473,0],[236,0],[236,236],[0,236]]]"), 1e-3);
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[37.0098126,56.0138446],"
        "[0,473],[37.0116594,55.9778984],"
        "[473,0],[37.0739848,56.0148637],"
        "[473,473],[37.0757721,55.978916]]"), 1e-6);
    }


    { // rectangular map defined by wgs rectangle, Gauss-Kruger projection, 1:100'000, 300dpi
      dLine L("[[24.801507,60.173730],[24.799790,60.176675],[24.805498,60.177358],[24.806914,60.174498]]");
      Opt o;
      o.put("mkref", "proj");
      o.put("proj", "SU27");
      o.put("scale", 250);
      o.put("coords_wgs", L.bbox());
      GeoMap map = geo_mkref_opts(o);

      assert_eq(map.name, "");
      assert_eq(map.proj, "SU27");
      assert_eq(map.image_dpi, 300);
      assert_eq(type_to_str(map.border), "[[[0,198],[195,198],[195,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[24.7995093,60.1773656],"
        "[0,198],[24.7997608,60.1736068],"
        "[195,0],[24.8069407,60.1774888],"
        "[195,198],[24.8071913,60.1737299]]"), 1e-6);
//std::cerr << "IMG: " << map.image_size << "\n";
//std::cerr << "BRD: " << map.border << "\n";
//std::cerr << "REF: " << l << "\n";
    }

    { // rectangular map defined by wgs border, Gauss-Kruger projection, 1:100'000, 300dpi
      dLine L("[[24.801507,60.173730],[24.799790,60.176675],[24.805498,60.177358],[24.806914,60.174498]]");
      Opt o;
      o.put("mkref", "proj");
      o.put("proj", "SU27");
      o.put("scale", 250);
      o.put("coords_wgs", L);
      o.put("border_wgs", L);
      o.put("margins", 10);
      o.put("left_margin", 5);
      o.put("top_margin", 15);
      GeoMap map = geo_mkref_opts(o);
      geo_mkref_brd(map,o);
      assert_eq(map.name, "");
      assert_eq(map.proj, "SU27");
      assert_eq(map.image_dpi, 300);
      assert_eq(map.image_size, iPoint(199,214));
      assert_eq(iMultiLine(rint(map.border)), iMultiLine(
         "[[[45,203],[5,47],[156,16],[188,167]]]"));
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[24.7995347,60.1775561],"
        "[0,214],[24.7998066,60.1734935],"
        "[199,0],[24.8071186,60.1776818],"
        "[199,214],[24.8073895,60.1736192]]"), 1e-6);
    }

    { // 2x2 km map, Gauss-Kruger projection, 1:100'000, 300dpi -- write map for manual test
      Opt o;
      o.put("mkref", "proj");
      o.put("proj", "SU99");
      o.put("coords", "[17552000,5624000,12000,6000]");
      o.put("dpi", 200);
      o.put("scale", 1000);
      GeoMap map = geo_mkref_opts(o);
      assert_eq(map.name, "");
      assert_eq(map.proj, "SU99");
      assert_eq(map.image_dpi, 200);
      assert_eq(map.image_size, iPoint(946,474));
      assert_eq(type_to_str(map.border), "[[[0,474],[946,474],[946,0],[0,0]]]");
      dLine l;
      for (auto & r:map.ref) {l.push_back(r.first); l.push_back(r.second);}
      assert_deq(l, dLine(
        "[[0,0],[99.7374411,50.7987634],"
        "[0,474],[99.7365908,50.744654],"
        "[946,0],[99.9078347,50.7975618],"
        "[946,474],[99.9067879,50.7434547]]"), 1e-6);
      map.image = "12x6+17552+5624k.png";
      write_ozi_map("test_data/12x6+17552+5624k.map", map, Opt());

    }


    {  // geo_mkref_web
      GeoMap m = geo_mkref_web();
      ConvMap c(m);
      assert_eq(m.name, "default");
      assert_eq(m.proj, "WEB");
      assert_eq(m.ref.size(), 4);
      assert_eq(m.image_size, iPoint(256,256));

      dPoint p(180,0);
      c.bck(p);
      assert_feq(p.x, 256, 1e-10);
      assert_feq(p.y, 128, 1e-10);

    }

  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
