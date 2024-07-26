#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include "getopt/getopt.h"
#include "getopt/help_printer.h"
#include "rainbow/rainbow.h"
#include "err/err.h"
#include "opt/opt.h"
#include "geo_data/geo_io.h"
#include "geom/point_int.h"

#include "srtm.h"
#include "image/image_r.h"
#include "image/io.h"

// Extract SRTM data for some region.

int
main(int argc, char *argv[]){
  try{

    Opt o;
    o.put("srtm_dir", "/mnt/disk/DEM/ALOS2024");
    o.put("srtm_use_overlay", 1);
//    dRect rng(72.7,39.3,0.5,0.3);
    dRect rng(145.9,64.95,0.2,0.1);

    SRTM S(o);
    Rainbow R(0,3000, RAINBOW_NORMAL);

    dPoint tlc, step;
    auto imgh = S.get_img(rng, tlc, step);
    if (imgh.is_empty()) throw Err() << "no data";
    size_t w = imgh.width(), h=imgh.height();

    ImageR imgc(w, h, IMAGE_24RGB);

    for (size_t x = 0; x<w; x++){
      for (size_t y = 0; y<h; y++){
        int16_t v = imgh.get16(x,y);

        dPoint d;
        double v1,v2,v3,v4;
        v1=v2=v3=v4=v;
        if (x>0)   {v1 = imgh.get16(x-1,y); d.x+=1;}
        if (x<w-1) {v2 = imgh.get16(x+1,y); d.x+=1;}
        if (y>0)   {v3 = imgh.get16(x,y-1); d.y+=1;}
        if (y<h-1) {v4 = imgh.get16(x,y+1); d.y+=1;}

        double lat = tlc.y + 1.0 - y/h;
        d *= step * 6380e3 * M_PI/180;
        d.x /= cos(M_PI*lat/180.0);
        double U = 0;

        if (d.x!=0 && d.y!=0) U = hypot((v2-v1)/d.x, (v4-v3)/d.y);
        U = atan(U)*180.0/M_PI;

        imgc.set24(x,y, color_shade(R.get(v), 1-U/90.0));
      }
    }

    image_save(imgh , "test_dem.tif");
    image_save(imgc , "test_dem.jpg");

  }
  catch (Err & e) {
    if (e.str()!="")
      std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond