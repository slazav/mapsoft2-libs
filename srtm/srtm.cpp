#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <list>
#include "srtm.h"
#include "geom/line.h"
#include "geom/point_int.h"
#include "geom/poly_tools.h"
#include "filename/filename.h"
#include "image_cnt/image_cnt.h"
#include <zlib.h>
#include <tiffio.h>


// load srtm data from *.hgt file
ImageR
read_file(const std::string & file){
  FILE *F = fopen(file.c_str(), "rb");
  if (!F) return ImageR();

  // find file length
  fseek(F, 0L, SEEK_END);
  auto flen = ftell(F);
  fseek(F, 0L, SEEK_SET);
  size_t width=0;
  switch (flen){
    case 2*1201*1201: width=1201; break;
    case 2*3601*3601: width=3601; break;
    default: throw Err()
      << "SRTM: unsupported file size: " << file << ": " << flen;
  }

  ImageR im(width, width, IMAGE_16);
  size_t length = width*width*sizeof(short);

  if (length != fread(im.data(), 1, length, F))
    throw Err() << "SRTM: bad .hgt file: " << file;

  for (size_t i=0; i<length/2; i++){ // swap bytes
    uint16_t tmp = ((uint16_t*)im.data())[i];
    ((uint16_t*)im.data())[i] = (tmp >> 8) + (tmp << 8);
  }
  fclose(F);
  return im;
}

// load srtm data from *.hgt.gz file
ImageR
read_zfile(const std::string & file){
  gzFile F = gzopen(file.c_str(), "rb");
  if (!F) return ImageR();

  // find uncompressed file length
  size_t width=0;
  {
    unsigned char buf[4];
    unsigned int  flen;
    FILE *FU = fopen(file.c_str(), "rb");
    fseek(FU, -4L, SEEK_END);
    auto len = fread(buf, 1, 4, FU);
    flen = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
    fclose(FU);
    switch (flen){
      case 2*1201*1201: width=1201; break;
      case 2*3601*3601: width=3601; break;
      default: throw Err()
        << "SRTM: unsupported file size: " << file << ": " << flen;
    }
  }

  ImageR im(width, width, IMAGE_16);
  int length = width*width*sizeof(short);

  if (length != gzread(F, im.data(), length))
    throw Err() << "SRTM: bad .hgt.gz file: " << file;

  for (int i=0; i<length/2; i++){ // swap bytes
    uint16_t tmp=((uint16_t*)im.data())[i];
    ((uint16_t*)im.data())[i] = (tmp >> 8) + (tmp << 8);
  }
  gzclose(F);
  return im;
}

// load srtm data from *.tif file
ImageR
read_tfile(const std::string & file){

  // open Tiff file, get width and height
  if (!file_exists(file)) return ImageR();
  TIFF* tif = TIFFOpen(file.c_str(), "rb");
  if (!tif) throw Err() << "can't open tiff file: " << file;

  int tiff_w, tiff_h;
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &tiff_w);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &tiff_h);

  int scan = TIFFScanlineSize(tif);
  int bpp = scan/tiff_w;
  auto *cbuf = (uint8_t *)_TIFFmalloc(scan);


  ImageR im(tiff_w, tiff_h, IMAGE_16);

  try {

    // check format: should be 16bpp greyscale
    int photometric=0;
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
    if (photometric != PHOTOMETRIC_MINISBLACK)
      throw Err() << "unsupported photometric type: " << photometric;
    if (bpp != 2)
      throw Err() << "not a 2 byte-per-pixel tiff";

    // read image
    for (int y = 0; y<tiff_h; y++){
      TIFFReadScanline(tif, cbuf, y);
      for (int x = 0; x<tiff_w; x++){
        im.set16(x,y, (cbuf[2*x+1]<<8) + cbuf[2*x]);
      }
    }
  }
  catch (Err & e) {
    _TIFFfree(cbuf);
    TIFFClose(tif);
    throw Err() << "srtm: " << file << ": " << e.str();
  }

  _TIFFfree(cbuf);
  TIFFClose(tif);
  return im;
}


