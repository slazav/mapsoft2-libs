#include <tiffio.h>
#include <cstring>
#include <string>
#include <fstream>
#include "image_tiff.h"
#include "image_colors.h"

/**********************************************************/
// custom error handler
#include <cstdarg>
void
my_error_exit (const char* module, const char* fmt, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, fmt, args);
  throw Err() << module << " error: " << buf;
}
/**********************************************************/
// Open libtiff handler for reading from std::istream

extern "C" {

static tsize_t
TiffStrReadProc(thandle_t handle, tdata_t buf, tsize_t size){
  auto str = (std::istream*) handle;
  str->read((char *)buf, size);
  return str->gcount();
}

static tsize_t
TiffStrWriteProc(thandle_t handle, tdata_t buf, tsize_t size){
  auto str = (std::ostream*) handle;
  auto pos0 = str->tellp();
  str->write((char *)buf, size);
  return str->tellp() - pos0;
}

static toff_t
TiffStrRSeekProc(thandle_t handle, toff_t off, int whence){
  auto str = (std::istream*) handle;
  switch (whence) {
    case SEEK_SET: str->seekg(off, std::ios_base::beg); break;
    case SEEK_CUR: str->seekg(off, std::ios_base::cur); break;
    case SEEK_END: str->seekg(off, std::ios_base::end); break;
    default: return -1;
  }
  return str->tellg();
}

static toff_t
TiffStrWSeekProc(thandle_t handle, toff_t off, int whence){
  auto str = (std::ostream*) handle;
  switch (whence) {
    case SEEK_SET: str->seekp(off, std::ios_base::beg); break;
    case SEEK_CUR: str->seekp(off, std::ios_base::cur); break;
    case SEEK_END: str->seekp(off, std::ios_base::end); break;
    default: return -1;
  }
  return str->tellp();
}

static int
TiffStrCloseProc(thandle_t handle){
  return 0;
}

static toff_t
TiffStrRSizeProc(thandle_t handle){
  auto str = (std::istream*) handle;
  auto pos0 = str->tellg();
  str->seekg(std::ios_base::end);
  auto pos1 = str->tellg();
  str->seekg(pos0, std::ios_base::beg);
  return pos1;
}

static toff_t
TiffStrWSizeProc(thandle_t handle){
  auto str = (std::ostream*) handle;
  auto pos0 = str->tellp();
  str->seekp(std::ios_base::end);
  auto pos1 = str->tellp();
  str->seekp(pos0, std::ios_base::beg);
  return pos1;
}

} // extern("C")

TIFF* TIFFStreamROpen(std::istream & str){
  return TIFFClientOpen("TIFF", "rb", (thandle_t) &str,
        TiffStrReadProc, TiffStrWriteProc,
        TiffStrRSeekProc, TiffStrCloseProc, TiffStrRSizeProc,
        NULL, NULL
    );
}

TIFF* TIFFStreamWOpen(std::ostream & str){
  return TIFFClientOpen("TIFF", "wb", (thandle_t) &str,
        TiffStrReadProc, TiffStrWriteProc,
        TiffStrWSeekProc, TiffStrCloseProc, TiffStrWSizeProc,
        NULL, NULL
    );
}

/**********************************************************/
// getting file dimensions from stream
iPoint image_size_tiff(std::istream & str){
  TIFF* tif = NULL;
  uint32_t w, h;

  try {
    TIFFSetErrorHandler((TIFFErrorHandler)&my_error_exit);

    tif = TIFFStreamROpen(str);
    if (!tif) throw Err() << "image_size_tiff: can't open TIFF";

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

    throw Err();
  }
  catch (Err e){
    if (tif) TIFFClose(tif);
    if (e.str()!="") throw e;
  }
  return iPoint(w,h);
}

/**********************************************************/

