#include <cstring>
#include <string>
#include <fstream>
#include <climits>
#include "io_pnm.h"
#include "image_colors.h"
#include <setjmp.h>

/**********************************************************/
// based on code from libnetpbm

enum {
  UNKNOWN   = 0,
  PBM_TYPE  = 0x5031, // P1
  PGM_TYPE  = 0x5032, // P3
  PPM_TYPE  = 0x5033, // P3
  RPBM_TYPE = 0x5034, // P1
  RPGM_TYPE = 0x5035, // P3
  RPPM_TYPE = 0x5036, // P6
//  PAM_TYPE  = 0x5037, // P7
} pnm_type_t;

// pnm structure (pam is not supported yet)
struct pnm_t {
  int fmt;
  unsigned int width, height, maxval, depth;

  pnm_t(): fmt(UNKNOWN), width(0), height(0), maxval(0), depth(0) {}
  unsigned int bps() const {
    if (maxval==1) return 0;
    return maxval<256 ? 1:2;
  }

  unsigned int row_size() const {
    if (maxval==1) return (width+7)/8;
    return width*bps()*depth;
  }
  bool is_plain() const {
    return fmt==PBM_TYPE || fmt==PGM_TYPE || fmt==PPM_TYPE;
  }
};

// get a single char, skip comments
char
pm_getc(std::istream & str) {
  char ch;
  if (!str.get(ch)) throw Err() << "pnm: unexpected EOF";
  if (ch == '#') {
    do {
      if (!str.get(ch)) throw Err() << "pnm: unexpected EOF";
    } while (ch != '\n' && ch != '\r');
  }
  return ch;
}

// Get an unsigned int.
// There is some hystorical story why it can not be larger
// then MAX_INT, not MAX_UINT.
unsigned int
pm_getuint(std::istream & str) {
  char ch;
  unsigned int i;
  do {
    ch = pm_getc(str);
  } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

  if (ch < '0' || ch > '9')
    throw Err() << "pnm: junk in file where an unsigned integer should be";

  i = 0;
  do {
    unsigned int const digit = ch - '0';
    if (i > INT_MAX/10 || 10*i > INT_MAX - digit)
      throw Err() << "pnm: ASCII decimal integer in file is "
                     "too large to be processed.";
    i = 10*i+digit;
    ch = pm_getc(str);
  } while (ch >= '0' && ch <= '9');
  return i;
}

// Read PNM header (see pnm_readpaminit)
pnm_t
pnm_read_head(std::istream & str) {
  pnm_t pnm;

  { // read magic number
    char ch1, ch2;
    if (!str.get(ch1) || !str.get(ch2))
      throw Err() << "pnm: can't read header";
    pnm.fmt = (int)ch1 * 256 + ch2;
  }

  switch (pnm.fmt) {
    //case PAM_TYPE:
    //  TODO
    //  break;

    case PPM_TYPE:
    case RPPM_TYPE:
      pnm.width  = pm_getuint(str);
      pnm.height = pm_getuint(str);
      pnm.maxval = pm_getuint(str);
      pnm.depth  = 3;
      break;

    case PGM_TYPE:
    case RPGM_TYPE:
      pnm.width  = pm_getuint(str);
      pnm.height = pm_getuint(str);
      pnm.maxval = pm_getuint(str);
      pnm.depth  = 1;
      break;

    case PBM_TYPE:
    case RPBM_TYPE:
      pnm.width  = pm_getuint(str);
      pnm.height = pm_getuint(str);
      pnm.maxval = 1;
      pnm.depth  = 1;
      break;

    default:
      throw Err() << "bad magic number, not a PNM file: "
                  << std::hex << pnm.fmt;
  }

  if (pnm.maxval == 0 || pnm.maxval > 65535) throw Err()
    << "pnm: wrong maxval value (1 to 65535 expected): " << pnm.maxval;

  return pnm;
}

