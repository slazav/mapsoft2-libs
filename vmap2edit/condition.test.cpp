///\cond HIDDEN (do not show this in Doxyden)

#include "err/assert_err.h"
#include "condition.h"

using namespace std;

int
main(){
  try{

    VMap2obj o1("line:0x10"), o2("point:0x10"), o3("area:0x10"), o4("text:0x10"), o5("none");

    // true/false
    assert_eq(VMap2cond::eval_single("true", o1), true);
    assert_eq(VMap2cond::eval_single("false", o1), false);

    assert_err(VMap2cond::eval_single("xxx", o1), "unknown condition: xxx");

    // type
    assert_eq(VMap2cond::eval_single("type==line",  o1), true);
    assert_eq(VMap2cond::eval_single("type==point", o1), false);
    assert_eq(VMap2cond::eval_single("type==area",  o1), false);
    assert_eq(VMap2cond::eval_single("type==text",  o1), false);
    assert_eq(VMap2cond::eval_single("type==none",  o1), false);
    assert_eq(VMap2cond::eval_single("type==line:0x10", o1), true);
    assert_eq(VMap2cond::eval_single("type==line:16", o1), true);
    assert_eq(VMap2cond::eval_single("type==line:17", o1), false);

    assert_eq(VMap2cond::eval_single("type==line",  o2), false);
    assert_eq(VMap2cond::eval_single("type==point", o2), true);
    assert_eq(VMap2cond::eval_single("type==area",  o2), false);
    assert_eq(VMap2cond::eval_single("type==text",  o2), false);
    assert_eq(VMap2cond::eval_single("type==none",  o2), false);
    assert_eq(VMap2cond::eval_single("type==point:0x10", o2), true);
    assert_eq(VMap2cond::eval_single("type==point:16", o2), true);
    assert_eq(VMap2cond::eval_single("type==point:17", o2), false);

    assert_eq(VMap2cond::eval_single("type==line",  o3), false);
    assert_eq(VMap2cond::eval_single("type==point", o3), false);
    assert_eq(VMap2cond::eval_single("type==area",  o3), true);
    assert_eq(VMap2cond::eval_single("type==text",  o3), false);
    assert_eq(VMap2cond::eval_single("type==none",  o3), false);
    assert_eq(VMap2cond::eval_single("type==area:0x10", o3), true);
    assert_eq(VMap2cond::eval_single("type==area:16", o3), true);
    assert_eq(VMap2cond::eval_single("type==area:17", o3), false);

    assert_eq(VMap2cond::eval_single("type==line",  o4), false);
    assert_eq(VMap2cond::eval_single("type==point", o4), false);
    assert_eq(VMap2cond::eval_single("type==area",  o4), false);
    assert_eq(VMap2cond::eval_single("type==text",  o4), true);
    assert_eq(VMap2cond::eval_single("type==none",  o4), false);
    assert_eq(VMap2cond::eval_single("type==text:0x10", o4), true);
    assert_eq(VMap2cond::eval_single("type==text:16", o4), true);
    assert_eq(VMap2cond::eval_single("type==text:17", o4), false);

    assert_eq(VMap2cond::eval_single("type==line",  o5), false);
    assert_eq(VMap2cond::eval_single("type==point", o5), false);
    assert_eq(VMap2cond::eval_single("type==area",  o5), false);
    assert_eq(VMap2cond::eval_single("type==text",  o5), false);
    assert_eq(VMap2cond::eval_single("type==none",  o5), true);
    assert_eq(VMap2cond::eval_single("type==text:0x10", o5), false);
    assert_err(VMap2cond::eval_single("type>text:0x10", o5), "unknown condition: type>text:0x10");
    assert_err(VMap2cond::eval_single("type<text:0x10", o5), "unknown condition: type<text:0x10");

    // ref_type
    o1.set_ref_type("line:0x12");
    o2.set_ref_type("point:0x12");
    o3.set_ref_type("area:0x12");
    o4.set_ref_type("text:0x12");

    assert_eq(VMap2cond::eval_single("ref_type==line",  o1), true);
    assert_eq(VMap2cond::eval_single("ref_type==point", o1), false);
    assert_eq(VMap2cond::eval_single("ref_type==area",  o1), false);
    assert_eq(VMap2cond::eval_single("ref_type==text",  o1), false);
    assert_eq(VMap2cond::eval_single("ref_type==none",  o1), false);
    assert_eq(VMap2cond::eval_single("ref_type==line:0x12", o1), true);
    assert_eq(VMap2cond::eval_single("ref_type==line:18", o1), true);
    assert_eq(VMap2cond::eval_single("ref_type==line:16", o1), false);

    assert_eq(VMap2cond::eval_single("ref_type==line",  o2), false);
    assert_eq(VMap2cond::eval_single("ref_type==point", o2), true);
    assert_eq(VMap2cond::eval_single("ref_type==area",  o2), false);
    assert_eq(VMap2cond::eval_single("ref_type==text",  o2), false);
    assert_eq(VMap2cond::eval_single("ref_type==none",  o2), false);
    assert_eq(VMap2cond::eval_single("ref_type==point:0x12", o2), true);
    assert_eq(VMap2cond::eval_single("ref_type==point:18", o2), true);
    assert_eq(VMap2cond::eval_single("ref_type==point:16", o2), false);

    assert_eq(VMap2cond::eval_single("ref_type==line",  o3), false);
    assert_eq(VMap2cond::eval_single("ref_type==point", o3), false);
    assert_eq(VMap2cond::eval_single("ref_type==area",  o3), true);
    assert_eq(VMap2cond::eval_single("ref_type==text",  o3), false);
    assert_eq(VMap2cond::eval_single("ref_type==none",  o3), false);
    assert_eq(VMap2cond::eval_single("ref_type==area:0x12", o3), true);
    assert_eq(VMap2cond::eval_single("ref_type==area:18", o3), true);
    assert_eq(VMap2cond::eval_single("ref_type==area:16", o3), false);

    assert_eq(VMap2cond::eval_single("ref_type==line",  o4), false);
    assert_eq(VMap2cond::eval_single("ref_type==point", o4), false);
    assert_eq(VMap2cond::eval_single("ref_type==area",  o4), false);
    assert_eq(VMap2cond::eval_single("ref_type==text",  o4), true);
    assert_eq(VMap2cond::eval_single("ref_type==none",  o4), false);
    assert_eq(VMap2cond::eval_single("ref_type==text:0x12", o4), true);
    assert_eq(VMap2cond::eval_single("ref_type==text:18", o4), true);
    assert_eq(VMap2cond::eval_single("ref_type==text:16", o4), false);

    assert_eq(VMap2cond::eval_single("ref_type==line",  o5), false);
    assert_eq(VMap2cond::eval_single("ref_type==point", o5), false);
    assert_eq(VMap2cond::eval_single("ref_type==area",  o5), false);
    assert_eq(VMap2cond::eval_single("ref_type==text",  o5), false);
    assert_eq(VMap2cond::eval_single("ref_type==none",  o5), true);
    assert_eq(VMap2cond::eval_single("ref_type==text:0x16", o5), false);

    assert_err(VMap2cond::eval_single("ref_type>text:0x10", o5), "unknown condition: ref_type>text:0x10");
    assert_err(VMap2cond::eval_single("ref_type<text:0x10", o5), "unknown condition: ref_type<text:0x10");

    // name
    o1.name = "abc";
    assert_eq(VMap2cond::eval_single("name==abc", o1), true);
    assert_eq(VMap2cond::eval_single("name!=abc", o1), false);
    assert_eq(VMap2cond::eval_single("name==", o2), true);
    assert_eq(VMap2cond::eval_single("name!=", o2), false);

    assert_eq(VMap2cond::eval_single("name=~b", o1), true);
    assert_eq(VMap2cond::eval_single("name!~b", o1), false);
    assert_eq(VMap2cond::eval_single("name=~d", o1), false);
    assert_eq(VMap2cond::eval_single("name!~d", o1), true);
    assert_eq(VMap2cond::eval_single("name=~[abc]bc", o1), true);
    assert_eq(VMap2cond::eval_single("name=~[abc]bc", o1), true);
    assert_eq(VMap2cond::eval_single("name=~..c", o1), true);

    // npts
    o1.set_coords("[[1,1],[2,2],[3,3],[4,4],[5,5]]");
    assert_eq(VMap2cond::eval_single("npts==5", o1), true);
    assert_eq(VMap2cond::eval_single("npts==1", o1), false);
    assert_eq(VMap2cond::eval_single("npts!=5", o1), false);
    assert_eq(VMap2cond::eval_single("npts!=1", o1), true);
    assert_eq(VMap2cond::eval_single("npts>4",  o1), true);
    assert_eq(VMap2cond::eval_single("npts>5",  o1), false);
    assert_eq(VMap2cond::eval_single("npts>6",  o1), false);
    assert_eq(VMap2cond::eval_single("npts>=4", o1), true);
    assert_eq(VMap2cond::eval_single("npts>=5", o1), true);
    assert_eq(VMap2cond::eval_single("npts>=6", o1), false);
    assert_eq(VMap2cond::eval_single("npts<4",  o1), false);
    assert_eq(VMap2cond::eval_single("npts<5",  o1), false);
    assert_eq(VMap2cond::eval_single("npts<6",  o1), true);
    assert_eq(VMap2cond::eval_single("npts<=4", o1), false);
    assert_eq(VMap2cond::eval_single("npts<=5", o1), true);
    assert_eq(VMap2cond::eval_single("npts<=6", o1), true);
    assert_err(VMap2cond::eval_single("npts==", o1), "can't parse value: \"\"");
    assert_err(VMap2cond::eval_single("npts!=", o1), "can't parse value: \"\"");

    // nseg
    assert_eq(VMap2cond::eval_single("nseg==1", o1), true);
    assert_eq(VMap2cond::eval_single("nseg==0", o1), false);
    assert_eq(VMap2cond::eval_single("nseg!=1", o1), false);
    assert_eq(VMap2cond::eval_single("nseg!=0", o1), true);
    assert_eq(VMap2cond::eval_single("nseg>0",  o1), true);
    assert_eq(VMap2cond::eval_single("nseg>1",  o1), false);
    assert_eq(VMap2cond::eval_single("nseg>2",  o1), false);
    assert_eq(VMap2cond::eval_single("nseg>=0", o1), true);
    assert_eq(VMap2cond::eval_single("nseg>=1", o1), true);
    assert_eq(VMap2cond::eval_single("nseg>=2", o1), false);
    assert_eq(VMap2cond::eval_single("nseg<0",  o1), false);
    assert_eq(VMap2cond::eval_single("nseg<1",  o1), false);
    assert_eq(VMap2cond::eval_single("nseg<2",  o1), true);
    assert_eq(VMap2cond::eval_single("nseg<=0", o1), false);
    assert_eq(VMap2cond::eval_single("nseg<=1", o1), true);
    assert_eq(VMap2cond::eval_single("nseg<=2", o1), true);
    assert_err(VMap2cond::eval_single("nseg==", o1), "can't parse value: \"\"");
    assert_err(VMap2cond::eval_single("nseg!=", o1), "can't parse value: \"\"");

    // opts
    o1.opts.put("nn","vv");
    assert_eq(VMap2cond::eval_single("opts[nn]==vv", o1), true);
    assert_eq(VMap2cond::eval_single("opts[nn]!=vv", o1), false);
    assert_eq(VMap2cond::eval_single("opts[nn1]==", o1), true);
    assert_eq(VMap2cond::eval_single("opts[nn1]!=", o1), false);
    assert_eq(VMap2cond::eval_single("opts[nn]==vv1", o1), false);
    assert_eq(VMap2cond::eval_single("opts[nn]!=vv1", o1), true);

    /*****************************************************************/
    // sequences
    assert_err(VMap2cond::eval_sequence({}, o5), "empty condition sequence");
    assert_eq(VMap2cond::eval_sequence({"type==line"}, o1), true);
    assert_eq(VMap2cond::eval_sequence({"type!=line"}, o1), false);
    assert_eq(VMap2cond::eval_sequence({"type==line", "and", "type==line:0x10"}, o1), true);  // true&&true
    assert_eq(VMap2cond::eval_sequence({"type==line", "and", "type==line:0x12"}, o1), false); // true&&false
    assert_eq(VMap2cond::eval_sequence({"type==text", "and", "type==line"}, o1), false);      // false&&true
    assert_eq(VMap2cond::eval_sequence({"type==text", "and", "type==area"}, o1), false);      // false&&false

    assert_eq(VMap2cond::eval_sequence({"type==line", "or", "type==line:0x10"}, o1), true);  // true||true
    assert_eq(VMap2cond::eval_sequence({"type==line", "or", "type==line:0x12"}, o1), true);  // true||false
    assert_eq(VMap2cond::eval_sequence({"type==text", "or", "type==line"}, o1), true);       // false||true
    assert_eq(VMap2cond::eval_sequence({"type==text", "or", "type==area"}, o1), false);      // false||false

    assert_err(VMap2cond::eval_sequence({"type==line", "type==line:0x10"}, o5), "\"and\" or \"or\" expected");
    assert_err(VMap2cond::eval_sequence({"type==line", "and"}, o5), "unterminated condition sequence");

    assert_eq(VMap2cond::eval_sequence({"true","and","true","or","false"}, o1), true); // true&&true||false
    assert_eq(VMap2cond::eval_sequence({"true","and","false","or","true"}, o1), true); // true&&false||true
    assert_eq(VMap2cond::eval_sequence({"true","or","false","and","true"}, o1), true); // true||false&&true
    assert_eq(VMap2cond::eval_sequence({"false","or","false","or","true"}, o1), true); // false||false||true

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
