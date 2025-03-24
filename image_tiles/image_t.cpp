#include <string>
#include <sstream>
#include "geo_tiles/quadkey.h"
#include "image_t.h"

// move to geo_tiles module or somewhere else?
std::string
tile_bbox_str(int x, int y, int z){
  int tsize = 256;
  double ires  = 2.0 * M_PI * 6378137.0 / tsize;
  double shift = 2.0 * M_PI * 6378137.0 / 2.0;
  y = ((1<<z) - 1) - y;
  dPoint t(x,y);
  // same as GeoTiles::px_to_m:
  dPoint p1 = t*tsize*ires/(1<<z) - dPoint(shift,shift);
  dPoint p2 = (t+iPoint(1,1))*tsize*ires/(1<<z) - dPoint(shift,shift);
  std::ostringstream ss;
  ss << std::fixed
     << p1.x << "," << p1.y << "," << p2.x << "," << p2.y;
  return ss.str();
}

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
    else if (s=="b") ret += tile_bbox_str(key.x, key.y, key.z);
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
ImageT::tile_get_cached(const iPoint & key) const {
  if (tile_cache.contains(key)) return tile_cache.get(key);
  tile_cache.add(key, tile_read(key));
  return tile_cache.get(key);
}

uint32_t
ImageT::get_argb(const size_t x, const size_t y) const {
  iPoint key(x/tsize, y/tsize, zoom);
  iPoint crd(x%tsize, y%tsize);
  if (swapy) key.y = (1<<zoom) - key.y - 1;
  auto & img = tile_get_cached(key);
  if (img.is_empty()) return 0;
  return img.get_argb(crd.x, crd.y);
}

ImageR
ImageT::get_image(const iRect & r) const{
  ImageR ret(r.w,r.h, IMAGE_32ARGB);
  for (int y = 0; y<r.h; ++y)
    for (int x = 0; x<r.w; ++x)
      ret.set32(x, y, get_argb(x+r.x,y+r.y));
  return ret;
}

iRect
ImageT::tile_bbox(const iPoint & key) const{
  return (int)tsize*iRect(key, key+iPoint(1,1));
}

void
ImageT::tile_rescale(const iPoint & key) {
  ImageR img(tsize, tsize, IMAGE_32ARGB);
  img.fill32(0);

  for (int t=0; t<4; t++){
    iPoint src(2*key.x + t%2, 2*key.y + t/2, key.z+1);
    if (!tile_exists(src)) continue;
    ImageR src_img = tile_read(src);

    for (size_t y1 = 0; y1<tsize/2; ++y1){
      for (size_t x1 = 0; x1<tsize/2; ++x1){
        // calculate 4-point average for all 4 color components
        int cc[4] = {0,0,0,0};
        for (int t1 = 0; t1<4; t1++){
          uint32_t c =  src_img.get_argb(2*x1 + t1%2, 2*y1 + t1/2);
          for (int i = 0; i<4; ++i) cc[i] += ((c>>(8*i)) & 0xff);
        }
        uint32_t c = 0;
        for (int i = 0; i<4; ++i)
          c += ((cc[i]/4) & 0xff) << (8*i);
        int dy = swapy? 1-t/2: t/2;
        img.set32(x1 + (t%2)*tsize/2, y1 + dy*tsize/2, c);
      }
    }
  }
  tile_write(key, img);
}
