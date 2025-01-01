#ifndef IMAGE_TILES_H
#define IMAGE_TILES_H

#include <string>
#include "cache/cache.h"
#include "image/image.h"
#include "image/image_r.h"

// Base interface for tiled images.

class ImageT: public Image {
  protected:
    std::string tmpl;
    size_t tsize;
    bool swapy;
    int zoom;
    mutable Cache<iPoint, ImageR> tile_cache;

  public:

    /*******************************************************/
    // Make url from a template.
    // Url template format:
    //  `{x}` and `{y}` - replaced by x and y values,
    //  `{z}` - replaced by z value,
    //  `{q}` - replaced by quadkey(x,y,z),
    //  `{[abc]}` - replaced by one letter from [...] set (depenting on x+y value),
    //  `{{}` and `{}}` - replaced by `{` and `}` literals.

    static std::string make_url(const std::string& tmpl, const iPoint & key);

    /*******************************************************/

    ImageT(const std::string & tmpl, bool swapy = false, size_t tsize=256,
           uint32_t bg=0xFF000000, size_t cache_size=16):
       tmpl(tmpl), swapy(swapy), tsize(tsize),
       Image(bg), tile_cache(cache_size), zoom(0) {};

    // Set options
    virtual void set_opt(const Opt & opt) {}

    // Clear all cached data
    virtual void clear() {tile_cache.clear();}

    // get a tile directly (without using cache)
    virtual ImageR read_tile(const iPoint & key) const = 0;

    // write a tile
    virtual void write_tile(const iPoint & key) const = 0;

    // get a tile (using tile cache)
    virtual ImageR & get_tile_cache(const iPoint & key) const;


    /*******************************************************/
    // Image interface

    // set zoom level (needed for Image interface)
    void set_zoom(int z) {zoom = z;}

    // get zoom level
    int get_zoom() const {return zoom;}

    // get point color
    uint32_t get_argb(const size_t x, const size_t y) const override;

    // get an image (move somewhere else?)
    ImageR get_image(const iRect & r) const;

    std::ostream & print (std::ostream & s) const override{
      s << "ImageT(" << tmpl << ")";
      return s;
    }
};

#endif
