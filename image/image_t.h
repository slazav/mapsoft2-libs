#ifndef IMAGE_T_H
#define IMAGE_T_H

#include <sstream>
#include "downloader/downloader.h"
#include "cache/cache.h"
#include "image.h"
#include "image_r.h"
#include "io.h"

/*
Read only access to a tiled image.

Usage:

We want to draw map on the screen. We know coordinate range, but do not
know which points in which order we will use (coordinate conversion can
be involved).

For each coordinate (x,y) we can calculate tile (x,y)/tsize and
coordinate on the tile (x,y)%tsize. Url template with `{x}` and `{y}`
fields is used to download a tile. `{{}` and `{}}` combinations can be
used in the template to insert `{` and `}` literals.

We request background downloading of the coordinate range
(`prepare_range` method). Packed data is stored in the Downloader
object. `Downloader::update_clean_list` is done in `prepare_range` to
remove URLs which we do not need anymore.

Then a point is read by `get_color` method, the image is unpacked
removed from Downloader cache and put into tile cache (this cache has fixed
capacity, 16 tiles).

*/

// size if tile cache (unpacked images)
#define IMAGE_T_CACHE_SIZE 16

// maximum number of parallel connections for downloading
#define DOWNLOAD_NCONN   4

class ImageT: public Image {
  size_t tsize;
  bool swapy;
  Cache<iPoint, ImageR> tiles;
  Downloader dmanager;
  std::string tmpl;

  public:
    ImageT(const std::string & tmpl, bool swapy = false, size_t tsize=256):
       tmpl(tmpl), tsize(tsize), swapy(swapy), tiles(IMAGE_T_CACHE_SIZE),
       dmanager(DOWNLOAD_NCONN) {};

    // Make url from a template - replace {x} by key.x, {y} by key.y
    std::string make_url(const iPoint & key){
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
        else if (s=="{") ret += '{';
        else if (s=="}") ret += '}';
        else throw Err() << "ImageT: unknown field " << s 
                         << " in URL template: " << tmpl;
        n0 = n2+1;
      }
      return ret;
    }

    // Start downloading all tiles in the range,
    // remove tiles outside this range
    void prepare_range(const iRect &r){
      int x1 = r.x/tsize;
      int y1 = r.y/tsize;
      int x2 = (r.x+r.w)/tsize+1;
      int y2 = (r.y+r.h)/tsize+1;
      for (int y=y1; y<y2; y++)
        for (int x=x1; x<x2; x++)
          dmanager.add(make_url(iPoint(x,y)));
      dmanager.update_clean_list();
    }

    // get point color
    uint32_t get_color(const int x, const int y) override {
      iPoint key(x/tsize, y/tsize);
      iPoint crd(x%tsize, y%tsize);
      if (swapy) crd.y = tsize - crd.y - 1;
      if (!tiles.contains(key)){
        auto url = make_url(key);
        auto s = dmanager.get(url);
        std::istringstream str(s);
        ImageR img = image_load(str, 1);
        if (img.width()!=tsize || img.height()!=tsize)
          throw Err() << "ImageT: wrong image size "
                      << img.width() << "x" << img.height()
                      << ": " << url << "\n";
        tiles.add(key, img);
        dmanager.del(url);
      }
      return tiles.get(key).get_argb(crd.x, crd.y);
    }

    // get an image
    ImageR get_image(const iRect & r){
      ImageR ret(r.w,r.h, IMAGE_32ARGB);
      for (int y = 0; y<r.h; ++y)
        for (int x = 0; x<r.w; ++x)
          ret.set32(x, swapy?r.h-1-y:y, get_color(x+r.x,y+r.y));
      return ret;
    }

};


#endif
