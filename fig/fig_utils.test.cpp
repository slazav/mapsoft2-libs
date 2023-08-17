///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "fig.h"
#include "fig_utils.h"


int
main(){
  try {

   std::string fig_text =
      "# c1\n"
      "6 1350 2115 2700 2160\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t1350 2115 2700 2160\n"
      "-6\n"
      "6 5265 3870 6660 5175\n"
      "6 5355 4635 6435 5175\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t5355 5175 6435 4635\n"
      "-6\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t5265 4320 6660 3870\n"
      "-6\n"
      "6 1485 3105 2925 3870\n"
      "6 1485 3105 2925 3870\n"
      "6 1485 3105 2925 3870\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t1485 3870 2925 3105\n"
      "-6\n"
      "-6\n"
      "-6\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "6 0 0 0 0\n"
      "-6\n"
      "6 0 0 0 0\n"
      "# c1\n"
      "6 0 0 0 0\n"
      "-6\n"
      "# c1\n"
      "-6\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "6 0 0 0 0\n"
      "# c1\n"
      "6 0 0 0 0\n"
      "-6\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "-6\n"
      "6 0 0 0 0\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "# c1\n"
      "6 0 0 0 0\n"
      "-6\n"
      "-6\n"
    ;

   std::string fig_text_out =
      "# c1\n"
      "6 1350 2115 2700 2160\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t1350 2115 2700 2160\n"
      "-6\n"
      "6 5265 3870 6660 5175\n"
      "6 5355 4635 6435 5175\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t5355 5175 6435 4635\n"
      "-6\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t5265 4320 6660 3870\n"
      "-6\n"
      "6 1485 3105 2925 3870\n"
      "6 1485 3105 2925 3870\n"
      "6 1485 3105 2925 3870\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t1485 3870 2925 3105\n"
      "-6\n"
      "-6\n"
      "-6\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "6 0 0 0 0\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "-6\n"
      "6 0 0 0 0\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "-6\n"
    ;

   std::string fig_text_out2 =
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t1350 2115 2700 2160\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t5355 5175 6435 4635\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t5265 4320 6660 3870\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 2\n"
      "\t1485 3870 2925 3105\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
      "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 3\n"
      "\t4455 3870 4950 2880 5220 2295\n"
    ;

    Fig fig;
    Opt opts;
    opts.put("fig_header", false);
    std::istringstream ss(fig_text);
    read_fig(ss, fig, opts);

    fig_remove_empty_comp(fig);
    {
      std::ostringstream so;
      write_fig(so, fig, opts);
      assert_eq(so.str(), fig_text_out);
    }

    fig_remove_comp(fig);
    {
      std::ostringstream so;
      write_fig(so, fig, opts);
      assert_eq(so.str(), fig_text_out2);
    }

    // fig_match_template
    {
      auto def_crc   = "1 3 0 1 0 7 50 -1 -1 0.000 1 0.0000";

      auto def_oline = "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0";
      auto def_box   = "2 2 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0";
      auto def_cline = "2 3 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0";
      auto def_rbox  = "2 4 0 1 0 7 50 -1 -1 0.000 0 0 7 0 0";
      auto def_img   = "2 5 0 1 0 -1 50 -1 -1 0.000 0 0 -1 0 0";

      auto def_ospl1 = "3 0 0 1 0 7 50 -1 -1 0.000 0 0 0";
      auto def_cspl1 = "3 1 0 1 0 7 50 -1 -1 0.000 0 0 0";
      auto def_ospl2 = "3 2 0 1 0 7 50 -1 -1 0.000 0 0 0";
      auto def_cspl2 = "3 3 0 1 0 7 50 -1 -1 0.000 0 0 0";

      auto def_txt   = "4 0 0 50 -1 0 12 0.0000 4";
      auto def_arc   = "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0";

      #define CMP(X,Y,R) assert_eq(fig_match_templates(X, Y), R);\
                         assert_eq(fig_match_templates(Y, X), R);

      // compaund/compaund_end object equal to same type
      CMP("6", "6", true);
      CMP("-6", "-6", true);

      // compound, compaund_end, text, image are not equal to any other type
      CMP( "6", "-6", false);
      CMP( "6", def_oline, false);
      CMP( "6", def_box,   false);
      CMP( "6", def_cline, false);
      CMP( "6", def_rbox,  false);
      CMP( "6", def_img,   false);
      CMP( "6", def_crc,   false);
      CMP( "6", def_arc,   false);
      CMP( "6", def_txt,   false);
      CMP( "6", def_ospl1, false);
      CMP( "6", def_cspl1, false);
      CMP( "6", def_ospl2, false);
      CMP( "6", def_cspl2, false);

      CMP( "-6", def_oline, false);
      CMP( "-6", def_box,   false);
      CMP( "-6", def_cline, false);
      CMP( "-6", def_rbox,  false);
      CMP( "-6", def_img,   false);
      CMP( "-6", def_crc,   false);
      CMP( "-6", def_arc,   false);
      CMP( "-6", def_txt,   false);
      CMP( "-6", def_ospl1, false);
      CMP( "-6", def_cspl1, false);
      CMP( "-6", def_ospl2, false);
      CMP( "-6", def_cspl2, false);

      CMP( def_txt, def_oline, false);
      CMP( def_txt, def_box,   false);
      CMP( def_txt, def_cline, false);
      CMP( def_txt, def_rbox,  false);
      CMP( def_txt, def_img,   false);
      CMP( def_txt, def_crc,   false);
      CMP( def_txt, def_arc,   false);
      CMP( def_txt, def_ospl1, false);
      CMP( def_txt, def_cspl1, false);
      CMP( def_txt, def_ospl2, false);
      CMP( def_txt, def_cspl2, false);

      CMP( def_img, def_oline, false);
      CMP( def_img, def_box,   false);
      CMP( def_img, def_cline, false);
      CMP( def_img, def_rbox,  false);
      CMP( def_img, def_crc,   false);
      CMP( def_img, def_arc,   false);
      CMP( def_img, def_ospl1, false);
      CMP( def_img, def_cspl1, false);
      CMP( def_img, def_ospl2, false);
      CMP( def_img, def_cspl2, false);

      // more tests are needed here
    }
  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
