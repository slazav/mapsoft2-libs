#ifndef IMAGE_TILES_REMOTE_H
#define IMAGE_TILES_REMOTE_H

#include <string>
#include "downloader/downloader.h"
#include "image_t.h"

// Remote tiled images.

// maximum number of parallel connections for downloading
#define IMAGE_T_NCONN   4

class ImageTRemote: public ImageT {
  mutable Downloader dmanager;

  public:

    ImageTRemote(const std::string & tmpl, bool swapy = false, size_t tsize=256,
                 uint32_t bg=0xFF000000, size_t tcache_size=16, size_t dcache_size=64):
       ImageT(tmpl, swapy, tsize, bg, tcache_size), dmanager(dcache_size, IMAGE_T_NCONN) {};

    /*******************************************************/
    // ImageT interface

    // Set options
    void set_opt(const Opt & opt) override { ImageT::set_opt(opt); dmanager.set_opt(opt); }

    // Clear tile cache and downloader queue
    void clear() override;

    // get a tile (without using cache)
    ImageR tile_read(const iPoint & key) const override;

    // check if tile exists
    bool tile_exists(const iPoint & key) const override;

    // write a tile
    void tile_write(const iPoint & key, const ImageR & img) override;

    // delete a tile
    void tile_delete(const iPoint & key) override;

};

#endif
