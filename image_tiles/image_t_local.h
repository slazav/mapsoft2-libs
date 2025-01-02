#ifndef IMAGE_TILES_LOCAL_H
#define IMAGE_TILES_LOCAL_H

#include <set>
#include <string>
#include "image_t.h"
#include "opt/opt.h"

// Local tiled images.

class ImageTLocal: public ImageT {
  Opt O;
  std::set<std::string> dirs; // set of existing directories (used in write_tile())

  public:

    ImageTLocal(const std::string & tmpl, bool swapy = false, size_t tsize=256,
                 uint32_t bg=0xFF000000, size_t tcache_size=16):
       ImageT(tmpl, swapy, tsize, bg, tcache_size) {};

    /*******************************************************/
    // ImageT interface

    // Set options for reading files ("img_in_fmt")
    void set_opt(const Opt & opt) override { ImageT::set_opt(opt); O = opt;}

    // get a tile (without using cache)
    ImageR tile_read(const iPoint & key) const override;

    // check if tile exists
    bool tile_exists(const iPoint & key) const override;

    // check if tile1 newer then tile2 OR tile2 does not exist
    bool tile_newer(const iPoint & key1, const iPoint & key2) const override;

    // write a tile
    void tile_write(const iPoint & key, const ImageR & img) override;

    // delete a tile
    void tile_delete(const iPoint & key) override;

};

#endif
