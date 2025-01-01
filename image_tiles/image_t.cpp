#include <string>
#include "geo_tiles/quadkey.h"
#include "image_t.h"

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

ImageR &
ImageT::get_tile_cache(const iPoint & key) const {
  if (tile_cache.contains(key)) return tile_cache.get(key);
  tile_cache.add(key, read_tile(key));
  return tile_cache.get(key);
}

uint32_t
ImageT::get_argb(const size_t x, const size_t y) const {
  iPoint key(x/tsize, y/tsize, zoom);
  iPoint crd(x%tsize, y%tsize);
  if (swapy) key.y = (1<<zoom) - key.y - 1;
  auto & img = get_tile_cache(key);
  if (img.is_empty()) return 0;
  return img.get_argb(crd.x, crd.y);
}

ImageR
ImageT::get_image(const iRect & r) const{
  ImageR ret(r.w,r.h, IMAGE_32ARGB);
  for (int y = 0; y<r.h; ++y)
    for (int x = 0; x<r.w; ++x)
      ret.set32(x, swapy?r.h-1-y:y, get_argb(x+r.x,y+r.y));
  return ret;
}