bool
SRTM::load(const iPoint & key){
  if ((key.x < -180) || (key.x >= 180) ||
      (key.y <  -90) || (key.y >=  90)) return false;

  char EW = key.x<0 ? 'W':'E';
  char NS = key.y<0 ? 'S':'N';

  // create filename
  std::ostringstream file;
  file << NS << std::setfill('0') << std::setw(2) << abs(key.y)
       << EW << std::setw(3) << abs(key.x);

  // try <name>.hgt.gz
  ImageR im = read_zfile(srtm_dir + "/" + file.str() + ".hgt.gz");

  // try <name>.hgt
  if (im.is_empty())
    im = read_file(srtm_dir + "/" + file.str() + ".hgt");

  // try <name>.tif
  if (im.is_empty())
    im = read_tfile(srtm_dir + "/" + file.str() + ".tif");

  if (im.is_empty())
    im = read_tfile(srtm_dir + "/" + file.str() + ".tiff");

  if (im.is_empty())
    std::cerr << "SRTM: can't find file: " << file.str() << "\n";

  srtm_cache.add(key, im);
  return !im.is_empty();
}


/************************************************/

void
ms2opt_add_srtm(GetOptSet & opts){
  const char *g = "SRTM";
  opts.add("srtm_dir", 1,0,g,
    "Set srtm data folder, default - $HOME/.srtm_data");
  opts.add("srtm_interp_holes", 1,0,g,
    "Interpolate holes (0|1, default 1).");

  opts.add("srtm_interp", 1,0,g,
    "Interpolation (nearest, linear, cubic. Default: linear).");
}

void
ms2opt_add_srtm_surf(GetOptSet & opts){
  ms2opt_add_srtm(opts);
  const char *g = "DRAWSRTM";
  opts.add("srtm_draw_mode", 1,0,g,
    "SRTM surface drawing mode (slopes, heights, shades, default - shades).");
  opts.add("srtm_hmin", 1,0,g,
    "Min height [m] for heights and shades modes (default - 0).");
  opts.add("srtm_hmax", 1,0,g,
    "Max height [m] for heights and shades modes (default - 5000).");
  opts.add("srtm_smin", 1,0,g,
    "Min slope [deg] for slopes mode (default - 35).");
  opts.add("srtm_smax", 1,0,g,
    "Max slope [deg] for slopes mode (default - 50).");
  opts.add("srtm_bgcolor", 1,0,g,
    "Color to draw no-data and out-of-scale areas (default 0x60FF0000).");
}

/************************************************/

SRTM::SRTM(const Opt & o): srtm_cache(SRTM_CACHE_SIZE) {
  set_opt(o);
}

Opt
SRTM::get_def_opt() {
  Opt o;
  o.put("srtm_dir", std::string(getenv("HOME")? getenv("HOME"):"") + "/.srtm_data");
  o.put("srtm_interp_holes", 1);
  o.put("srtm_interp", "linear");

  o.put("srtm_draw_mode", "shades");
  o.put("srtm_hmin", 0);
  o.put("srtm_hmax", 5000);
  o.put("srtm_smin", 35);
  o.put("srtm_smax", 50);
  o.put("srtm_bgcolor", 0x60FF0000);

  return o;
}


void
SRTM::set_opt(const Opt & opt){

  // Data directory. Default: $HOME/.srtm_data
  std::string dir = opt.get("srtm_dir",
    std::string(getenv("HOME")? getenv("HOME"):"") + "/.srtm_data");

  // set new value and clear data cache if needed
  if (dir!=srtm_dir){
    srtm_dir = dir;
    srtm_cache.clear();
  }

  auto srtm_interp_s = opt.get("srtm_interp", "linear");
  if      (srtm_interp_s == "nearest") srtm_interp = SRTM_NEAREST;
  else if (srtm_interp_s == "linear")  srtm_interp = SRTM_LINEAR;
  else if (srtm_interp_s == "cubic")   srtm_interp = SRTM_CUBIC;
  else throw Err() << "SRTM: unknown srtm_interp setting: " << srtm_interp_s;

  // surface parameters
  auto     m = opt.get("srtm_draw_mode", "shades");
  if      (m == "heights") { draw_mode = SRTM_DRAW_HEIGHTS; }
  else if (m == "shades")  { draw_mode = SRTM_DRAW_SHADES; }
  else if (m == "slopes")  { draw_mode = SRTM_DRAW_SLOPES; }
  else throw Err() << "unknown value of srtm_draw_mode parameter "
    << "(heights, shades, or slopes expected): " << m;

  // color limits
  double hmin = opt.get("srtm_hmin", 0.0);
  double hmax = opt.get("srtm_hmax", 5000.0);
  double smin = opt.get("srtm_smin", 35.0);
  double smax = opt.get("srtm_smax", 50.0);

  // set rainbow converters:
  if (draw_mode == SRTM_DRAW_HEIGHTS ||
      draw_mode == SRTM_DRAW_SHADES)
    R = Rainbow(hmin,hmax, RAINBOW_NORMAL);
  else if (draw_mode == SRTM_DRAW_SLOPES)
    R = Rainbow(smin,smax, RAINBOW_BURNING);

  bgcolor = opt.get<int>("srtm_bgcolor", 0x60FF0000);
}

/************************************************/
/************************************************/
// Distance between points (dx,dy) at a given place.
// (0,0) if data is missing.
dPoint
SRTM::get_step(const iPoint& p){
  // find tile
  dPoint ret;
  iPoint key = floor(p);
  if ((!srtm_cache.contains(key)) && (!load(key))) return ret;
  auto im = srtm_cache.get(key);
  if (im.is_empty()) return ret;
  auto w = im.width(), h = im.height();
  ret.x = 1.0/(w%2==1 ? w-1 : w);
  ret.y = 1.0/(h%2==1 ? h-1 : h);
  return ret;
}

// Low-level get function: rounding coordinate to the nearest point
int16_t
SRTM::get_raw(const dPoint& p){
  // find tile, load image, get width and height
  iPoint key = floor(p);
  if ((!srtm_cache.contains(key)) && (!load(key))) return SRTM_VAL_NOFILE;
  auto im = srtm_cache.get(key);
  if (im.is_empty()) return SRTM_VAL_NOFILE;
  auto w = im.width(), h = im.height();

  // Pixel coordinate (different for SRTM/ALOS):
  //   nearest pixel(srtm) = rint((pt - tile)*(width-1))  0..width-1
  //   nearest pixel(alos) = h- floor((pt - tile)*width) 0..width-1
  iPoint crd;
  crd.x = (w%2==1) ? rint((p.x - floor(p.x))*(w-1)) : floor((p.x - floor(p.x))*w);
  crd.y = (h%2==1) ? rint((ceil(p.y)  - p.y)*(h-1)) : floor((ceil(p.y)  - p.y)*h);

  // Use overlay
  if (overlay.count(key) && overlay[key].count(crd))
     return overlay[key][crd];

  // obtain the point
  return (int16_t)im.get16(crd.x, crd.y);
}

// Cubic interpolation (used in get_val_int16()).
// see http://www.paulinternet.nl/?page=bicubic
double
cubic_interp(const double h[4], const double x){
  return h[1] + 0.5 * x*(h[2] - h[0] + x*(2.0*h[0] - 5.0*h[1] + 4.0*h[2] -
              h[3] + x*(3.0*(h[1] - h[2]) + h[3] - h[0])));
}

// interpolate 1-point or 2-points holes
// maybe this can be written smarter...
void
int_holes(double h[4]){
  if ((h[0]>SRTM_VAL_MIN) && (h[1]>SRTM_VAL_MIN) && (h[2]>SRTM_VAL_MIN) && (h[3]>SRTM_VAL_MIN)) return;
  for (int cnt=0; cnt<2; cnt++){
    if      ((h[0]<=SRTM_VAL_MIN) && (h[1]>SRTM_VAL_MIN) && (h[2]>SRTM_VAL_MIN)) h[0]=2*h[1]-h[2];
    else if ((h[1]<=SRTM_VAL_MIN) && (h[0]>SRTM_VAL_MIN) && (h[2]>SRTM_VAL_MIN)) h[1]=(h[0]+h[2])/2;
    else if ((h[1]<=SRTM_VAL_MIN) && (h[2]>SRTM_VAL_MIN) && (h[3]>SRTM_VAL_MIN)) h[1]=2*h[2]-h[3];
    else if ((h[2]<=SRTM_VAL_MIN) && (h[1]>SRTM_VAL_MIN) && (h[3]>SRTM_VAL_MIN)) h[2]=(h[1]+h[3])/2;
    else if ((h[2]<=SRTM_VAL_MIN) && (h[0]>SRTM_VAL_MIN) && (h[1]>SRTM_VAL_MIN)) h[2]=2*h[1]-h[0];
    else if ((h[3]<=SRTM_VAL_MIN) && (h[1]>SRTM_VAL_MIN) && (h[2]>SRTM_VAL_MIN)) h[3]=2*h[2]-h[1];
    else break;
  }
  if ((h[1]<=SRTM_VAL_MIN) && (h[2]<=SRTM_VAL_MIN) && (h[0]>SRTM_VAL_MIN) && (h[3]>SRTM_VAL_MIN)){
    h[1]=(2*h[0] + h[3])/3;
    h[2]=(h[0] + 2*h[3])/3;
  }
}

// Get with interpolation
double
SRTM::get_h(const dPoint& p, bool raw){
  auto h0 = get_raw(p);
  if (h0<SRTM_VAL_MIN || raw || srtm_interp == SRTM_NEAREST) return h0;

  if (srtm_interp == SRTM_LINEAR) {
    dPoint d = get_step(p);
    double x = p.x - floor(p.x);
    double y = p.y - floor(p.y);
    double sx = floor(x/d.x)*d.x - x;
    double sy = floor(y/d.y)*d.y - y;

    auto h1 = get_raw(p + dPoint(sx, sy));
    auto h2 = get_raw(p + dPoint(sx, sy+d.y));
    if ((h1<SRTM_VAL_MIN)||(h2<SRTM_VAL_MIN)) return SRTM_VAL_UNDEF;
    double h12 = h1 - (h2-h1)*sy/d.y;

    auto h3=get_raw(p + dPoint(sx+d.x, sy));
    auto h4=get_raw(p + dPoint(sx+d.x, sy+d.y));
    if ((h3<SRTM_VAL_MIN)||(h4<SRTM_VAL_MIN)) return SRTM_VAL_UNDEF;
    double h34 = h3 - (h4-h3)*sy/d.y;
    return h12 - (h34-h12)*sx/d.x;
  }

  if (srtm_interp == SRTM_CUBIC) {
    dPoint d = get_step(p);
    double x = p.x - floor(p.x);
    double y = p.y - floor(p.y);
    double sx = floor(x/d.x)*d.x - x;
    double sy = floor(y/d.y)*d.y - y;
    double hx[4], hy[4];

    for (int i=0; i<4; i++){
      for (int j=0; j<4; j++){
        hx[j]=get_raw(p + dPoint(sx+d.x*(j-1), sy+d.y*(i-1)));
      }
      int_holes(hx);
      hy[i]= cubic_interp(hx, -sx/d.x);
    }
    int_holes(hy);
    return cubic_interp(hy, -sy/d.y);
  }

  throw Err() << "SRTM: unknown interpolation style: " << srtm_interp;
}

double
SRTM::get_s(const dPoint& p, bool raw){
  dPoint d = get_step(p);
  int h  = get_h(p, raw);
  int h1 = get_h(p + dPoint(-d.x/2.0, 0), raw);
  int h2 = get_h(p + dPoint(+d.x/2.0, 0), raw);
  if (h1 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h2 > SRTM_VAL_MIN) h1 = 2*h - h2;
  if (h2 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h1 > SRTM_VAL_MIN) h2 = 2*h - h1;

  int h3 = get_h(p + dPoint(0, -d.y/2.0));
  int h4 = get_h(p + dPoint(0, +d.y/2.0));
  if (h3 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h4 > SRTM_VAL_MIN) h3 = 2*h - h4;
  if (h4 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h3 > SRTM_VAL_MIN) h4 = 2*h - h3;

  if (h1 < SRTM_VAL_MIN || h2 < SRTM_VAL_MIN ||
      h3 < SRTM_VAL_MIN || h4 < SRTM_VAL_MIN) return NAN;

  d *= 6380e3 * M_PI/180;
  d.x *= cos(M_PI*p.y/180.0);
  double  U = hypot((h2-h1)/d.x, (h4-h3)/d.y);
  return atan(U)*180.0/M_PI;
}

/************************************************/

void
SRTM::overlay_cut(const dLine & l){
  auto r = l.bbox();
  dPolyTester pt(l);
  for (int x = floor(r.x); x<ceil(r.x+r.w); x++){
    for (int y = floor(r.y); y<ceil(r.y+r.h); y++){
      iPoint key(x,y);

      // load image (only for w and h)
      if ((!srtm_cache.contains(key)) && (!load(key))) continue;
      auto im = srtm_cache.get(key);
      size_t w = im.width(), h=im.height();
      if (im.is_empty()) continue;

      std::set<iPoint> pts;
      // not very efficient
      for (size_t cx = 0; cx < w; cx++){
        for (size_t cy = 0; cy < h; cy++){
          dPoint p(key);
          p.x += (double)cx/(w%2==1 ? w-1 : w);
          p.y += 1.0 - (double)cy/(h%2==1 ? h-1 : h);
          if (pt.test_pt(p)) pts.emplace(cx,cy);
        }
      }

      if (!pts.size()) continue;
      if (!overlay.count(key)) overlay[key] = std::map<iPoint, int16_t>();

      for (const auto & p:pts)
        overlay[key][p] = SRTM_VAL_UNDEF;
    }
  }
}

void
SRTM::overlay_clear(const dLine & l){
  auto r = l.bbox();
  dPolyTester pt(l);
  for (int x = floor(r.x); x<ceil(r.x+r.w); x++){
    for (int y = floor(r.y); y<ceil(r.y+r.h); y++){
      iPoint key(x,y);
      if (!overlay.count(key)) continue;

      if ((!srtm_cache.contains(key)) && (!load(key))) continue;
      auto im = srtm_cache.get(key);
      size_t w = im.width(), h=im.height();
      if (im.is_empty()) continue;
      dPoint d;
      d.x = 1.0/(w%2==1 ? w-1 : w);
      d.y = 1.0/(h%2==1 ? h-1 : h);

      auto i = overlay[key].begin();
      while (i!=overlay[key].end()){
        dPoint p1(i->first);
        dPoint p2(p1.x*d.x + key.x, 1.0 - p1.y*d.y + key.y);
        if (pt.test_pt(p2)) i = overlay[key].erase(i);
        else i++;
      }

    }
  }
}

/************************************************/

uint32_t
SRTM::get_color(const double h, const double s){
  switch (draw_mode){
    case SRTM_DRAW_SLOPES:  return R.get(s);
    case SRTM_DRAW_HEIGHTS: return R.get(h);
    case SRTM_DRAW_SHADES:  return color_shade(R.get(h), 1-s/90.0);
  }
  return bgcolor;
}


/// Get color for a point (lon-lat coords), according with drawing options.
uint32_t
SRTM::get_color(const dPoint & p, bool raw) {
  switch (draw_mode){
    case SRTM_DRAW_SLOPES:  return R.get(get_s(p, raw));
    case SRTM_DRAW_HEIGHTS: return R.get(get_h(p, raw));
    case SRTM_DRAW_SHADES:
      return color_shade(R.get(get_h(p, raw)), 1-get_s(p, raw)/90.0);
  }
  return bgcolor;
}

/************************************************/

std::map<double, dMultiLine>
SRTM::find_contours(const dRect & range, double step, double vtol){

  dPoint d = get_step(range.cnt());

  // integer rectangle covering the area
  iRect irange = ceil(range/d);

  int x1  = irange.tlc().x;
  int x2  = irange.brc().x;
  int y1  = irange.tlc().y;
  int y2  = irange.brc().y;

  // Extract altitudes for the whole range, make image
  ImageR img(x2-x1+1, y2-y1+1, IMAGE_16);
  for (int y=y1; y<=y2; y++){
    for (int x=x1; x<=x2; x++){
      img.set16(x-x1,y-y1, get_h(dPoint(x*d.x,y*d.y)));
    }
  }

  std::map<double, dMultiLine> ret = image_cnt(img, NAN, NAN, step, 0, vtol);
  for (auto & r:ret) r.second = (dPoint(x1,y1)+r.second)*d;
  return ret;
}

