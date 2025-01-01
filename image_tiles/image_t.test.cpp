///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "image_t.h"
#include "err/assert_err.h"

int
main(){
  try{

    // test URL template substitutions
    iPoint p(10,20,5);

    assert_eq(ImageT::make_url("abc{x}/{y}", p), "abc10/20");
    assert_eq(ImageT::make_url("abc{x}/{y}-{y}/{x} {{} {}}", p), "abc10/20-20/10 { }");

    std::string tmpl("abc{x}/{y}/{z}-{[abc]}");
    assert_eq(ImageT::make_url(tmpl, iPoint(10,20,5)), "abc10/20/5-a");
    assert_eq(ImageT::make_url(tmpl, iPoint(11,20,5)), "abc11/20/5-b");
    assert_eq(ImageT::make_url(tmpl, iPoint(11,21,5)), "abc11/21/5-c");
    assert_eq(ImageT::make_url(tmpl, iPoint(11,22,5)), "abc11/22/5-a");

    assert_err(ImageT::make_url("abc{x/{y}", p),
      "ImageT: unknown field x/{y in URL template: abc{x/{y}");
    assert_err(ImageT::make_url("abc{x}/{y", p),
      "ImageT: } is missing in URL template: abc{x}/{y");

    {
//      ImageT imgt("https://tiles.nakarte.me/eurasia25km/7/{x}/{y}", true);
//      iRect r(87, 78, 3, 2);
//      auto img = imgt.get_image(r*256);
//      image_save(img, "test.jpg");
    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond