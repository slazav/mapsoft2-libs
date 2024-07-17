#ifndef IMAGE_T_H
#define IMAGE_T_H

#include <sstream>
#include "downloader/downloader.h"
#include "cache/cache.h"
#include "geo_tiles/quadkey.h"
#include "image.h"
#include "image_r.h"
#include "io.h"

/*
Read only access to a tiled image.

Usage:

We want to draw map on the screen. We know coordinate range, but do not
know which points in which order we will use (coordinate conversion can
be involved).

For each coordinate (x,y) and zoom level z we can calculate
tile key (x/tsize,y/tsize,z) and coordinate on the tile (x,y)%tsize.
Url template format:
 `{x}` and `{y}` - replaced by x and y values,
 `{z}` - replaced by z value,
 `{q}` - replaced by quadkey(x,y,z),
 `{[abc]}` - replaced by one letter from [...] set (depenting on x+y value),
 `{{}` and `{}}` - replaced by `{` and `}` literals.

We request background downloading of the coordinate range
(`prepare_range` method). Packed data is stored in the Downloader
object. `Downloader::update_clean_list` is done in `prepare_range` to
remove URLs which we do not need anymore.

Then a point is read by `get_rgb` method, the image is unpacked
removed from Downloader cache and put into tile cache (this cache has fixed
capacity, 16 tiles).

*/

// size of data cache (packed images)
#define IMAGE_T_DCACHE_SIZE 64

// size of tile cache (unpacked images)
#define IMAGE_T_TCACHE_SIZE 16

// maximum number of parallel connections for downloading
#define IMAGE_T_NCONN   4

class ImageT: public Image {
  std::string tmpl;
  size_t tsize;
  bool swapy;
  int zoom;
  mutable Cache<iPoint, ImageR> tiles;
  mutable Downloader dmanager;

  public:
    ImageT(const std::string & tmpl, bool swapy = false, size_t tsize=256):
       tmpl(tmpl), tsize(tsize), swapy(swapy), tiles(IMAGE_T_TCACHE_SIZE), zoom(0),
       dmanager(IMAGE_T_DCACHE_SIZE, IMAGE_T_NCONN) {};

    // Set options, now it's only Downloader options
    void set_opt(const Opt & opt) {dmanager.set_opt(opt);}

    // Make url from a template - replace {x} by key.x, {y} by key.y, {z} by 
    static std::string make_url(const std::string& tmpl, const iPoint & key);

    // set zoom level
    void set_zoom(int z) {zoom = z;}

    // get zoom level
    int get_zoom() const {return zoom;}


    // Start downloading all tiles in the range.
    void prepare_range(const iRect &r);

    // Clear all data, including downloader's queue
    void clear();

    // Clear downloader's queue
    void clear_queue();

    // load tile to the cache
    void load_key(const iPoint & key) const;

    // get point color
    uint32_t get_argb(const size_t x, const size_t y) const override {
      iPoint key(x/tsize, y/tsize, zoom);
      iPoint crd(x%tsize, y%tsize);
      if (swapy) key.y = (1<<zoom) - key.y - 1;
      if (!tiles.contains(key)) load_key(key);
      auto img = tiles.get(key);
      if (img.is_empty()) return 0;
      return img.get_argb(crd.x, crd.y);
    }

    // get an image
    ImageR get_image(const iRect & r) const;

    std::ostream & print (std::ostream & s) const override{
      s << "ImageT(" << tmpl << ")";
      return s;
    }
};

#endif