dMultiLine
SRTM::find_slope_contours(const dRect & range, double val, double vtol){

  dPoint d = get_step(range.cnt());

  // integer rectangle covering the area
  iRect irange = ceil(range/d);
  int x1  = irange.tlc().x;
  int x2  = irange.brc().x;
  int y1  = irange.tlc().y;
  int y2  = irange.brc().y;

  // Extract slopes for the whole range.
  // make image with slopes
  ImageR img(x2-x1+1, y2-y1+1, IMAGE_FLOAT);
  for (int y=y1; y<=y2; y++){
    for (int x=x1; x<=x2; x++){
      img.setF(x-x1,y-y1, get_s(dPoint(x*d.x,y*d.y)));
    }
  }

  std::map<double, dMultiLine> ret = image_cnt(img, val, val, 1.0, 1, vtol);
  if (ret.size()==0) return dMultiLine();

  for (auto & r:ret) r.second = (dPoint(x1,y1)+r.second)*d;
  return ret.begin()->second;
}


std::map<dPoint, short>
SRTM::find_peaks(const dRect & range, int DH, size_t PS){

  // integer rectangle covering the area
  dPoint d = get_step(range.cnt());
  iRect irange = ceil(range/d);
  int x1  = irange.tlc().x;
  int x2  = irange.brc().x;
  int y1  = irange.tlc().y;
  int y2  = irange.brc().y;

  // Summit finder:
  // 1. Find all local maxima with altitude h0 (they can contain multiple points).
  // 2. From each of them build a set of points by adding the highest point
  //    of the set boundary.
  // 3. If altitude of the last added point is less then h0-DH, or if
  //    set size is larger then PS, then stop the calculation and
  //    say that the original point is a summit.
  // 4. If altitude of the last added point is more then h0, then
  //    the original point is not a summit.

  std::set<iPoint> done;
  std::map<dPoint, short> ret;
  for (int y=y2; y>y1; y--){
    for (int x=x1; x<x2-1; x++){

      iPoint p(x,y);
      if (done.count(p)>0) continue;
      short h0 = get_h(dPoint(x,y)*d);
      if (h0 < SRTM_VAL_MIN) continue;

      std::set<iPoint> pts, brd;
      add_set_and_border(p, pts, brd);
      do{
        // find maximum of the border
        short max = SRTM_VAL_UNDEF;
        iPoint maxpt;
        for (auto const & b:brd){
          short h1 = get_h((dPoint)b*d);
          // original point is too close to data edge
          if ((h1<SRTM_VAL_MIN) && (dist(b,p)<1.5)) {max = h1; break;}
          if (h1>max) {max = h1; maxpt=b;}
        }
        if (max < SRTM_VAL_MIN) break;

        // if max is higher then original point:
        if (max > h0) { break; }

        // if we descended more then DH or covered area more then PS:
        if ((h0 - max > DH ) || (pts.size() > PS)) {
          ret[dPoint(p)*d] = h0;
          break;
        }
        add_set_and_border(maxpt, pts, brd);
        done.insert(maxpt);
      } while (true);
    }
  }
  return ret;
}

dMultiLine
SRTM::find_holes(const dRect & range){

  // integer rectangle covering the area
  dPoint d = get_step(range.cnt());
  iRect irange = ceil(range/d);
  int x1  = irange.tlc().x;
  int x2  = irange.brc().x;
  int y1  = irange.tlc().y;
  int y2  = irange.brc().y;

  std::set<iPoint> set;
  for (int y=y2; y>y1; y--){
    for (int x=x1; x<x2-1; x++){
      short h = get_h(dPoint(x,y));
      if (h!=SRTM_VAL_UNDEF) continue;
      set.insert(dPoint(x*d.x, y*d.y));
    }
  }
  // convert points to polygons
  dMultiLine ret = border_line(set);
  return (ret - dPoint(0.5,0.5))*d;
}
