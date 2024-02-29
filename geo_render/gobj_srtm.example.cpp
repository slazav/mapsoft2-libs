///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "gobj_srtm.h"
#include "err/assert_err.h"
#include "image/io.h"
#include "conv/conv_aff.h"

// a simple example, how to draw 

int
main(){
  try{

    Opt o;
    o.put("srtm_dir", "../srtm/test_srtm");
    o.put("srtm_hmin",   0.0);
    o.put("srtm_hmax", 200.0);
    o.put("srtm_smin",   35.0);
    o.put("srtm_smax", 50.0);
    o.put("srtm_interp_holes", 1);
    o.put("srtm_draw_mode", "shades");
    o.put("srtm_bgcolor", 0);
    o.put("srtm_cnt", 1);
    o.put("srtm_cnt_w", 0.2);
    o.put("srtm_holes",  1);
    o.put("srtm_peaks", 1);

    SRTM s;
    GObjSRTM S(&s, o);

    int x0 = 29;   // coordinates of top-left corner 
    int y0 = 79;   // (datafile for this area N78E029.hgt should be in srtm_dir)
    int w = 1200;  // image size
    int h = 1200;
    double k = 4; // scale

    CairoWrapper cr;

    ImageR img(w,h,IMAGE_32ARGB);
    img.fill32(0);
    cr.set_surface_img(img);

    // build a simple conversion. It's flipped upside down.
    std::shared_ptr<ConvAff2D> cnv(new ConvAff2D);
    cnv->shift_dst(dPoint(x0,y0));
    cnv->set_scale_src(dPoint(1/k/1200.0, -1/k/1200.0));
    S.set_cnv(cnv);

    S.draw(cr, dRect(0,0,w,h));
    image_save(img, "srtm1.png", o);

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
