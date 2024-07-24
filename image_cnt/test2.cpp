///\cond HIDDEN (do not show this in Doxyden)

#include <cmath>
#include <cstdlib> // rand
#include "image_cnt.h"
#include "image/io.h"
#include "cairo/cairo_wrapper.h"
#include "rainbow/rainbow.h"

// load image from test_dem.tif

int
main(){
  try{

    int w, h;   // grid size
    int mult=5; // image size will be bigget by this factor

    double step = 100; // countour step
    double vmin = 0; // countour start value
    double vmax = NAN; // countour end value

    double cmin=0, cmax=3000; // min/max value for the color image

    double vtol = 5; // tolerance for contour filtering
    bool closed = 1;    // produce closed/open contours

    ImageR img = image_load("test_dem.tif");
    if (img.type()!=IMAGE_16) throw Err() << "wrong image type";
    w = img.width();
    h = img.height();

    // make color image
    ImageR cimg(w*mult,h*mult, IMAGE_32ARGB);
    Rainbow R(cmin, cmax);
    for (size_t y=0; y<img.height(); y++){
      for (size_t x=0; x<img.width(); x++){
        double v = img.get_double(x,y);
        for (size_t dx=0; dx<mult; dx++){
          for (size_t dy=0; dy<mult; dy++){
            cimg.set32(mult*x+dx, mult*y+dy, R.get(v));
          }
        }
      }
    }
    CairoWrapper cr;
    cr.set_surface_img(cimg);


    // find and draw contours (no filtering)
    auto ret = image_cnt(img, vmin, vmax, step, closed, 0);
    for (const auto & l:ret)
      cr->mkpath_smline((double)mult*l.second, 0, 0);
    cr->cap_round();
    cr->set_line_width(1);
    cr->set_color_a(0xFF0000FF);
    cr->stroke();

    // draw points
    for (const auto & l:ret)
      cr->mkpath_points((double)mult*l.second);
    cr->set_line_width(3);
    cr->stroke();

    // draw tolerances
    ret = image_cnt(img, vmin-vtol, vmax, step, closed, 0);
    for (const auto & l:ret)
      cr->mkpath_smline((double)mult*l.second, 0, 0);
    ret = image_cnt(img, vmin+vtol, vmax, step, closed, 0);
    for (const auto & l:ret)
      cr->mkpath_smline((double)mult*l.second, 0, 0);
    cr->cap_round();
    cr->set_line_width(1);
    cr->set_color_a(0xFF888888);
    cr->stroke();

    // line filtering
    auto ret1 = image_cnt(img, vmin, vmax, step, closed, vtol);
    for (const auto & l:ret1)
      cr->mkpath_smline((double)mult*l.second, 0, 0);
    cr->set_line_width(1);
    cr->set_color_a(0xFFFF00FF);
    cr->stroke();

    // draw points
    for (const auto & l:ret1)
      cr->mkpath_points((double)mult*l.second);
    cr->set_line_width(3);
    cr->stroke();

    // find summits
    cr->set_color_a(0xFF000000);
    cr->set_line_width(5);
    auto peaks = image_peaks(img, 20);
    for (const auto & pt:peaks){
      cr->move_to(pt * mult);
      cr->line_to(pt * mult);
      cr->text(type_to_str((int)pt.z).c_str(), pt*mult + iPoint(10,0), 0);
    }
    cr->stroke();

    // save image
    image_save(cr.get_image(), "test2.png");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond