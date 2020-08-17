///\cond HIDDEN (do not show this in Doxyden)

#include <iostream>
#include "image_srtm.h"
#include "err/assert_err.h"
#include "io.h"

int
main(){
  try{

    Opt o;
    o.put("srtm_dir", "./test_srtm");
    o.put("srtm_hmin",   0.0);
    o.put("srtm_hmax", 100.0);
    o.put("srtm_smin",   0.0);
    o.put("srtm_smax", 30.0);
    ImageSRTM S(o);

    int x0 = 29*1200;
    int y0 = 78*1200;

    assert_eq(S.get_val(x0 + 1300, y0, false), SRTM_VAL_NOFILE);
    assert_eq(S.get_val(-10, -20, false), SRTM_VAL_NOFILE);
    assert_eq(S.get_val(x0, y0-1, false), SRTM_VAL_NOFILE);

    assert_eq(S.get_val(x0+10, y0+10, false), 0);
    assert_eq(S.get_val(x0+700, y0+1100, false), 20);

    // wgs coordinates
    assert_eq(S.get_val(dPoint(x0+10, y0+10)/1200.0, false), 0);
    assert_eq(S.get_val(dPoint(x0+700, y0+1100)/1200.0, false), 20);

    // colors
    assert_eq(S.get_color_fast(x0+10, y0+10), 0xff0000ff);
    assert_eq(S.get_color_fast(x0+700, y0+1100), 0xff00f5f5);

    // set value
    assert_eq(S.set_val(x0, y0-1, 100), SRTM_VAL_NOFILE);
    assert_eq(S.set_val(x0, y0, 100), 100);
    assert_eq(S.get_val(x0, y0, false), 100);


    /****************/
    if (0) { // make image
      int x0 = 78*1200;
      int y0 = 41*1200;
      o.put("srtm_hmin",   0.0);
      o.put("srtm_hmax", 5000.0);
      S.set_opt(o);

      ImageR im(1200,1200, IMAGE_32ARGB);
      for (int y=0; y<1200; y++){
        for (int x=0; x<1200; x++){
          uint32_t c = S.R.get(S.get_val(x+x0,y+y0,true));
          double s = S.get_slope(x+x0, y+y0, true);
          c = color_shade(c, 1-s/90.0);
          im.set32(x,1200-1-y, c);
        }
      }
      image_save(im, "test_srtm/img.png");
    }


    /****************/
    // wrong (too large) width
    o.put("srtm_width", 1202); 
    S.set_opt(o);
    assert_err(S.get_val(x0+700, y0+1100, false),
      "ImageSRTM: bad .hgt.gz file: ./test_srtm/N78E029.hgt.gz");


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond