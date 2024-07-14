#include <tiffio.h>
#include <cstring>
#include <string>
#include <fstream>
#include "io_tiff.h"
#include "image_colors.h"
#include <setjmp.h>

/**********************************************************/
// Custom error handler.
// We are using longjmp + global vars instead of throwing
// an exception from the callback.
// See notes in io_jpeg.cpp.

Err tiff_err; // error to be thrown
jmp_buf tiff_jmp_buf;

#include <cstdarg>
void
my_error_exit (const char* module, const char* fmt, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, fmt, args);
  tiff_err = Err() << module << " error: " << buf;
  longjmp(tiff_jmp_buf, 0);
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
  str->seekg(0, std::ios_base::end);
  auto pos1 = str->tellg();
  str->seekg(pos0, std::ios_base::beg);
  return pos1;
}

static toff_t
TiffStrWSizeProc(thandle_t handle){
  auto str = (std::ostream*) handle;
  auto pos0 = str->tellp();
  str->seekp(0, std::ios_base::end);
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
    if (setjmp(tiff_jmp_buf)) throw tiff_err;

    tif = TIFFStreamROpen(str);
    if (!tif) throw Err() << "image_size_tiff: can't open TIFF";

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
  }
  catch (Err & e){
    if (tif) TIFFClose(tif);
    throw;
  }
  if (tif) TIFFClose(tif);
  return iPoint(w,h);
}

/**********************************************************/
// LOading Tiff file

// Create image according with TIFF type
ImageR
tiff_make_image(TIFF* tif, const size_t w1, const size_t h1,
                const int samples, const int bps, const uint16_t photometric) {

  // create image (type depends on TIFF type)
  switch (photometric){

    case PHOTOMETRIC_PALETTE:
      if (samples == 1 && bps == 8) {
        ImageR img(w1,h1, IMAGE_8PAL);
        img.cmap.clear();
        uint16_t *cmap[3];
        TIFFGetField(tif, TIFFTAG_COLORMAP, cmap, cmap+1, cmap+2);
        for (int i=0; i<256; i++){
          img.cmap.push_back(
            (((uint32_t)cmap[0][i] & 0xFF00)<<8) +
             ((uint32_t)cmap[1][i] & 0xFF00) +
            (((uint32_t)cmap[2][i] & 0xFF00)>>8) +
            0xFF000000);
        }
        return img;
      }
      throw Err() << "image_load_tiff: unsupported format: "
                  << "PALETTE " << samples << "x" << bps;

    case PHOTOMETRIC_RGB:

      if (samples == 3 && bps==8) // 24RGB
        return ImageR(w1,h1, IMAGE_24RGB);

      if (samples == 3 && bps==16) // 48RGB
        return ImageR(w1,h1, IMAGE_48RGB);

      if (samples == 4 && bps==8) // 32RGBA
        return ImageR(w1,h1, IMAGE_32ARGB);

      if (samples == 4 && bps==16) // 64RGBA
        return ImageR(w1,h1, IMAGE_64ARGB);

      if (samples == 1 && bps==8) // G
        return ImageR(w1,h1, IMAGE_8);

      throw Err() << "image_load_tiff: unsupported format: "
                  << "RGB " << samples << "x" << bps;

    case PHOTOMETRIC_MINISWHITE:
    case PHOTOMETRIC_MINISBLACK:

      if (samples == 1 && bps==16) // 16bit
        return ImageR(w1,h1, IMAGE_16);

      if (samples == 1 && bps == 8) // 8bit
        return ImageR(w1,h1, IMAGE_8);

      throw Err() << "image_load_tiff: unsupported format: "
                  << "MINISWHITE/MINISBLACK " << samples << "x" << bps;
  }
  throw Err() << "image_load_tiff: unsupported photometric type: " << photometric;
}

