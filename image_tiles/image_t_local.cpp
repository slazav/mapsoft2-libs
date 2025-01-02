#include "image/io.h"
#include "image_t_local.h"
#include "filename/filename.h"

ImageR
ImageTLocal::tile_read(const iPoint & key) const {
  auto fname = make_url(tmpl, key);
  if (verb) std::cout << "tile_read: " << fname << ": ";
  try{
    auto img = image_load(fname, 1, O);
    if (img.height()==tsize || img.width()==tsize){
      if (verb) std::cout << "OK\n";
      return img;
    }
    if (verb) std::cout << "BAD SIZE\n";
  }
  catch (Err & e){
    if (verb) std::cout << "FAIL: " << e.str() << "\n";
  }
  return ImageR();
}

bool
ImageTLocal::tile_exists(const iPoint & key) const {
  return file_exists(make_url(tmpl, key));
}

time_t
ImageTLocal::tile_mtime(const iPoint & key) const {
  return file_mtime(make_url(tmpl, key));
}

bool
ImageTLocal::tile_newer(const iPoint & key1, const iPoint & key2) const {
  return file_newer(make_url(tmpl, key1), make_url(tmpl, key2));
}

void
ImageTLocal::tile_write(const iPoint & key, const ImageR & img) {
  // make filename
  auto fname = make_url(tmpl, key);
  if (verb) std::cout << "tile_write: " << fname << "\n";

  // create subdirectories if needed
  for (const auto & d: file_get_dirs(fname, 1)) {
    if (dirs.count(d)>0) continue;
    file_mkdir(d);
    dirs.insert(d);
  }
  // save image
  image_save(img, fname, O);
}

void
ImageTLocal::tile_delete(const iPoint & key) {
  auto fname = make_url(tmpl, key);
  if (verb) std::cout << "tile_delete: " << fname << "\n";
  file_remove(fname);
}
