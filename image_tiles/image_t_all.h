#ifndef IMAGE_TILES_ALL_H
#define IMAGE_TILES_ALL_H

#include <memory>
#include <string>
#include "opt/opt.h"
#include "image_t.h"
#include "image_t_local.h"
#include "image_t_remote.h"
#include "image_t_mbtiles.h"

// Open an arbitrary tiled map, depending on url
std::shared_ptr<ImageT> open_tile_img(const std::string & url, const Opt & opts);

#endif
