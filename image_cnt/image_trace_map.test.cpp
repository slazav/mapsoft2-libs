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

    ImageR rrd = trace_map_dirs(img, 10000, true);
    ImageR rrh = trace_map_dh(img, rrd, 0);
    ImageR rra = trace_map_areas(rrd);
    dPoint rrng = rrh.get_double_range();
    Rainbow R1(rrng.x, -200, "bBCW");

    ImageR mmd = trace_map_dirs(img, 10000, false);
    ImageR mmh = trace_map_dh(img, mmd, 50);
    ImageR mma = trace_map_areas(mmd);
    dPoint mrng = mmh.get_double_range();
    Rainbow R2(200, mrng.y, "WMRr");

    ImageR cimg(w,h, IMAGE_32ARGB);
    cimg.fill32(0xFFFFFFFF);

    for (size_t y=0; y<img.height(); y++){
      for (size_t x=0; x<img.width(); x++){
        double ra = rra.get_double(x,y);
        double ma = mma.get_double(x,y);
        double rh = rrh.get_double(x,y);
        double mh = mmh.get_double(x,y);
        if (ra > 256 && rh<0) cimg.set32(x, y, 0xFF0000FF);
        if (ma > 256 && mh>200) cimg.set32(x, y, 0xFF804000);
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
