///\cond HIDDEN (do not show this in Doxyden)

#include <cmath>
#include <cstdlib> // rand
#include "image_cnt.h"
#include "image_trace.h"
#include "image/io.h"
#include "cairo/cairo_wrapper.h"
#include "rainbow/rainbow.h"

double plane_dev(const Image & dem, const iPoint & p, const int r);
double perp_dev(const Image & dem, const iPoint & p, const int dir, const int r);

/*
  - use test_dem.tif profile
  - draw color surface using plane_dev() function from image_trace.cpp
    (RMS difference from a best-fit plane 2r+1 x 2r+1)
  - draw contours
  - save to image_plane.test.png
*/

int
main(){
  try{

    int w, h;   // grid size
    double vmin = NAN, vmax = NAN, vstep = 100, vtol = 2; // contour parameters
    double cmin=0, cmax=8; // min/max value for the color image


    ImageR img = image_load("test_dem.tif");
    if (img.type()!=IMAGE_16) throw Err() << "wrong image type";
    w = img.width();
    h = img.height();

    // trace directions
//    bool down = false;
//    ImageR dirs = trace_map_dirs(img, 10000, down);

    // make color image
    ImageR cimg(w,h, IMAGE_32ARGB);
    Rainbow R(cmin, cmax);

    for (size_t y=0; y<img.height(); y++){
      for (size_t x=0; x<img.width(); x++){
        double v = plane_dev(img, iPoint(x,y), 3);
        cimg.set32(x, y, R.get(v));
      }
    }
    CairoWrapper cr;
    cr.set_surface_img(cimg);


    // find and draw contours (no filtering)
    auto ret = image_cnt(img, vmin, vmax, vstep, 0);
    image_cnt_vtol_filter(img, ret, vtol);

    for (const auto & l:ret)
      cr->mkpath_smline(l.second, 0, 0);
    cr->cap_round();
    cr->set_line_width(0.3);
    cr->set_color_a(0xFF000000);
    cr->stroke();

    image_save(cr.get_image(), "image_plane.test.png");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