// read a single row of data (byte array in the pnm order)
std::vector<uint8_t>
pnm_read_row(std::istream & str, const pnm_t & pnm){
  std::vector<uint8_t> ret(pnm.row_size(), 0);

  // raw fmt:
  if (!pnm.is_plain()){
    str.read((char*)ret.data(), pnm.row_size());
    return ret;
  }

  // plain fmt
  size_t bps = pnm.bps();
  for (size_t x = 0; x<pnm.width; ++x){
    for (size_t c = 0; c<pnm.depth; c++){
      auto val = pm_getuint(str);
      if (val > pnm.maxval) throw Err() << "pnm: value exceeds maxval: " << val;
      switch (bps) {
      case 0:
        if (val) ret[x/8] |=  (1<<(7-x%8));
        else     ret[x/8] &= ~(1<<(7-x%8));
        break;
      case 1:
        ret[x*pnm.depth + c] = val & 0xFF;
        break;
      case 2:
        ret[2*(x*pnm.depth + c)] = val & 0xFF;
        ret[2*(x*pnm.depth + c)+1] = (val>>8) & 0xFF;
        break;
      }
    }
  }
  return ret;
}

/**********************************************************/
// getting file dimensions from stream
iPoint image_size_pnm(std::istream & str){
  auto pnm = pnm_read_head(str);
  return iPoint(pnm.width, pnm.height);
}

/**********************************************************/

ImageR
image_load_pnm(std::istream & str, const double scale){

  if (scale < 1) throw Err() << "image_load_pnm: wrong scale: " << scale;

  // unscaled parameters
  auto pnm = pnm_read_head(str);
  auto bpr = pnm.row_size();
  auto bps = pnm.bps();
  auto bpp = bps*pnm.depth;

  // scaled image size
  size_t w1 = floor((pnm.width-1)/scale+1);
  size_t h1 = floor((pnm.height-1)/scale+1);
  ImageR img;

  // create image (type depends on pnm fmt)
  switch (pnm.fmt) {
    //case PAM_TYPE: TODO

    case PPM_TYPE:
    case RPPM_TYPE:
      img = ImageR(w1,h1, bps==1? IMAGE_24RGB:IMAGE_48RGB);
      break;

    case PGM_TYPE:
    case RPGM_TYPE:
      img = ImageR(w1,h1, bps==1? IMAGE_8:IMAGE_16);
      break;

    case PBM_TYPE:
    case RPBM_TYPE:
      img = ImageR(w1,h1, IMAGE_1);
      break;

    default:
      throw Err() << "unsupported PNM type: "
                  << std::hex << pnm.fmt;
  }

  // Main loop
  size_t line = 0;
  for (size_t y=0; y<h1; ++y){

    // skip lines until y*scale
    while (line < rint(y*scale)){
      if (pnm.is_plain())
        pnm_read_row(str, pnm);
      else
        str.seekg(bpr,std::ios_base::cur);
      line++;
    }

    // read line (byte array in the pnm order)
    auto cbuf = pnm_read_row(str, pnm);
    line++;

    // transfer line to the image

//    if (scale==1 && (pnm.fmt == PBM_TYPE || pnm.fmt == RPBM_TYPE)){
//      std::memcpy(img.data() + y*bps, cbuf.data(), bpr);
//      continue;
//    }

    for (size_t x=0; x<w1; ++x){
      int xs = scale==1.0? x:rint(x*scale);

      if (pnm.fmt == PBM_TYPE || pnm.fmt == RPBM_TYPE){
        img.set1(x,y, ((cbuf[xs/8]) >> (7-(xs%8))) & 1);
        continue;
      }
      char *src = (char*)cbuf.data() + xs*bpp;
      char *dst = (char*)img.data() + (y*w1+x)*bpp;
      for (int i = 0; i<bpp; i+=1)
        std::memcpy(dst + bpp - i - 1, src + i, 1);
    }
  }
  return img;
}


/**********************************************************/

