///\cond HIDDEN (do not show this in Doxyden)

#include <cmath>
#include <cstdlib> // rand
#include "image_cnt.h"
#include "image_trace.h"
#include "image/io.h"
#include "cairo/cairo_wrapper.h"
#include "rainbow/rainbow.h"

// load image from test_dem.tif

int
main(){
  try{

    int w, h;   // grid size
    int mult=3; // image size will be bigger by this factor
    double vmin = NAN, vmax = NAN, vstep = 100, vtol = 2; // contour parameters

    double cmin=0, cmax=3000; // min/max value for the color image


    ImageR img = image_load("test_dem.tif");
    if (img.type()!=IMAGE_16) throw Err() << "wrong image type";
    w = img.width();
    h = img.height();


    // trace rivers
    Rainbow R1(0, 16, "CBb");
    Rainbow R2(0, 16, "MRr");
    ImageR rrd = trace_map_dirs(img, 10000, true);
    ImageR rra = trace_map_areas(rrd);

    ImageR mmd = trace_map_dirs(img, 10000, false);
    ImageR mma = trace_map_areas(mmd);

    ImageR cimg(w,h, IMAGE_32ARGB);
    cimg.fill32(0xFFFFFFFF);

    for (size_t y=0; y<img.height(); y++){
      for (size_t x=0; x<img.width(); x++){

        double r = rra.get_double(x,y);
        double m = mma.get_double(x,y);
        if (r>128) cimg.set32(x, y, R1.get(log(r-128)));
        if (m>128) cimg.set32(x, y, R2.get(log(m-128)));
      }
    }

    CairoWrapper cr;
    cr.set_surface_img(cimg);

    // find and draw contours (no filtering)
    auto ret = image_cnt(img, vmin, vmax, vstep, 0, vtol);
    for (const auto & l:ret)
      cr->mkpath_smline(l.second, 0, 0);
    cr->cap_round();
    cr->set_line_width(0.3);
    cr->set_color_a(0xFF000000);
    cr->stroke();


    // save image
    image_save(cr.get_image(), "image_trace_map.test.png");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
