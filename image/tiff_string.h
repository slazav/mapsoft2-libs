#ifndef TIFF_STREAM_H
#define TIFF_STREAM_H

#include <string>
#include <tiffio.h>

// Open libtiff handler for reading from std::string

// Example of reading and writing TIFF from/to memory can be found in
// https://stackoverflow.com/questions/4624144/c-libtiff-read-and-save-file-from-and-to-memory

struct TiffStringData{
  const std::string &str;
  size_t pos;
  size_t size() {return str.size();}
  const char * data() {return str.data();}
  TiffStringData(const std::string & s):pos(0),str(s){}
};


extern "C" {

static tsize_t
TiffStrReadProc(thandle_t handle, tdata_t buf, tsize_t size){
  auto str = (TiffStringData*) handle;
  tsize_t n;
  if (str->pos + size <= str->size()) n = size;
  else n = str->size() - str->pos;
  memcpy(buf, str->data() + str->pos, n);
  str->pos += n;
  return n;
}

static tsize_t
TiffStrWriteProc(thandle_t handle, tdata_t buf, tsize_t size){
  return 0;
}

static toff_t
TiffStrSeekProc(thandle_t handle, toff_t off, int whence){
  auto str = (TiffStringData*) handle;
  long int pos;
  switch (whence) {
    case SEEK_SET:
      str->pos = off;
      break;
    case SEEK_CUR:
      pos = str->pos + off;
      break;
    case SEEK_END:
      pos = str->size() + off;
      break;
    default:
      return -1;
  }
  if (pos > str->size() || pos < 0) return -1;
  str->pos = pos;
  return -1;
}

static int
TiffStrCloseProc(thandle_t handle){
  return 0;
}

static toff_t TiffStrSizeProc(thandle_t handle){
  auto str = (TiffStringData*) handle;
  return str->size();
}

static int
TiffStrMapProc(thandle_t handle, tdata_t* base, toff_t* psize){
  auto str = (TiffStringData*) handle;
  *base = (char *)str->data();
  *psize = str->size();
  return 1;
}

static void
TiffStrUnmapProc(thandle_t handle, tdata_t base, toff_t size){
  return;
}

} // extern("C")

TIFF* TIFFStringOpen(const TiffStringData & str){
  return TIFFClientOpen("TiffString", "rb", (thandle_t) &str,
        TiffStrReadProc, TiffStrWriteProc,
        TiffStrSeekProc, TiffStrCloseProc, TiffStrSizeProc,
        TiffStrMapProc,  TiffStrUnmapProc
    );
}

#endif