/*******************/
// Transfer point from tiff buffer to image.
// Some checks are skipped because ther were done in tiff_make_image().
void
tiff_image_pt(ImageR & img, const int x, const int y, uint8_t cbuf[], const int xs,
          const int samples, const int bps, const uint16_t photometric) {

  switch (photometric){

    case PHOTOMETRIC_PALETTE:
      img.set8(x,y, cbuf[xs]);
      break;

    case PHOTOMETRIC_RGB:
      if (samples==3){ // RGB
        if (bps==8){
          uint8_t r = cbuf[3*xs];
          uint8_t g = cbuf[3*xs+1];
          uint8_t b = cbuf[3*xs+2];
          img.set24(x,y, color_argb(0xFF, r,g,b));
        }
        if (bps==16){
          uint16_t r = ((uint16_t)cbuf[6*xs+0]<<8) + cbuf[6*xs+1];
          uint16_t g = ((uint16_t)cbuf[6*xs+2]<<8) + cbuf[6*xs+3];
          uint16_t b = ((uint16_t)cbuf[6*xs+4]<<8) + cbuf[6*xs+5];
          img.set64(x,y, color_argb64(0xFFFF,r,g,b));
        }
        break;
      }
      if (samples==4){ // RGBA -- // color scaling!
        if (bps==8){
          uint8_t r = cbuf[4*xs];
          uint8_t g = cbuf[4*xs+1];
          uint8_t b = cbuf[4*xs+2];
          uint8_t a = cbuf[4*xs+3];
          img.set32(x,y, color_argb(a,r,g,b));
        }
        if (bps==16){
          uint16_t r = ((uint16_t)cbuf[8*xs+0]<<8) + cbuf[8*xs+1];
          uint16_t g = ((uint16_t)cbuf[8*xs+2]<<8) + cbuf[8*xs+3];
          uint16_t b = ((uint16_t)cbuf[8*xs+4]<<8) + cbuf[8*xs+5];
          uint16_t a = ((uint16_t)cbuf[8*xs+6]<<8) + cbuf[8*xs+7];
          img.set64(x,y, color_argb64(a,r,g,b));
        }
        break;
      }
      if (samples==1 && bps==8){ // G
        img.set8(x,y, cbuf[xs]);
        break;
      }

    case PHOTOMETRIC_MINISWHITE:
      if (bps==16){ // 16bit
        img.set16(x,y, 0xFFFF - ((uint16_t*)cbuf)[xs]);
        break;
      }
      if (bps==8){ // 8bit
        img.set8(x,y, 0xFF - cbuf[xs]);
        break;
      }

    case PHOTOMETRIC_MINISBLACK:
      if (bps==16){ // 16bit
        img.set16(x,y, ((uint16_t*)cbuf)[xs]);
        break;
      }
      if (bps==8){ // 8bit
        img.set8(x,y, cbuf[xs]);
        break;
      }
    default:
      throw Err() << "image_load_tiff: unsupported photometric type: " << photometric;
  }
}

/*******************/

ImageR
image_load_tiff(std::istream & str, const double scale){

  if (scale < 1) throw Err() << "image_load_tiff: wrong scale: " << scale;

  uint8_t *cbuf = NULL;
  TIFF* tif = NULL;
  ImageR img;

  try {
    TIFFSetErrorHandler((TIFFErrorHandler)&my_error_exit);
    if (setjmp(tiff_jmp_buf)) throw tiff_err;

    tif = TIFFStreamROpen(str);
    if (!tif) throw Err() << "image_load_tiff: can't load tiff";

    // image dimensions
    uint32_t w, h;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

    // scaled image size
    uint32_t w1 = floor((w-1)/scale+1);
    uint32_t h1 = floor((h-1)/scale+1);

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

    img = tiff_make_image(tif, w1, h1, samples, bps, photometric);


    // Main loop

    // allocate buffer
    size_t scan = TIFFScanlineSize(tif);
    cbuf = (uint8_t *)_TIFFmalloc(scan);

    size_t line = 0;
    for (size_t y=0; y<h1; ++y){

      if (can_skip_lines) line = y*scale;

      while (line<=rint(y*scale)){
        TIFFReadScanline(tif, cbuf, line);
        ++line;
      }

      for (size_t x=0; x<w1; ++x){
        int xs = scale==1.0? x:rint(x*scale);
        tiff_image_pt(img, x, y, cbuf, xs, samples, bps, photometric);
      }
    }

  }
  catch (Err & e) {
    if (cbuf) _TIFFfree(cbuf);
    if (tif) TIFFClose(tif);
    throw e;
  }
  if (cbuf) _TIFFfree(cbuf);
  if (tif) TIFFClose(tif);
  return img;
}


