///\cond HIDDEN (do not show this in Doxyden)

#include <unistd.h>
#include <cassert>
#include <iostream>
#include "err/assert_err.h"
#include "vmap2obj.h"

using namespace std;

int
main(){
  try{

    // VMap2obj types
    {
      assert_eq(VMap2obj::make_type(VMAP2_POINT,   0xFFFF), 0xFFFF);
      assert_eq(VMap2obj::make_type(VMAP2_LINE,    0xFFFF), 0x100FFFF);
      assert_eq(VMap2obj::make_type(VMAP2_POLYGON, 0xFFFF), 0x200FFFF);

      assert_eq(VMap2obj::make_type("point:0xFFFF"), 0xFFFF);
      assert_eq(VMap2obj::make_type("line:0xFFFF"),  0x100FFFF);
      assert_eq(VMap2obj::make_type("area:0xFFFF"),  0x200FFFF);
      assert_eq(VMap2obj::make_type("line:0xFFFFFF"),  0x1FFFFFF);
      assert_eq(VMap2obj::make_type("area:0xFFFFFF"),  0x2FFFFFF);

      assert_eq(VMap2obj::make_type("point:123"), 123);

      std::string err("bad VMAP2 object type, (point|line|area):<number> expected: ");
      assert_err(VMap2obj::make_type(""), "can't parse VMAP2 object type: empty string");
      assert_err(VMap2obj::make_type("a"), "can't parse VMAP2 object type \"a\": ':' separator not found");
      assert_err(VMap2obj::make_type("point:"), "can't parse VMAP2 object type \"point:\": can't parse value: \"\"");
      assert_err(VMap2obj::make_type("point:a"), "can't parse VMAP2 object type \"point:a\": can't parse value: \"a\"");
      assert_err(VMap2obj::make_type("point:0x1FFFFFF"), "can't parse VMAP2 object type \"point:0x1FFFFFF\": too large number");
      assert_err(VMap2obj::make_type("abc:0xFFFF"), "can't parse VMAP2 object type \"abc:0xFFFF\": point, line, area, or text word expected");

      assert_eq(VMap2obj::print_type(0x123), "point:0x123");
      assert_eq(VMap2obj::print_type(0x100FFFF), "line:0xffff");
      assert_eq(VMap2obj::print_type(0x200FFFF), "area:0xffff");
      assert_eq(VMap2obj::print_type(0x300FFFF), "text:0xffff");
      assert_eq(VMap2obj::print_type(0x400FFFF), "unknown:0xffff");

      assert_eq(VMap2obj::print_type(0x312FFFF), "text:0x12ffff");
      assert_eq(VMap2obj::print_type(0x412FFFF), "unknown:0x12ffff");
      assert_eq(VMap2obj::print_type(0xEFFFFFFF), "unknown:0xffffff");
      assert_eq(VMap2obj::print_type(0xFFFFFFFF), "none");


    }

    // VMap2obj structure
    {
      VMap2obj o1,o2;
      // defaults
      assert_eq(o1.get_class(), VMAP2_POINT);
      assert_eq(o1.get_tnum(),  0);
      assert_eq(o1.scale,       1.0);
      assert_eq(o1.align,       VMAP2_ALIGN_SW);
      assert_eq(isnan(o1.angle), true);
      assert_eq(o1.name, "");
      assert_eq(o1.comm, "");
      assert_eq(o1.opts.size(), 0);
      assert_eq(o1.size(), 0);

      // <=> operators
      assert_eq(o1, o2);
      assert(o1 >= o2);
      assert(o1 <= o2);
      o2.set_type(VMAP2_LINE, 0);
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o2.set_type(VMAP2_LINE, 1);
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1; o2.angle = 10;
      assert(o1 != o2);
      assert(o1 < o2); // nan < then any number
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o1.name = "a";
      o2.name = "b";
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o1.comm = "a";
      o2.comm = "b";
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2=o1;
      o1.opts.put("a", "1");
      o2.opts.put("b", "2");
      assert(o1 != o2);
      assert(o1 < o2);
      assert(o1 <= o2);
      assert(o2 > o1);
      assert(o2 >= o1);

      o2.opts.put("a", "1");
      o1.opts.put("b", "2");
      assert_eq(o1, o2);
      assert(o1 <= o2);
      assert(o2 >= o1);

    }

    // packing and unpacking of VMap2obj
    {
      VMap2obj o1,o2;
      o1.set_type(VMAP2_LINE, 0x2342);
      o1.angle  = 60;
      o1.scale  = 2.5;
      o1.align  = VMAP2_ALIGN_C;
      o1.name = "object name\nsecond line";
      o1.comm = "object comment\nsecond line";
      o1.opts.put("opt1", "");
      o1.opts.put("opt2", "1");
      o1.opts.put("opt3", "2");
      o1.set_coords("[[[0,0],[1,1]],[[1,1],[2,2]]]");
      assert_err(o1.set_coords("[0,0]"), "can't parse multisegment line: \"[0,0]\": a JSON array expected");
      assert_eq(dMultiLine(o1), dMultiLine("[[[0,0],[1,1]],[[1,1],[2,2]]]"));

      // pack/upack
      {
        std::string pack = VMap2obj::pack(o1);
        o2 = VMap2obj::unpack(pack);
        assert_eq(o1,o2);
      }

      //write/read
      {
        std::ostringstream s1;
        VMap2obj::write(s1,o1);
        std::istringstream s2(s1.str());
        o2 = VMap2obj::read(s2);
        assert_eq(o1,o2);
      }

      o1.set_type(VMAP2_POINT, 0x12);
      o1.angle  = 0;
      o1.scale  = 1.0;
      o1.align  = VMAP2_ALIGN_NW;
      o1.name = "";
      o1.comm = "";
      o1.opts.clear();

      // pack/upack
      {
        std::string pack = VMap2obj::pack(o1);
        o2 = VMap2obj::unpack(pack);
        assert_eq(o1,o2);
      }

      //write/read
      {
        std::ostringstream s1;
        VMap2obj::write(s1,o1);
        std::istringstream s2(s1.str());
        o2 = VMap2obj::read(s2);
        assert_eq(o1,o2);
      }

      o1.set_type(VMAP2_POINT, 0x2342);
      o1.set_coords("[1,1]");
      assert_err(o1.set_coords("[[0,0],[1,1]]"), "can't parse point: \"[[0,0],[1,1]]\": can't extract a number from array index 0");
      assert_eq(dMultiLine(o1), dMultiLine("[[[1,1]]]"));
    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
