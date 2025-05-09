///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "fig.h"

Fig test_objs(const std::string & in, std::string out = "<in>", int fig_header = 0, int txt7bit = 0){
  std::istringstream s1(in);
  std::ostringstream s2;
  Fig w;
  Opt opts;
  opts.put("fig_header", fig_header);
  opts.put("fig_7bit", txt7bit);
  opts.put("fig_enc", "koi8-r");
  read_fig(s1, w, opts);
  write_fig(s2, w, opts);
  if (out=="<in>") out = in;
  if (s2.str() != out) {
    std:: cerr << "\nIN:  [" << in  << "]\n";
    std:: cerr << "OUT: [" << out << "]\n";
    std:: cerr << "RET: [" << s2.str() << "]\n";
  }
  assert_eq(s2.str(), out);
  return w;
}


int
main(){
  try {

    // static members
    assert_eq(Fig::colors.size(), 33);
    assert_eq(Fig::colors.find(-1)->second, 0);
    assert_eq(Fig::colors.find(0)->second, 0);
    assert_eq(Fig::colors.find(1)->second, 0x0000ff);
    assert_eq(Fig::colors.find(10)->second, 0x0000d0);
    assert_eq(Fig::colors.find(31)->second, 0xffd700);

    assert_eq(Fig::psfonts.size(), 36);
    assert_eq(Fig::psfonts.find(-1)->second, "Default");
    assert_eq(Fig::psfonts.find(0)->second, "Times-Roman");
    assert_eq(Fig::psfonts.find(34)->second, "ZapfDingbats");

    assert_eq(Fig::texfonts.size(), 6);
    assert_eq(Fig::texfonts.find(0)->second, "Default");
    assert_eq(Fig::texfonts.find(1)->second, "Roman");
    assert_eq(Fig::texfonts.find(5)->second, "Typewriter");

    assert_eq(Fig::cm2fig, 450);
    assert_eq(Fig::fig2cm, 1.0 / Fig::cm2fig);

    // constructing objects from a header string -- ellipse
    {
      FigObj o;
      read_figobj_header(o, "1 3 0 2 0 7 50 -1 -1 0.000 1 0.0000 7380 5310 664 664 7380 5310 7695 4725");
      assert(o.is_ellipse());
      assert(o.is_closed());
      assert_eq(o.size(),1); // center
      assert_eq(o[0].x,7380);
      assert_eq(o[0].y,5310);

      assert_eq(o.sub_type,3);
      assert_eq(o.line_style,0);
      assert_eq(o.thickness,2);
      assert_eq(o.pen_color,0);
      assert_eq(o.fill_color,0xffffff);
      assert_eq(o.depth,50);
      assert_eq(o.pen_style,-1);
      assert_eq(o.area_fill,-1);
      assert_eq(o.style_val,0);
      assert_eq(o.direction,1);
      assert_eq(o.angle,0);
      assert_eq(o.radius_x,664);
      assert_eq(o.radius_y,664);
      assert_eq(o.start_x,7380);
      assert_eq(o.start_y,5310);
      assert_eq(o.end_x,7695);
      assert_eq(o.end_y,4725);

      // template
      auto o1 = read_figobj_header("1 3 0 2 1 7 50 -1 -1 0.000 1 0.0000 7380 5310 664 664 7380 5310 7695 4725");
      auto o2 = figobj_template("1 3 0 2 1 7 50 -1 -1 0.000 1 0.0000");
      assert_eq(o1.sub_type,   o2.sub_type);
      assert_eq(o1.line_style, o2.line_style);
      assert_eq(o1.thickness,  o2.thickness);
      assert_eq(o1.pen_color,  o2.pen_color);
      assert_eq(o1.fill_color, o2.fill_color);
      assert_eq(o1.depth,      o2.depth);
      assert_eq(o1.pen_style,  o2.pen_style);
      assert_eq(o1.area_fill,  o2.area_fill);
      assert_eq(o1.style_val,  o2.style_val);
      assert_eq(o1.direction,  o2.direction);
      assert_eq(o1.angle,      o2.angle);

      // rgb colors
      o2 = figobj_template("1 3 0 2 #abcdef #AA00cc 50 -1 -1 0.000 1 0.0000");
      assert_eq(o2.pen_color,0xabcdef);
      assert_eq(o2.fill_color,0xaa00cc);

    }

    {
      FigObj o;
      read_figobj_header(o, "1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 405 8765 4092 10990 2658");
      assert(o.is_ellipse());
      assert(o.is_closed());
      assert_eq(o.size(),1); // center
      assert_eq(o[0].x,9877);
      assert_eq(o[0].y,3375);

      assert_eq(o.sub_type,2);
      assert_eq(o.line_style,0);
      assert_eq(o.thickness,1);
      assert_eq(o.pen_color,0);
      assert_eq(o.fill_color,0xffffff);
      assert_eq(o.depth,50);
      assert_eq(o.pen_style,-1);
      assert_eq(o.area_fill,-1);
      assert_eq(o.style_val,0);
      assert_eq(o.direction,1);
      assert(fabs(o.angle-0.2618)<1e-6);
      assert_eq(o.radius_x,1260);
      assert_eq(o.radius_y,405);
      assert_eq(o.start_x,8765);
      assert_eq(o.start_y,4092);
      assert_eq(o.end_x,10990);
      assert_eq(o.end_y,2658);

      // template
      auto o1 = read_figobj_header("1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 405 8765 4092 10990 2658");
      auto o2 = figobj_template("1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618");
      assert_eq(o1.sub_type,   o2.sub_type);
      assert_eq(o1.line_style, o2.line_style);
      assert_eq(o1.thickness,  o2.thickness);
      assert_eq(o1.pen_color,  o2.pen_color);
      assert_eq(o1.fill_color, o2.fill_color);
      assert_eq(o1.depth,      o2.depth);
      assert_eq(o1.pen_style,  o2.pen_style);
      assert_eq(o1.area_fill,  o2.area_fill);
      assert_eq(o1.style_val,  o2.style_val);
      assert_eq(o1.direction,  o2.direction);
      assert_eq(o1.angle,      o2.angle);

    }

    {
      FigObj o;
      assert_err(
        read_figobj_header(o,
          "1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 405 8765 4092 10990"),
        "FigObj: can't read ellipse object: "
          "[1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 405 8765 4092 10990]");

      assert_err(
        read_figobj_header(o,
          "1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 x 8765 4092 10990 2658"),
        "FigObj: can't read ellipse object: "
          "[1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 x 8765 4092 10990 2658]");

      assert_err(
        read_figobj_header(o,
          "1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 405 8765 4092 10990 2658 1"),
        "FigObj: can't read ellipse object: "
          "[1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 405 8765 4092 10990 2658 1]");

    }

    // constructing objects from a header string -- lines
    {
      FigObj o;
      int size = read_figobj_header(o, "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 4");
      assert(o.is_polyline());
      assert(!o.is_closed());
      assert_eq(size,4);

      assert_eq(o.sub_type,1);
      assert_eq(o.line_style,0);
      assert_eq(o.thickness,1);
      assert_eq(o.pen_color,0);
      assert_eq(o.fill_color,0xffffff);
      assert_eq(o.depth,50);
      assert_eq(o.pen_style,-1);
      assert_eq(o.area_fill,-1);
      assert_eq(o.style_val,0);
      assert_eq(o.join_style,0);
      assert_eq(o.cap_style,0);
      assert_eq(o.radius,-1);
      assert_eq(o.forward_arrow,0);
      assert_eq(o.backward_arrow,0);

      // template
      auto o1 = read_figobj_header("2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 4");
      auto o2 = figobj_template("2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0");
      assert_eq(o1.sub_type,   o2.sub_type);
      assert_eq(o1.line_style, o2.line_style);
      assert_eq(o1.thickness,  o2.thickness);
      assert_eq(o1.pen_color,  o2.pen_color);
      assert_eq(o1.fill_color, o2.fill_color);
      assert_eq(o1.depth,      o2.depth);
      assert_eq(o1.pen_style,  o2.pen_style);
      assert_eq(o1.area_fill,  o2.area_fill);
      assert_eq(o1.style_val,  o2.style_val);
      assert_eq(o1.join_style, o2.join_style);
      assert_eq(o1.cap_style,  o2.cap_style);
      assert_eq(o1.radius,     o2.radius);
      assert_eq(o1.forward_arrow,  o2.forward_arrow);
      assert_eq(o1.backward_arrow,  o2.backward_arrow);

      //arrows
      o2 = figobj_template("2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 1 0  1 2 2.00 65.00 125.00");
      assert_eq(o2.farrow_type,  1);
      assert_eq(o2.farrow_style, 2);
      assert_eq(o2.farrow_thickness, 2);
      assert_eq(o2.farrow_width, 65);
      assert_eq(o2.farrow_height, 125);

      assert_eq(o2.barrow_type,  0);
      assert_eq(o2.barrow_style, 0);
      assert_eq(o2.barrow_thickness, 1);
      assert_eq(o2.barrow_width, 60);
      assert_eq(o2.barrow_height, 30);

      o2 = figobj_template("2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 1  2 1 3.00 64 124");

      assert_eq(o2.farrow_type,  0);
      assert_eq(o2.farrow_style, 0);
      assert_eq(o2.farrow_thickness, 1);
      assert_eq(o2.farrow_width, 60);
      assert_eq(o2.farrow_height, 30);

      assert_eq(o2.barrow_type,  2);
      assert_eq(o2.barrow_style, 1);
      assert_eq(o2.barrow_thickness, 3);
      assert_eq(o2.barrow_width, 64);
      assert_eq(o2.barrow_height, 124);

      o2 = figobj_template("2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1  1 1  2 2 2 3 4  3 3 5 6 7");

      assert_eq(o2.farrow_type,  2);
      assert_eq(o2.farrow_style, 2);
      assert_eq(o2.farrow_thickness, 2);
      assert_eq(o2.farrow_width, 3);
      assert_eq(o2.farrow_height, 4);

      assert_eq(o2.barrow_type,  3);
      assert_eq(o2.barrow_style, 3);
      assert_eq(o2.barrow_thickness, 5);
      assert_eq(o2.barrow_width, 6);
      assert_eq(o2.barrow_height, 7);

    }

    {
      FigObj o;
      assert_err(
        read_figobj_header(o, "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0"),
        "FigObj: can't read line object: "
          "[2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0]");
      assert_err(
        read_figobj_header(o, "2 1 0.1 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 4"),
        "FigObj: can't read line object: "
         "[2 1 0.1 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 4]");
      assert_err(
        read_figobj_header(o, "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 4 1"),
        "FigObj: can't read line object: "
          "[2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 4 1]");
    }

    // constructing objects from a header string -- splines
    {
      FigObj o;
      int size = read_figobj_header(o, "3 0 0 1 0 7 50 -1 -1 0.000 0 0 0 4");
      assert(o.is_spline());
      assert(!o.is_closed());
      assert_eq(size,4);

      assert_eq(o.sub_type,0);
      assert_eq(o.line_style,0);
      assert_eq(o.thickness,1);
      assert_eq(o.pen_color,0);
      assert_eq(o.fill_color,0xffffff);
      assert_eq(o.depth,50);
      assert_eq(o.pen_style,-1);
      assert_eq(o.area_fill,-1);
      assert_eq(o.style_val,0);
      assert_eq(o.cap_style,0);
      assert_eq(o.forward_arrow,0);
      assert_eq(o.backward_arrow,0);

      // template
      auto o1 = read_figobj_header("3 0 0 1 0 7 50 -1 -1 0.000 0 0 0 4");
      auto o2 = figobj_template("3 0 0 1 0 7 50 -1 -1 0.000 0 0 0");
      assert_eq(o1.sub_type,   o2.sub_type);
      assert_eq(o1.line_style, o2.line_style);
      assert_eq(o1.thickness,  o2.thickness);
      assert_eq(o1.pen_color,  o2.pen_color);
      assert_eq(o1.fill_color, o2.fill_color);
      assert_eq(o1.depth,      o2.depth);
      assert_eq(o1.pen_style,  o2.pen_style);
      assert_eq(o1.area_fill,  o2.area_fill);
      assert_eq(o1.style_val,  o2.style_val);
      assert_eq(o1.cap_style,  o2.cap_style);
      assert_eq(o1.forward_arrow,  o2.forward_arrow);
      assert_eq(o1.backward_arrow,  o2.backward_arrow);

    }

    {
      FigObj o;
      assert_err(
        read_figobj_header(o, "3 0 0 1 0 7 50 -1 -1 0.000 0 0 0"),
        "FigObj: can't read spline object: "
        "[3 0 0 1 0 7 50 -1 -1 0.000 0 0 0]");

      assert_err(
        read_figobj_header(o, "3 0 0 1 0 7 50 -1 -1 0.000 a 0 0 4"),
        "FigObj: can't read spline object: "
        "[3 0 0 1 0 7 50 -1 -1 0.000 a 0 0 4]");

      assert_err(
        read_figobj_header(o, "3 0 0 1 0 7 50 -1 -1 0.000 0 0 0 4 1"),
        "FigObj: can't read spline object: "
         "[3 0 0 1 0 7 50 -1 -1 0.000 0 0 0 4 1]");
    }

    // constructing objects from a header string -- text
    {
      FigObj o;
      read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840 text\\001");
      assert(o.is_text());
      assert(!o.is_closed());
      assert_eq(o.size(),1);
      assert_eq(o[0].x,-1575);
      assert_eq(o[0].y,6840);

      assert_eq(o.sub_type,0);
      assert_eq(o.pen_color,0);
      assert_eq(o.depth,50);
      assert_eq(o.pen_style,-1);
      assert_eq(o.font,0);
      assert_eq(o.font_size,12);
      assert_eq(o.angle,0.0);
      assert_eq(o.font_flags,4);
      assert_eq(o.height,210);
      assert_eq(o.length,300);
      assert_eq(o.text,"text");

      // template
      auto o1 = read_figobj_header("4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840 text\\001");
      auto o2 = figobj_template("4 0 0 50 -1 0 12 0.0000 4");
      assert_eq(o1.sub_type,   o2.sub_type);
      assert_eq(o1.pen_color,  o2.pen_color);
      assert_eq(o1.depth,      o2.depth);
      assert_eq(o1.pen_style,  o2.pen_style);
      assert_eq(o1.font,       o2.font);
      assert_eq(o1.font_size,  o2.font_size);
      assert_eq(o1.angle,      o2.angle);
      assert_eq(o1.font_flags, o2.font_flags);
    }

    {
      FigObj o;
      assert_err(
        read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 z -1575 6840 text\\001"),
        "FigObj: can't read text object: "
        "[4 0 0 50 -1 0 12 0.0000 4 210 z -1575 6840 text\\001]");

      assert_err(
        read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840"),
        "FigObj: can't read text object: "
        "[4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840]");
    }

    {
      FigObj o;
      int m;
      m = read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840 text\\001");
      assert_eq(o.text, "text");

      m = read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840 te\\xt\\001");
      assert_eq(o.text, "text");

      m = read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840 te\\170t\\001");
      assert_eq(o.text, "text");

      m = read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840 te\\170t\\001aaa");
      assert_eq(o.text, "text");
      assert_eq(m, 0);

      m = read_figobj_header(o, "4 0 0 50 -1 0 12 0.0000 4 210 300 -1575 6840 text");
      assert_eq(o.text, "text");
      assert_eq(m, 1);
    }

    // constructing objects from a header string -- arc
    {
      FigObj o;
      read_figobj_header(o, "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 7380\n");
      assert(o.is_arc());
      assert(!o.is_closed());
      assert_eq(o.size(),3);
      assert_eq(o[0].x,3420);
      assert_eq(o[0].y,7380);
      assert_eq(o[1].x,3870);
      assert_eq(o[1].y,6975);
      assert_eq(o[2].x,4410);
      assert_eq(o[2].y,7380);

      assert_eq(o.sub_type,1);
      assert_eq(o.line_style,0);
      assert_eq(o.thickness,1);
      assert_eq(o.pen_color,0);
      assert_eq(o.fill_color,0xffffff);
      assert_eq(o.depth,50);
      assert_eq(o.pen_style,-1);
      assert_eq(o.area_fill,-1);
      assert_eq(o.style_val,0);
      assert_eq(o.cap_style,0);
      assert_eq(o.direction,0);
      assert_eq(o.forward_arrow,0);
      assert_eq(o.backward_arrow,0);
      assert_eq(o.center_x,3915.000);
      assert_eq(o.center_y,7477.500);

      // template
      auto o1 = read_figobj_header("5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 7380");
      auto o2 = figobj_template("5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0");
      assert_eq(o1.sub_type,   o2.sub_type);
      assert_eq(o1.line_style, o2.line_style);
      assert_eq(o1.thickness,  o2.thickness);
      assert_eq(o1.pen_color,  o2.pen_color);
      assert_eq(o1.fill_color, o2.fill_color);
      assert_eq(o1.depth,      o2.depth);
      assert_eq(o1.pen_style,  o2.pen_style);
      assert_eq(o1.area_fill,  o2.area_fill);
      assert_eq(o1.style_val,  o2.style_val);
      assert_eq(o1.cap_style,  o2.cap_style);
      assert_eq(o1.forward_arrow,  o2.forward_arrow);
      assert_eq(o1.backward_arrow,  o2.backward_arrow);

    }

    {
      FigObj o;
      assert_err(
        read_figobj_header(o, "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 x"),
        "FigObj: can't read arc object: "
        "[5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 x]");

      assert_err(
        read_figobj_header(o, "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 7380 1"),
        "FigObj: can't read arc object: "
        "[5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 7380 1]");

      assert_err(
        read_figobj_header(o, "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410"),
        "FigObj: can't read arc object: "
        "[5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410]");
    }


    // constructing objects from a header string -- compound
    {
      FigObj o;
      read_figobj_header(o, "6");
      assert(o.is_compound());
      assert(!o.is_closed());
      assert_eq(o.size(),2);
      assert_eq(o[0].x,0);
      assert_eq(o[0].y,0);
      assert_eq(o[1].x,0);
      assert_eq(o[1].y,0);
    }

    {
      FigObj o;
      read_figobj_header(o, "6 1 2 3 4");
      assert(o.is_compound());
      assert(!o.is_closed());
      assert_eq(o.size(),2);
      assert_eq(o[0].x,1);
      assert_eq(o[0].y,2);
      assert_eq(o[1].x,3);
      assert_eq(o[1].y,4);
    }

    {
      FigObj o;
      assert_err(
        read_figobj_header(o, "6 1 2 3"),
        "FigObj: can't read compound object: [6 1 2 3]");

      assert_err(
        read_figobj_header(o, "6 1 2 3 4 1"),
        "FigObj: can't read compound object: [6 1 2 3 4 1]");

      assert_err(
        read_figobj_header(o, "6 1 2 3.2 4"),
        "FigObj: can't read compound object: [6 1 2 3.2 4]");
    }

    {
      FigObj o;
      read_figobj_header(o, "-6");
      assert(o.is_compound_end());
      assert(!o.is_closed());
      assert_eq(o.size(),0);
    }

    {
      FigObj o;
      assert_err(
        read_figobj_header(o, "-6 1"),
        "FigObj: can't read compound end object: [-6 1]");
    }

    /*********************************************************/
    // reading/writing objects

    // arc with arrow, options, comments
    test_objs(
      "# comment1\n"
      "\n"
      "# \\opt1=val1\n"
      "# \\opt2=\n"
      "# \\opt3\n"
      "# comment2\n"
      "5 1 1 1 0 7 50 -1 -1 4.000 0 0 1 0 3545.445 7246.849 3150 6930 3645 6750 4050 7200\n"
      "\t0 0 1.00 60.00 120.00\n",
      "# comment1\n"
      "# \\opt1=val1\n"
      "# \\opt2=\n"
      "# \\opt3\n"
      "# comment2\n"
      "5 1 1 1 0 7 50 -1 -1 4.000 0 0 1 0 3545.445 7246.849 3150 6930 3645 6750 4050 7200\n"
      "\t0 0 1.00 60.00 120.00\n");

    // object without trailing newline
    test_objs(
    "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 7380",
    "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 7380\n");

    // fix closed line, add one more point
    test_objs(
    "2 3 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 5\n"
    "\t945 2250 1935 1935 2925 2430 2790 3240 1170 3105\n",
    "2 3 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 6\n"
    "\t945 2250 1935 1935 2925 2430 2790 3240 1170 3105 945 2250\n");

    // color -1 is converted to -1
    test_objs(
    "2 1 0 1 -1 7 50 -1 -1 0.000 0 0 -1 0 0 5\n"
    "\t945 2250 1935 1935 2925 2430 2790 3240 1170 3105\n",
    "2 1 0 1 -1 7 50 -1 -1 0.000 0 0 -1 0 0 5\n"
    "\t945 2250 1935 1935 2925 2430 2790 3240 1170 3105\n");

    {
    // colors
    Fig w = test_objs(
    "0 32 #7a8d00\n"
    "0 33 #7a8d10\n"
    "2 1 0 1 32 33 50 -1 -1 0.000 0 0 -1 0 0 5\n"
    "\t945 2250 1935 1935 2925 2430 2790 3240 1170 3105\n"
    "2 1 0 1 1 11 50 -1 -1 0.000 0 0 -1 0 0 4\n"
    "\t315 1440 810 945 1800 1215 3330 675\n",

    "0 32 #7a8d00\n"
    "0 33 #7a8d10\n"
    "2 1 0 1 32 33 50 -1 -1 0.000 0 0 -1 0 0 5\n"
    "\t945 2250 1935 1935 2925 2430 2790 3240 1170 3105\n"
    "2 1 0 1 1 11 50 -1 -1 0.000 0 0 -1 0 0 4\n"
    "\t315 1440 810 945 1800 1215 3330 675\n");

      assert_eq(w.size(),2);
      FigObj o1 = *w.begin();
      FigObj o2 = *(w.rbegin());
      assert(o1.pen_color = 0x7a8d00);
      assert(o1.fill_color = 0x7a8d10);
      assert(o2.pen_color = 0x0000ff);
      assert(o2.fill_color = 0x87ceff);
    }

    {
    // multiple objects: ellipses, lines, images
    Fig w = test_objs(
    "# comment1\n"
    "# comment2\n"
    "5 1 0 1 0 7 50 -1 -1 0.000 0 0 0 0 3915.000 7477.500 3420 7380 3870 6975 4410 7380\n"
    "1 3 0 1 0 7 50 -1 -1 0.000 1 0.0000 7380 5310 664 664 7380 5310 7695 4725\n"
    "1 2 0 1 0 7 50 -1 -1 0.000 1 0.2618 9877 3375 1260 405 8765 4092 10990 2658\n"
    "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 1 1 4\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t315 1440 810 945 1800 1215 3330 675\n"
    "2 3 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 6\n"
    "\t945 2250 1935 1935 2925 2430 2790 3240 1170 3105 945 2250\n"
    "2 3 0 1 0 7 50 -1 -1 0.000 0 0 0 0 0 7\n"
    "\t6840 7155 6693 6720 6243 6630 5940 6975 6087 7410 6537 7500\n"
    "\t6840 7155\n"
    "2 5 0 1 0 0 50 -1 -1 0.000 0 0 -1 0 0 5\n"
    "\t0 <empty>\n"
    "\t-2610 5085 -1620 5085 -1620 5580 -2610 5580 -2610 5085\n"
    "2 5 0 1 0 0 50 -1 -1 0.000 0 0 -1 0 0 5\n"
    "\t0 some file.jpg\n"
    "\t-2610 5085 -1620 5085 -1620 5580 -2610 5580 -2610 5085\n"
    "3 0 0 1 0 7 50 -1 -1 0.000 0 0 1 4\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t495 3510 1080 4410 2295 4230 3825 4500\n"
    "\t0.000 1.000 1.000 0.000\n"
    "3 5 0 1 0 7 50 -1 -1 0.000 0 0 0 5\n"
    "\t5175 5760 5679 5206 6372 5532 5959 4339 4812 4926\n"
    "\t0.000 -0.500 -0.600 -0.500 1.000\n"
    "4 0 0 50 -1 0 12.000 0.0000 4 210 150 -1575 6840 text\\001\n"
    );
    assert_eq(w.size(), 11);
    }

    // text -> 7 bit
    {
      Fig w = test_objs(
      "4 0 0 50 -1 0 12 0.0000 4 210 150 -1575 6840 text\\\\120\\120\\331\\001\n"
      "4 0 0 50 -1 0 12.0 0.0000 4 210 150 -1575 6840 text\\\\120\\120\\331\\001\n"
      ,
      "4 0 0 50 -1 0 12.000 0.0000 4 210 150 -1575 6840 text\\\\120P\\331\\001\n"
      "4 0 0 50 -1 0 12.000 0.0000 4 210 150 -1575 6840 text\\\\120P\\331\\001\n"
      ,0,1);
    }

    // Fig with header
    // - no \nnn character conversion in comments,
    // - \nnn character conversion in text
    {
    Fig w = test_objs(
    "#FIG 3.2 ...\n"
    "Landscape\n"
    "Center\n"
    "Metric \n"
    " A4\n"
    "100.00\n"
    "Single\n"
    "-2\n"
    "# comm1\\\\120\\120\n"
    "# \\opt1=val1\n"
    "# \n"
    "# comm2\n"
    "# \\opt2\n"
    "# \\opt3=\n"
    "\n"
    "1200 2\n"
    "4 0 0 50 -1 0 12 0.0000 4 210 150 -1575 6840 text\\\\120\\120\\331\\001\n"
    "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 1 1 4\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t315 1440 810 945 1800 1215 3330 675\n"
    ,
    "#FIG 3.2\n"
    "Landscape\n"
    "Center\n"
    "Metric\n"
    "A4\n"
    "100.00\n"
    "Single\n"
    "-2\n"
    "# comm1\\\\120\\120\n"
    "# \\opt1=val1\n"
    "# \n"
    "# comm2\n"
    "# \\opt2\n"
    "# \\opt3=\n"
    "1200 2\n"
    "4 0 0 50 -1 0 12.000 0.0000 4 210 150 -1575 6840 text\\\\120P�\\001\n"
    "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 1 1 4\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t315 1440 810 945 1800 1215 3330 675\n"
    , 1);
    assert_eq(w.comment.size(), 6);
    assert_eq(w.comment[0], "comm1\\\\120\\120");
    assert_eq(w.size(), 2);
    assert_eq(w.begin()->text, "text\\120Pы");
    }

    {
    // encodings
    Fig w = test_objs(
    "#FIG 3.2\n"
    "Landscape\n"
    "Center\n"
    "Metric\n"
    "A4\n"
    "100.00\n"
    "Single\n"
    "-2\n"
    "# �������1\n"
    "# \\����1=��������1\n"
    "1200 2\n"
    "# �������2\n"
    "# \\����2=��������2\n"
    "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 1 1 4\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t0 0 1.00 60.00 120.00\n"
    "\t315 1440 810 945 1800 1215 3330 675\n"
    "4 0 0 50 -1 0 12.000 0.0000 4 210 150 -1575 6840 �����\\001\n",
    "<in>", 1);
    assert_eq(w.size(), 2);
    assert_eq(w.comment.size(),2);
    assert_eq(w.comment[0],"коммент1");
    assert_eq(w.comment[1],"\\ключ1=значение1");
    assert_eq(w.begin()->comment.size(),2);
    assert_eq(w.begin()->comment[0],"коммент2");
    assert_eq(w.begin()->comment[1],"\\ключ2=значение2");
    assert(w.rbegin()->is_text());
    assert_eq(w.rbegin()->text, "текст");
    }

    { // write comments

    Fig fig;
    auto o = figobj_template("4 0 0 50 -1 0 12 0.0000 4");
    o.comment.push_back("abc");
    o.comment.push_back("de\nfg\nhi");
    o.comment.push_back("12\n\n34\n");
    o.push_back(iPoint(0,0));
    fig.push_back(o);
    Opt opts;
    opts.put("fig_header", false);
    std::ostringstream ss;
    write_fig(ss, fig, opts);
    assert_eq(ss.str(), "# abc\n# de\n# fg\n# hi\n# 12\n# \n# 34\n# \n"
                        "4 0 0 50 -1 0 12.000 0.0000 4 0 0 0 0 \\001\n");

    }

  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