Image
image_load_tiff(std::istream & str, const double scale){

  if (scale < 1) throw Err() << "image_load_tiff: wrong scale: " << scale;

  uint8_t *cbuf = NULL;
  TIFF* tif = NULL;
  Image img;

  try {

    TIFFSetErrorHandler((TIFFErrorHandler)&my_error_exit);
    tif = TIFFStreamROpen(str);
    if (!tif) throw Err() << "image_load_tiff: can't load tiff";

    // image dimensions
    uint32_t w, h;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

    // scaled image size
    int w1 = floor((w-1)/scale+1);
    int h1 = floor((h-1)/scale+1);

    // can we do random access to lines?
    int compression_type, rows_per_strip;
    TIFFGetField(tif, TIFFTAG_COMPRESSION,  &compression_type);
    TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip);
    bool can_skip_lines =
      (compression_type==COMPRESSION_NONE)||(rows_per_strip==1);

    // samples per pixel, bits per sample, photometric type
    int samples=0, bps=0;
    uint16_t photometric = 0;
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE,   &bps);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
    //std::cerr << "TIFF TYPE: " << photometric << " "
    //          << samples << "x" << bps << "\n";

    // create image (type depends on TIFF type)
    switch (photometric){

      case PHOTOMETRIC_PALETTE:
        if (samples == 1 && bps == 8) {
          img = Image(w1,h1, IMAGE_8PAL);
          uint16 *cmap[3];
          TIFFGetField(tif, TIFFTAG_COLORMAP, cmap, cmap+1, cmap+2);
          for (int i=0; i<256; i++){
            img.cmap.push_back(
              (((uint32)cmap[0][i] & 0xFF00)<<8) +
               ((uint32)cmap[1][i] & 0xFF00) +
              (((uint32)cmap[2][i] & 0xFF00)>>8) +
              0xFF000000);
          }
          break;
        }
        throw Err() << "image_load_tiff: unsupported format: "
                    << "PALETTE " << samples << "x" << bps;

      case PHOTOMETRIC_RGB:
        if (samples == 3 && bps==8){ // RGB
          img = Image(w1,h1, IMAGE_24RGB);
          break;
        }
        if (samples == 4 && bps==8){ // RGBA
          img = Image(w1,h1, IMAGE_32ARGB);
          break;
        }
        if (samples == 1 && bps==8){ // G
          img = Image(w1,h1, IMAGE_8);
          break;
        }
        throw Err() << "image_load_tiff: unsupported format: "
                    << "RGB " << samples << "x" << bps;

      case PHOTOMETRIC_MINISWHITE:
      case PHOTOMETRIC_MINISBLACK:
        if (samples == 1 && bps==16){ // 16bit
          img = Image(w1,h1, IMAGE_16);
          break;
        }
        if (samples == 1 && bps == 8){ // 8bit
          img = Image(w1,h1, IMAGE_8);
          break;
        }
        throw Err() << "image_load_tiff: unsupported format: "
                    << "MINISWHITE/MINISBLACK " << samples << "x" << bps;

      default: throw Err() << "image_load_tiff: unsupported photometric type: " << photometric;
    }


    // allocate buffer
    int scan = TIFFScanlineSize(tif);
    cbuf = (uint8 *)_TIFFmalloc(scan);

    // Main loop
    // Note that there are less checks then in the image creation switch().
    //

    int line = 0;
    for (int y=0; y<h1; ++y){

      if (can_skip_lines) line = y*scale;

      while (line<=rint(y*scale)){
        TIFFReadScanline(tif, cbuf, line);
        ++line;
      }

      for (int x=0; x<w1; ++x){
        uint32_t c;
        int xs = scale==1.0? x:rint(x*scale);
        switch (photometric){

          case PHOTOMETRIC_PALETTE:
            img.data()[y*w1+x] = cbuf[xs];
            break;

          case PHOTOMETRIC_RGB:
            if (samples==3){ // RGB
              memcpy(img.data() + 3*(y*w1+x), cbuf + 3*xs, 3);
              break;
            }
            if (samples==4){ // RGBA
              ((uint32_t*)img.data())[y*w1+x] =
                 color_argb(cbuf[4*xs+3], cbuf[4*xs], cbuf[4*xs+1], cbuf[4*xs+2]);
              break;
            }
            if (samples==1){ // G
              img.data()[y*w1+x] = cbuf[xs];
              break;
            }
            throw Err() << "image_load_tiff: unsupported format: "
                        << "RGB " << samples << "x" << bps;

          case PHOTOMETRIC_MINISWHITE:
            if (bps==16){ // 16bit
              ((uint16_t*)img.data())[y*w1+x] = 0xFFFF - ((uint16_t*)cbuf)[xs];
              break;
            }
            if (bps==8){ // 8bit
              img.data()[y*w1+x] = 0xFF - cbuf[xs];
              break;
            }
            throw Err() << "image_load_tiff: unsupported format: "
                        << "MINISWHITE " << samples << "x" << bps;

          case PHOTOMETRIC_MINISBLACK:
            if (bps==16){ // 16bit
              ((uint16_t*)img.data())[y*w1+x] = ((uint16_t*)cbuf)[xs];
              break;
            }
            if (bps==8){ // 8bit
              img.data()[y*w1+x] = cbuf[xs];
              break;
            }
            throw Err() << "image_load_tiff: unsupported format: "
                        << "MINISBLACK " << samples << "x" << bps;

          default: throw Err() <<
            "image_load_tiff: unsupported photometric type: " << photometric;
        }
      }
    }
    throw Err();
  }
  catch (Err e) {
    if (cbuf) _TIFFfree(cbuf);
    if (tif) TIFFClose(tif);
    if (e.str() != "") throw e;
  }
  return img;
}


/**********************************************************/

