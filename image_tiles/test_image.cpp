///\cond HIDDEN (do not show this in Doxyden)

#include "image_t_all.h"
#include "err/assert_err.h"
#include "geo_tiles/geo_tiles.h"
#include "image/io.h"

// Test Image interface, get an image from different sources

void
make_image(const iRect & box, const int zoom, ImageT & timg, const std::string & fname){
  Opt o("{\"verbose\": \"1\"}");
  timg.set_opt(o);
  timg.set_zoom(zoom);
  image_save(timg.get_image(box), fname);
}


int
main(){
  try{

    int z = 11;
//    dPoint p1(72.55234, 38.10701);
//    dPoint p2(73.25340, 38.44875);

    dPoint p1(72, 38.0);
    dPoint p2(73, 38.4);

    // get pixel range for a given coordinates
    // (pixels increasing from south to north)
    GeoTiles tcalc;
    dPoint p1p = tcalc.m_to_px(tcalc.ll_to_m(p1), z);
    dPoint p2p = tcalc.m_to_px(tcalc.ll_to_m(p2), z);

    //pixels from north to south
    p1p.y = (1<<z)*256 - p1p.y;
    p2p.y = (1<<z)*256 - p2p.y;
    iRect box(p1p, p2p);
    std::cout << "pixel range: " << box << "\n";

    ImageTRemote I1("https://{[abc]}.tile.openstreetmap.org/{z}/{x}/{y}.png", false);
    make_image(box, z, I1, "test1.jpg");

    ImageTRemote I2("https://tiles.slazav.xyz/hr/{z}/{x}/{y}.png", false);
    make_image(box, z, I2, "test2.jpg");

    ImageTRemote I3("https://tiles.nakarte.me/topo1000/{z}/{x}/{y}", true);
    make_image(box, z, I3, "test3.jpg");

//    ImageTLocal I4("/home/sla/MYMAPS/map_hr/TILES/{x}-{y}-{z}.png", false);
//    make_image(box, z, I4, "test4.jpg");

//    ImageMBTiles I5("/home/sla/MYMAPS/map_hr/OUT/slazav_hr.mbtiles", true);
//    make_image(box, z, I5, "test5.jpg");

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond