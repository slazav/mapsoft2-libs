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
    bool verb; // if true - print messages to stdout when reading/writing tiles

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
       Image(bg), tile_cache(cache_size), zoom(0), verb(false) {};

    // Set options
    virtual void set_opt(const Opt & opt) { verb = opt.get("verbose", false); }

    // Clear all cached data
    virtual void clear() {tile_cache.clear();}

    /*******************************************************/

    // get a tile directly (without using cache)
    virtual ImageR tile_read(const iPoint & key) const = 0;

    // write a tile
    virtual void tile_write(const iPoint & key, const ImageR & img) = 0;

    // delete a tile
    virtual void tile_delete(const iPoint & key) = 0;

    // check if tile exists
    virtual bool tile_exists(const iPoint & key) const = 0;

    // check if tile1 newer then tile2 OR tile2 does not exist
    virtual bool tile_newer(const iPoint & key1, const iPoint & key2) const = 0;

    // return true if any of z+1 sub-tiles is newer or tile does not exist
    // (then one may want to update the tile by rescaling z+1 sub-tiles)
    virtual bool tile_rescale_check(const iPoint & key) const;

    // make tile by scaling four z+1 sub-tiles and write it
    virtual void tile_rescale(const iPoint & key);

    // get a tile (using tile cache)
    virtual ImageR & tile_get_cached(const iPoint & key) const;

    // tile pixel range
    virtual iRect tile_bbox(const iPoint & key) const;

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
