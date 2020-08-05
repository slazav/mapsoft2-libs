///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "image_t.h"
#include "err/assert_err.h"

int
main(){
  try{

    // test URL template substitutions
    iPoint p(10,20,5);
    {
      ImageT imgt("abc{x}/{y}");
      assert_eq(imgt.make_url(p), "abc10/20");
    }

    {
      ImageT imgt("abc{x}/{y}-{y}/{x} {{} {}}");
      assert_eq(imgt.make_url(p), "abc10/20-20/10 { }");
    }
    {
      ImageT imgt("abc{x}/{y}/{z}-{[abc]}");
      assert_eq(imgt.make_url(p), "abc10/20/5-a");
      assert_eq(imgt.make_url(p), "abc10/20/5-b");
      assert_eq(imgt.make_url(p), "abc10/20/5-c");
      assert_eq(imgt.make_url(p), "abc10/20/5-a");
    }

    {
      ImageT imgt("abc{x/{y}");
      assert_err(imgt.make_url(p), "ImageT: unknown field x/{y in URL template: abc{x/{y}");
    }
    {
      ImageT imgt("abc{x}/{y");
      assert_err(imgt.make_url(p), "ImageT: } is missing in URL template: abc{x}/{y");
    }

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