///\cond HIDDEN (do not show this in Doxyden)

#include <cmath>
#include <cstdlib> // rand
#include "image_cnt.h"
#include "image_trace.h"
#include "image/io.h"
#include "cairo/cairo_wrapper.h"
#include "rainbow/rainbow.h"

/*
  - use test_dem.tif profile
  - draw it as a color surface, draw contours
  - trace and draw a single ridge and river ( trace_river() function)
  - save to image_trace_river.test.png
*/

int
main(){
  try{

    int w, h;   // grid size
    double vmin = NAN, vmax = NAN, vstep = 100, vtol = 2; // contour parameters
    double cmin=0, cmax=3000; // min/max value for the color image


    ImageR img = image_load("test_dem.tif");
    if (img.type()!=IMAGE_16) throw Err() << "wrong image type";
    w = img.width();
    h = img.height();

    // make color image
    ImageR cimg(w,h, IMAGE_32ARGB);
    Rainbow R(cmin, cmax);
    for (size_t y=0; y<img.height(); y++){
      for (size_t x=0; x<img.width(); x++){
        double v = img.get_double(x,y);
        cimg.set32(x, y, R.get(v));
      }
    }
    CairoWrapper cr;
    cr.set_surface_img(cimg);


    // find and draw contours
    auto ret = image_cnt(img, vmin, vmax, vstep, 0);
    image_cnt_vtol_filter(img, ret, vtol);

    for (const auto & l:ret)
      cr->mkpath_smline(l.second, 0, 0);
    cr->cap_round();
    cr->set_line_width(0.3);
    cr->set_color_a(0xFF000000);
    cr->stroke();

    // trace a river
    cr->cap_round();
    cr->set_line_width(2);
    cr->set_color_a(0xFF0000FF);
    dLine r1 = trace_river(img, iPoint(311,141), 1000, -1, true);
    cr->mkpath_smline(r1, 0, 0);
    cr->stroke();

    // trace a ridge
    cr->set_color_a(0xFF803000);
    r1 = trace_river(img, iPoint(38,158), 10000, -1, false);
    cr->mkpath_smline(r1, 0, 0);
    cr->stroke();

    // save image
    image_save(cr.get_image(), "image_trace_river.test.png");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
