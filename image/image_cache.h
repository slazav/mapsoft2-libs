#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include <string>
#include "cache/cache.h"
#include "geom/line.h"
#include "io.h"
#include "image_r.h"
#include "image_colors.h"

// class for image cache
class ImageRCache {
  Cache<std::string, std::pair<int, ImageR> > cache;

public:
  // constructor: make cache with maxnum capacity.
  ImageRCache(const int maxnum) : cache(maxnum){}

  // Load image with scale sc or use already loaded image.
  ImageR get(const std::string & fn,
             const int sc = 1){

    if (cache.contains(fn)){
      auto ip = cache.get(fn);
      if (ip.first == sc) return ip.second;
    }

    ImageR img = image_load(fn, sc);
    cache.add(fn, std::make_pair(sc, img));
    return img;
  }

  // Delete image from the cache
  void del(const std::string & fn){
    if (cache.contains(fn)) cache.erase(fn);
  }


};

#endif