/**********************************************************/

void image_save_tiff(const ImageR & im, std::ostream & str, const Opt & opt){

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
      case IMAGE_64ARGB:  samples=4; bps=16; break;
      case IMAGE_48RGB:   samples=3; bps=16; break;
      case IMAGE_UNKNOWN: samples=4; break;
    }

    // set TIFF type from options
    std::string s;
    s = opt.get("tiff_format", "");
    if (s != "") {
      if      (s == "argb")  {bps=8; samples = 4; use_cmap = 0; }
      else if (s == "rgb")   {bps=8; samples = 3; use_cmap = 0; }
      else if (s == "argb64"){bps=16; samples = 4; use_cmap = 0; }
      else if (s == "rgb64") {bps=16; samples = 3; use_cmap = 0; }
      else if (s == "grey")  {bps=8; samples = 1; use_cmap = 0; }
      else if (s == "pal")   {bps=8; samples = 1; use_cmap = 1;}
      else throw Err() << "image_save_tiff: unknown tiff_format setting: " << s << "\n";
    }

    // palette
    ImageR im8 = im;
    if (s == "pal"){
      Opt opt1(opt);
      opt1.put("cmap_alpha", "none");
      std::vector<uint32_t> colors = image_colormap(im, opt1);
      im8 = image_remap(im, colors);
    }

    // set error callback
    TIFFSetErrorHandler((TIFFErrorHandler)&my_error_exit);
    if (setjmp(tiff_jmp_buf)) throw tiff_err;

    // open file
    tif = TIFFStreamWOpen(str);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im.width());
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im.height());
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   bps);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG,    1);

    // Compression
    // Supported by libtiff (see man libtiff and tiff.h):
    //  - no compression                       (COMPRESSION_NONE = 1)
    //  - CCITT 1D Huffman compression         (COMPRESSION_CCITTRLE = 2),
    //  - CCITT Group 3 Facsimile compression  (COMPRESSION_CCITTFAX3 = 3),
    //  - CCITT Group 4 Facsimile compression  (COMPRESSION_CCITTFAX4 = 4),
    //  - Lempel-Ziv & Welch compression       (COMPRESSION_LZW = 5),
    //  - baseline JPEG compression            (COMPRESSION_JPEG = 7),
    //  - word-aligned 1D Huffman compression  (COMPRESSION_CCITTRLEW = 32771),
    //  - PackBits compression (Macintosh RLE) (COMPRESSION_PACKBITS = 32773)
    s = opt.get("tiff_compression", "lzw");
    uint16_t tiff_compr = 1;
    if      (s == "none")      tiff_compr = COMPRESSION_NONE;
    else if (s == "ccit_rle")  tiff_compr = COMPRESSION_CCITTRLE;
    else if (s == "ccit_rlew") tiff_compr = COMPRESSION_CCITTRLEW;
    else if (s == "ccit_fax3") tiff_compr = COMPRESSION_CCITTFAX3;
    else if (s == "ccit_fax4") tiff_compr = COMPRESSION_CCITTFAX4;
    else if (s == "lzw")       tiff_compr = COMPRESSION_LZW;
    else if (s == "jpeg")      tiff_compr = COMPRESSION_JPEG;
    else if (s == "packbits")  tiff_compr = COMPRESSION_PACKBITS;
    else throw Err() << "Unknown --tiff_compression value: " << s;
    TIFFSetField(tif, TIFFTAG_COMPRESSION, tiff_compr);

    // Set ROWSPERSTRIP. We want 1 for random access in file
    // loading. Jpeg compression supports only multiples of 8:
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 
      tiff_compr == COMPRESSION_JPEG ? 8:1);

    // Set quality for JPEG compression (note, that TIFFTAG_JPEGQUALITY
    // is a "pseudo tag", it's not written to the file):
    if (tiff_compr == COMPRESSION_JPEG) {
      TIFFSetField(tif, TIFFTAG_JPEGQUALITY, opt.get("jpeg_quality", 95));
    }

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

    uint16_t cmap[3][256];
    if (use_cmap){
      for (size_t i=0; i<im8.cmap.size(); i++){
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
    uint8_t *cbuf = (uint8_t *)buf;

    for (size_t y=0; y<im.height(); y++){
      for (size_t x=0; x<im.width(); x++){
        uint32_t c;
        uint64_t c64;
        switch (samples*bps){

          case 64:
            c64 = im.get_argb64(x, y);
            cbuf[8*x+7] = (c64 >> 56) & 0xFF;
            cbuf[8*x+6] = (c64 >> 48) & 0xFF;
            c64 = color_rem_transp64(c64, false); // unscaled color!
            cbuf[8*x+1] = (c64 >> 40) & 0xFF;
            cbuf[8*x+0] = (c64 >> 32) & 0xFF;
            cbuf[8*x+3] = (c64 >> 24) & 0xFF;
            cbuf[8*x+2] = (c64 >> 16) & 0xFF;
            cbuf[8*x+5] = (c64 >> 8)  & 0xFF;
            cbuf[8*x+4] = c64 & 0xFF;
            break;

          case 48:
            c64 = im.get_rgb64(x, y); // unscaled color
            cbuf[6*x+1] = (c64 >> 40) & 0xFF;
            cbuf[6*x+0] = (c64 >> 32) & 0xFF;
            cbuf[6*x+3] = (c64 >> 24) & 0xFF;
            cbuf[6*x+2] = (c64 >> 16) & 0xFF;
            cbuf[6*x+5] = (c64 >> 8)  & 0xFF;
            cbuf[6*x+4] = c64 & 0xFF;
            break;

          case 32:
            c = im.get_argb(x, y);
            cbuf[4*x+3] = (c >> 24) & 0xFF;
            c = color_rem_transp(c, false); // unscaled color
            cbuf[4*x]   = (c >> 16) & 0xFF;
            cbuf[4*x+1] = (c >> 8)  & 0xFF;
            cbuf[4*x+2] = c & 0xFF;
            break;

          case 24:
            c = im.get_rgb(x, y); // unscaled color
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
  }
  catch (Err & e) {
    if (buf) _TIFFfree(buf);
    if (tif) TIFFClose(tif);
    throw;
  }
  if (buf) _TIFFfree(buf);
  if (tif) TIFFClose(tif);
}


/**********************************************************/

iPoint
image_size_tiff(const std::string & fname){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  iPoint ret;
  try { ret = image_size_tiff(str); }
  catch(Err & e){ e << ": " << fname; throw;}
  return ret;
}

ImageR
image_load_tiff(const std::string & fname, const double scale){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  ImageR ret;
  try { ret = image_load_tiff(str, scale); }
  catch(Err & e){ e << ": " << fname; throw;}
  return ret;
}

void
image_save_tiff(const ImageR & im, const std::string & fname,
               const Opt & opt){
  std::ofstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  try { image_save_tiff(im, str, opt); }
  catch(Err & e){ e << ": " << fname; throw;}
}