void image_save_pnm(const ImageR & img, std::ostream & str, const Opt & opt){

  pnm_t pnm;
  pnm.width  = img.width();
  pnm.height = img.height();
  auto t = img.type();
  auto w = img.width();

  switch (t){
    case IMAGE_32ARGB:
    case IMAGE_24RGB:
    case IMAGE_8PAL:   pnm.fmt=RPPM_TYPE; pnm.depth=3; pnm.maxval=0xFF;   break;
    case IMAGE_16:     pnm.fmt=RPGM_TYPE; pnm.depth=1; pnm.maxval=0xFFFF; break;
    case IMAGE_8:      pnm.fmt=RPGM_TYPE; pnm.depth=1; pnm.maxval=0xFF;   break;
    case IMAGE_1:      pnm.fmt=RPBM_TYPE; pnm.depth=1; pnm.maxval=1;      break;
    case IMAGE_64ARGB:
    case IMAGE_48RGB:   pnm.fmt=RPPM_TYPE; pnm.depth=3; pnm.maxval=0xFFFF;   break;
    default: throw Err() << "image_save_pnm: unsupported image type";
  }
  auto bpr = pnm.row_size();
  auto bps = pnm.bps();
  auto bpp = bps*pnm.depth;

  // write header
  str << 'P' << (char)(pnm.fmt & 0xFF) << "\n"
      << pnm.width << " " << pnm.height << "\n";
  if (pnm.fmt!=RPBM_TYPE && pnm.fmt!=PBM_TYPE) str << pnm.maxval << "\n";

  for (size_t y=0; y<img.height(); y++){
    char *p = (char *)img.data() + bpr*y;
    switch (t){
      case IMAGE_24RGB:
      case IMAGE_32ARGB:
        for (size_t x=0; x<w; x++){
          auto c = img.get_rgb(x,y); // unscale color!
          str.write((char*)&c + 2, 1);
          str.write((char*)&c + 1, 1);
          str.write((char*)&c + 0, 1);
        }
        break;
      case IMAGE_8PAL:
        for (size_t x=0; x<w; x++){
          auto c = img.get8pal(x,y);
          str.write((char*)&c + 2, 1);
          str.write((char*)&c + 1, 1);
          str.write((char*)&c + 0, 1);
        }
        break;
      case IMAGE_16:
        for (size_t x=0; x<w; x++){
          auto c = img.get16(x,y);
          str.write((char*)&c+1, 1);
          str.write((char*)&c, 1);
        }
        break;
      case IMAGE_8:
        for (size_t x=0; x<w; x++){
          auto c = img.get8(x,y);
          str.write((char*)&c, 1);
        }
        break;
      case IMAGE_48RGB:
        for (size_t x=0; x<w; x++){
          auto c = img.get48(x,y);
          for (int i=0; i<6; i++)
            str.write((char*)&c + 5-i, 1);
        }
        break;
      case IMAGE_64ARGB:
        for (size_t x=0; x<w; x++){
          auto c = color_rem_transp64(img.get64(x,y),false); // unscale
          for (int i=0; i<6; i++)
            str.write((char*)&c + 5-i, 1);
        }
        break;
      case IMAGE_1:
        str.write((char*)img.data() + y*bpr, bpr);
        break;
    }
  }
}


/**********************************************************/

iPoint
image_size_pnm(const std::string & fname){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  iPoint ret;
  try { ret = image_size_pnm(str); }
  catch(Err & e){ e << ": " << fname; throw;}
  return ret;
}

ImageR
image_load_pnm(const std::string & fname, const double scale){
  std::ifstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  ImageR ret;
  try { ret = image_load_pnm(str, scale); }
  catch(Err & e){ e << ": " << fname; throw;}
  return ret;
}

void
image_save_pnm(const ImageR & im, const std::string & fname,
               const Opt & opt){
  std::ofstream str(fname);
  if (!str) throw Err() << "Can't open file: " << fname;
  try { image_save_pnm(im, str, opt); }
  catch(Err & e){ e << ": " << fname; throw;}
}