void image_save_tiff(const Image & im, std::ostream & str, const Opt & opt){

  TIFF *tif = NULL;
  tdata_t buf = NULL;

  try {
    int samples = 3; // samples per pixel
    int bps = 8;     // bits per sample
    bool use_cmap = false;
    switch (im.type()){
      case IMAGE_32ARGB:  samples=4; break;
      case IMAGE_24RGB:   samples=3; break;
      case IMAGE_16:      samples=1; bps=16; break;
      case IMAGE_8:       samples=1; break;
      case IMAGE_1:       samples=1; break;
      case IMAGE_8PAL:    samples=1; use_cmap=1; break;
      case IMAGE_FLOAT:   samples=3; break;
      case IMAGE_DOUBLE:  samples=3; break;
      case IMAGE_UNKNOWN: samples=4; break;
    }

    // set TIFF type from options
    std::string s;
    s = opt.get("tiff_format", "");
    if (s != "") {
      if      (s == "argb")  {bps=8; samples = 4; use_cmap = 0; }
      else if (s == "rgb")   {bps=8; samples = 3; use_cmap = 0; }
      else if (s == "grey")  {bps=8; samples = 1; use_cmap = 0; }
      else if (s == "pal")   {bps=8; samples = 1; use_cmap = 1;}
      else throw Err() << "image_save_tiff: unknown tiff_format setting: " << s << "\n";
    }

    // palette
    Image im8 = im;
    if (s == "pal"){
      Opt opt1(opt);
      opt1.put("cmap_alpha", "none");
      std::vector<uint32_t> colors = image_colormap(im, opt1);
      im8 = image_remap(im, colors, opt1);
    }

    // open file
    tif = TIFFStreamWOpen(str);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im.width());
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im.height());
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   bps);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,    1);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,    1);
    TIFFSetField(tif, TIFFTAG_COMPRESSION,     COMPRESSION_LZW);

    if (samples == 3 || samples == 4){
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    }

    if (samples == 4){
      int type=EXTRASAMPLE_UNASSALPHA;
      TIFFSetField(tif, TIFFTAG_EXTRASAMPLES,  1, &type);
    }

    if (samples == 1 && !use_cmap ){
      if (opt.get("tiff_minwhite", false))
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
      else
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    }

    uint16 cmap[3][256];
    if (use_cmap){
      for (int i=0; i<im8.cmap.size(); i++){
        cmap[0][i] = (im8.cmap[i]>>8)&0xFF00;
        cmap[1][i] =  im8.cmap[i]    &0xFF00;
        cmap[2][i] = (im8.cmap[i]<<8)&0xFF00;
      }
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
      TIFFSetField(tif, TIFFTAG_COLORMAP, cmap, cmap+1, cmap+2);
    }

    // note: only for bps>=8!
    int scan = samples*bps*im.width()/8;

    buf = _TIFFmalloc(scan);
    uint8 *cbuf = (uint8 *)buf;

    for (int y=0; y<im.height(); y++){
      for (int x=0; x<im.width(); x++){
        uint32_t c;
        //uint16_t c16;
        switch (samples*bps){

          case 32:
            c = im.get_argb(x, y);
            cbuf[4*x+3] = (c >> 24) & 0xFF;
            c = color_rem_transp(c, false);
            cbuf[4*x]   = (c >> 16) & 0xFF;
            cbuf[4*x+1] = (c >> 8)  & 0xFF;
            cbuf[4*x+2] = c & 0xFF;
            break;

          case 24:
            c = im.get_rgb(x, y);
            cbuf[3*x]   = (c >> 16) & 0xFF;
            cbuf[3*x+1] = (c >> 8)  & 0xFF;
            cbuf[3*x+2] = c & 0xFF;
            break;

          case 16:
            ((uint16_t *)cbuf)[x] = im.get_grey16(x, y);
            break;

          case 8:
            if (use_cmap) cbuf[x] = im8.get8(x,y);
            else          cbuf[x] = im.get_grey8(x,y);
            break;

        }
      }
      TIFFWriteScanline(tif, buf, y);
    }
    throw Err();
  }
  catch (Err e) {
    if (buf) _TIFFfree(buf);
    if (tif) TIFFClose(tif);
    if (e.str() != "") throw e;
  }
}


/**********************************************************/

iPoint
image_size_tiff(const std::string & fname){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  iPoint ret;
  try { ret = image_size_tiff(str); }
  catch(Err e){ throw Err() << e.str() << ": " << fname; }
  return ret;
}

Image
image_load_tiff(const std::string & fname, const double scale){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  Image ret;
  try { ret = image_load_tiff(str, scale); }
  catch(Err e){ throw Err() << e.str() << ": " << fname; }
  return ret;
}

void
image_save_tiff(const Image & im, const std::string & fname,
               const Opt & opt){
  std::ofstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  try { image_save_tiff(im, str, opt); }
  catch(Err e){ throw Err() << e.str() << ": " << fname; }
}
