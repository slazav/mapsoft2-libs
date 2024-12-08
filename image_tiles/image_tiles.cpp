#include <sstream>
#include "geo_tiles/quadkey.h"
#include "image/io.h"
#include "image_tiles.h"

std::string
ImageT::make_url(const std::string& tmpl, const iPoint & key){
  std::string ret;
  size_t n0 = 0;
  while (1) {
    auto n1 = tmpl.find('{', n0);
    ret += tmpl.substr(n0,n1-n0);
    if (n1==std::string::npos) break;

    auto n2 = tmpl.find('}', n1+2);
    if (n2==std::string::npos) throw Err()
       << "ImageT: } is missing in URL template: " << tmpl;

    auto s = tmpl.substr(n1+1,n2-n1-1);
    if      (s=="x") ret += type_to_str(key.x);
    else if (s=="y") ret += type_to_str(key.y);
    else if (s=="z") ret += type_to_str(key.z);
    else if (s=="q") ret += tile_to_quadkey(key.x, key.y, key.z);
    else if (s=="{") ret += '{';
    else if (s=="}") ret += '}';
    else if (s.size()>2 && s[0]=='[' && s[s.size()-1]==']'){
      int len=s.size()-2;
      ret+=s[1+abs(key.x+key.y)%len];
    }
    else throw Err() << "ImageT: unknown field " << s 
                     << " in URL template: " << tmpl;
    n0 = n2+1;
  }
  return ret;
}

void
ImageT::prepare_range(const iRect &r){
  int x1 = r.x/tsize;
  int y1 = r.y/tsize;
  int x2 = (r.x+r.w-1)/tsize+1;
  int y2 = (r.y+r.h-1)/tsize+1;
  // same order as in DThreadViewer
  for (int x=x1; x<x2; x++)
    for (int y=y1; y<y2; y++)
      dmanager.add(make_url(tmpl, iPoint(x,y,zoom)));
}

void
ImageT::clear(){
  tiles.clear();
  dmanager.clear();
}

void
ImageT::clear_queue(){
  dmanager.clear_queue();
}

void
ImageT::load_key(const iPoint & key) const {
  if (tiles.contains(key)) return;
  auto url = make_url(tmpl, key);
  try {
    auto s = dmanager.get(url);
    std::istringstream str(s);
    ImageR img = image_load(str, 1);
    if (img.width()!=tsize || img.height()!=tsize){
      std::cerr << "ImageT: wrong image size "
                << img.width() << "x" << img.height()
                << ": " << url << "\n";
      tiles.add(key, ImageR());
    }
    else {
      tiles.add(key, img);
    }
  }
  catch (Err & e){
    // No error messages. Turn on Downloader logging to
    // See OK/Errors
    tiles.add(key, ImageR());
  }
}

ImageR
ImageT::get_image(const iRect & r) const{
  ImageR ret(r.w,r.h, IMAGE_32ARGB);
  for (int y = 0; y<r.h; ++y)
    for (int x = 0; x<r.w; ++x)
      ret.set32(x, swapy?r.h-1-y:y, get_argb(x+r.x,y+r.y));
  return ret;
}

