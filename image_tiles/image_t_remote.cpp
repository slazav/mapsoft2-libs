#include <sstream>
#include "geo_tiles/quadkey.h"
#include "image/io.h"
#include "image_t_remote.h"

void
ImageTRemote::clear(){
  ImageT::clear();
  dmanager.clear();
}

ImageR
ImageTRemote::tile_read(const iPoint & key) const {
  auto url = make_url(tmpl, key);
  if (verb) std::cout << "tile_read: " << url << ": ";
  try {
    auto s = dmanager.get(url);
    std::istringstream str(s);
    ImageR img = image_load(str, 1);
    if (img.width()==tsize && img.height()==tsize){
      if (verb) std::cout << "OK\n";
      return img;
    }
    if (verb) std::cout << "BAD SIZE\n";
  }
  catch (Err & e){
    // One can also turn on Downloader logging
    if (verb) std::cout << "FAIL: " << e.str() << "\n";
  }
  return ImageR();
}

bool
ImageTRemote::tile_exists(const iPoint & key) const {
  throw Err() << "tile_exists for remote tiles is not supported";
}

bool
ImageTRemote::tile_newer(const iPoint & key1, const iPoint & key2) const {
  throw Err() << "tile_newer for remote tiles is not supported";
}

void
ImageTRemote::tile_write(const iPoint & key, const ImageR & img) {
  throw Err() << "Write access for remote tiles is not supported";
}
