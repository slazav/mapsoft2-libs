#include "image_t_all.h"
#include "filename/filename.h"

std::shared_ptr<ImageT>
open_tile_img(const std::string & url, const Opt & opts){

  int  tsize    = opts.get("tsize",      256);
  bool swapy    = opts.get("tmap_swapy", false);
  uint32_t bg   = opts.get("bgcolor",    0);
  int tcache_s  = opts.get("tmap_cache_size", 16);
  int dcache_s  = opts.get("tmap_dcache_size", 64);
  bool rdonly   = opts.get("tmap_readonly", false);
  int db_sync   = opts.get("db_sync", 0);

  // If url contains schema, use remote
  if (url.find("://")!=url.npos){
    std::shared_ptr<ImageT> ret(
      new ImageTRemote(url, swapy, tsize, bg, tcache_s, dcache_s));
    ret->set_opt(opts);
    return ret;
  }

  // If extension is mbtiles
  if (file_ext_check(url, ".mbtiles")){
    std::shared_ptr<ImageT> ret(
      new ImageMBTiles(url, rdonly, db_sync));
    ret->set_opt(opts);
    return ret;
  }

  std::shared_ptr<ImageT> ret(
    new ImageTLocal(url, swapy, tsize, bg, tcache_s));
  ret->set_opt(opts);
  return ret;
}
