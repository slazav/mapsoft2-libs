///\cond HIDDEN (do not show this in Doxyden)

#include <cmath>
#include <cstdlib> // rand
#include "image_cnt.h"
#include "image/io.h"
#include "cairo/cairo_wrapper.h"
#include "rainbow/rainbow.h"

int
main(){
  try{

    int w=64, h=32;  // grid size
    int mult=10;     // image size will be bigger by this factor

    double amp = 1.0;   // amplitude of the function
    double namp = 0.05; // noise amplitude

    double step = 0.2; // countour step
    double vmin = NAN; // countour start value
    double vmax = NAN; // countour end value
//    double vmin = 0.3;
//    double vmax = 0.3;

    double vtol = 0.03; // tolerance for contour filtering
    bool closed = 1;    // produce closed/open contours

    ImageR img(w,h, IMAGE_DOUBLE);
    ImageR cimg(w*mult,h*mult, IMAGE_32ARGB);
    Rainbow R(-1, +1);

    for (size_t y=0; y<img.height(); y++){
      for (size_t x=0; x<img.width(); x++){
        double v = amp*cos(M_2_PI*x/32.0) * sin(M_2_PI*y/32.0)
                 + namp*(double)rand()/RAND_MAX;
        img.setD(x,y, v);
        for (size_t dx=0; dx<mult; dx++){
          for (size_t dy=0; dy<mult; dy++){
            cimg.set32(mult*x+dx, mult*y+dy, R.get(v));
          }
        }
      }
    }


    CairoWrapper cr;
    cr.set_surface_img(cimg);


    // find and draw contours
    auto ret = image_cnt(img, vmin, vmax, step, closed);
    for (const auto & l:ret) cr->mkpath_smline((double)mult*l.second, 0, 0);
    cr->cap_round();
    cr->set_line_width(1);
    cr->set_color_a(0xFF000000);
    cr->stroke();

    // draw points
    for (const auto & l:ret) cr->mkpath_points((double)mult*l.second);
    cr->set_line_width(3);
    cr->set_color_a(0xFF0000FF);
    cr->stroke();

    // test line filtering
    image_cnt_vtol_filter(img, ret, vtol);
    for (const auto & l:ret){
      cr->mkpath_smline((double)mult*l.second, 0, 0);
    }
    cr->set_line_width(1);
    cr->set_color_a(0xFFFF00FF);
    cr->stroke();

    // save image
    image_save(cr.get_image(), "image.png");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond