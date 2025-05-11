///\cond HIDDEN (do not show this in Doxyden)

#include "err/assert_err.h"
#include "vmap2edit.h"

using namespace std;

int
main(){
  try{

    VMap2obj o1("line:0x10"), o2("point:0x10"), o3("area:0x10"), o4("text:0x10"), o5("none");
    // all statements are true
    assert_eq(calc_cond({"type==line",  "type==line:16",  "type!=point:16", "type!=line:10"},  o1), true);
    assert_eq(calc_cond({"type==point", "type==point:16", "type!=area:16",  "type!=point:10"}, o2), true);
    assert_eq(calc_cond({"type==area",  "type==area:16",  "type!=text:16",  "type!=area:10"},  o3), true);
    assert_eq(calc_cond({"type==text",  "type==text:16",  "type!=none",     "type!=text:10"},  o4), true);
    assert_eq(calc_cond({"type==none",  "type!=text", "type!=point:16"},  o5), true);

    // one is false
    assert_eq(calc_cond({"type==line", "type==line:16", "type==line:10"}, o1), false);
    assert_eq(calc_cond({"type==point", "type==point:10", "type==point:16"}, o2), false);
    assert_eq(calc_cond({"type==point", "type==area", "type==area:16"}, o2), false);

    o1.set_ref_type("text:0xA");
    o2.set_ref_type("line:0xA");
    o3.set_ref_type("point:0xA");
    o4.set_ref_type("area:0xA");

    // same with ref_type
    assert_eq(calc_cond({"ref_type==text",  "ref_type==text:10",  "ref_type!=none",     "ref_type!=text:8"},  o1), true);
    assert_eq(calc_cond({"ref_type==line",  "ref_type==line:10",  "ref_type!=point:10", "ref_type!=line:8"},  o2), true);
    assert_eq(calc_cond({"ref_type==point", "ref_type==point:10", "ref_type!=area:10",  "ref_type!=point:8"}, o3), true);
    assert_eq(calc_cond({"ref_type==area",  "ref_type==area:10",  "ref_type!=text:10",  "ref_type!=area:8"},  o4), true);
    assert_eq(calc_cond({"ref_type==none",  "ref_type!=text", "ref_type!=point:10"},  o5), true);

    // one is false
    assert_eq(calc_cond({"ref_type==line", "ref_type==line:10", "ref_type==line:8"}, o2), false);
    assert_eq(calc_cond({"ref_type==point", "ref_type==point:8", "ref_type==point:10"}, o3), false);
    assert_eq(calc_cond({"ref_type==point", "ref_type==area", "ref_type==area:10"}, o4), false);


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
