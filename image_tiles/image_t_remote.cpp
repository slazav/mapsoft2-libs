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
ImageTRemote::read_tile(const iPoint & key) const {
  auto url = make_url(tmpl, key);
  try {
    auto s = dmanager.get(url);
    std::istringstream str(s);
    ImageR img = image_load(str, 1);
    if (img.width()!=tsize || img.height()!=tsize){
      std::cerr << "ImageTRemote: wrong image size "
                << img.width() << "x" << img.height()
                << ": " << url << "\n";
      return ImageR();
    }
    else {
      return img;
    }
  }
  catch (Err & e){
    // No error messages. Turn on Downloader logging to
    // See OK/Errors
  }
  return ImageR();
}

void
ImageTRemote::write_tile(const iPoint & key) const {
  throw Err() << "Write access for remote tiles is not supported";
}
