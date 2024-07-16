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
#include "image/io_tiff.h"
#include "image_cnt/image_cnt.h"
#include <zlib.h>
#include <tiffio.h>

/**********************************************************/
// load srtm data from *.hgt file
ImageR
read_hgt_file(const std::string & file){
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
read_zhgt_file(const std::string & file){
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
read_demtif_file(const std::string & file){
  FILE *F = fopen(file.c_str(), "rb");
  if (!F) return ImageR();
  fclose(F);

  try {
    ImageR img = image_load_tiff(file);
    if (img.type() != IMAGE_16)
      throw Err() << "srtm: 16-bit tiff image expected";
    return img;

  }
  catch (Err & e) {
    throw Err() << "srtm: " << file << ": " << e.str();
  }
}

/**********************************************************/
// load SRTM tile
SRTMTile::SRTMTile(const std::string & dir, const iPoint & key_){
  key = key_;
  w = 0; h = 0;
  empty = true;
  srtm = false;

  if ((key.x < -180) || (key.x >= 180) ||
      (key.y <  -90) || (key.y >=  90)) return;

  char EW = key.x<0 ? 'W':'E';
  char NS = key.y<0 ? 'S':'N';

  // create filename
  std::ostringstream file;
  file << NS << std::setfill('0') << std::setw(2) << abs(key.y)
       << EW << std::setw(3) << abs(key.x);

  ImageR im;

  // try <name>.hgt.gz
  im= read_zhgt_file(dir + "/" + file.str() + ".hgt.gz");

  // try <name>.hgt
  if (im.is_empty())
    im = read_hgt_file(dir + "/" + file.str() + ".hgt");

  // try <name>.tif
  if (im.is_empty())
    im = read_demtif_file(dir + "/" + file.str() + ".tif");

  if (im.is_empty())
    im = read_demtif_file(dir + "/" + file.str() + ".tiff");

  if (im.is_empty())
    std::cerr << "SRTM: can't find file: " << file.str() << "\n";

  if (im.is_empty()) return;

  // TODO: read overlay

  ImageR::operator=(im);
  w = im.width();
  h = im.height();
  srtm = (w%2 == 1);

  empty = !(w > 2 && h > 1);
  if (!empty) step = srtm? dPoint(1.0/(w-1), 1.0/(h-1)) : dPoint(1.0/w, 1.0/h);
}

/**********************************************************/

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
  return get_tile(floor(p)).step;
}

// Low-level get function: rounding coordinate to the nearest point
double
SRTM::get_nearest(const dPoint& p){
  auto & tile = get_tile(floor(p));
  if (tile.is_empty()) return SRTM_VAL_NOFILE;

  // Pixel coordinate, [0..1200) for srtm,  [-0.5 .. 1199.5) for alos
  dPoint px = rint(tile.ll2px(p));
  if (px.x<0) px.x=0;
  if (px.y<0) px.y=0;
  if (px.x>=tile.w) px.x=tile.w-1;
  if (px.y>=tile.h) px.y=tile.h-1;
  return tile.get_unsafe(px);
}

void
SRTM::get_interp_pts(const iPoint key, const dPoint & p, std::set<dPoint> & pts){
  auto & tile = get_tile(key);
  if (tile.empty) return;

  // Pixel coordinate (could be outside the image)
  dPoint px = tile.ll2px(p);
  int x1 = floor(px.x), x2 = x1+1;
  int y1 = floor(px.y), y2 = y1+1;

  // side tiles
  if (x1 < 0) { x1 = 0; x2=-1; }
  if (x2 >= (int)tile.w) { x2 = (int)tile.w-1; x1 = -1; }
  if (y1 < 0) { y1 = 0; y2=-1; }
  if (y2 >= (int)tile.h) { y2 = (int)tile.h-1; y1 = -1; }

  // now coords are either valid or -1
  if (x1!=-1 && y1!=-1){
    dPoint pt = tile.px2ll(dPoint(x1,y1));
    pt.z = tile.get_unsafe(iPoint(x1,y1));
    if (pt.z>=SRTM_VAL_MIN) pts.insert(pt);
  }
  if (x1!=-1 && y2!=-1){
    dPoint pt = tile.px2ll(dPoint(x1,y2));
    pt.z = tile.get_unsafe(iPoint(x1,y2));
    if (pt.z>=SRTM_VAL_MIN) pts.insert(pt);
  }
  if (x2!=-1 && y1!=-1){
    dPoint pt = tile.px2ll(dPoint(x2,y1));
    pt.z = tile.get_unsafe(iPoint(x2,y1));
    if (pt.z>=SRTM_VAL_MIN) pts.insert(pt);
  }
  if (x2!=-1 && y2!=-1){
    dPoint pt = tile.px2ll(dPoint(x2,y2));
    pt.z = tile.get_unsafe(iPoint(x2,y2));
    if (pt.z>=SRTM_VAL_MIN) pts.insert(pt);
  }
  return;
}

// Bilinear interpolation
double
SRTM::get_interp(const dPoint& p){
  auto & tile = get_tile(floor(p));
  if (tile.is_empty()) return SRTM_VAL_NOFILE;

  // Pixel coordinate, [0..1200)
  dPoint px = tile.ll2px(p);
  int x1 = floor(px.x), x2 = x1+1;
  int y1 = floor(px.y), y2 = y1+1;

  // This should always work for SRTM.
  // But ALOS has 0.5 pixel shift and 1 pixel smaller image,
  // coordinate could be outside the image on any side.
  if (x1>0 && y1>0 && x2<tile.w && y2<tile.h){
    double sx = px.x - x1;
    double sy = px.y - y1;
    double h1 = tile.get_unsafe(iPoint(x1, y1));
    double h2 = tile.get_unsafe(iPoint(x1, y2));
    if ((h1<SRTM_VAL_MIN)||(h2<SRTM_VAL_MIN)) return SRTM_VAL_UNDEF;

    double h3 = tile.get_unsafe(iPoint(x2, y1));
    double h4 = tile.get_unsafe(iPoint(x2, y2));
    if ((h3<SRTM_VAL_MIN)||(h4<SRTM_VAL_MIN)) return SRTM_VAL_UNDEF;

    return h1*(1-sx)*(1-sy) + h2*(1-sx)*sy + h3*sx*(1-sy) + h4*sy*sx;
  }
  else {
    // collect points from adjecent tiles
    std::set<dPoint> pts;
    dPoint pa = tile.px2ll(dPoint(x1,y1));
    dPoint pb = tile.px2ll(dPoint(x2,y2));
    for (int kx = floor(pa.x); kx<=floor(pb.x); kx++){
      for (int ky = floor(pb.y); ky<=floor(pa.y); ky++){
        get_interp_pts(iPoint(kx,ky), p, pts);
      }
    }

    // We want to do something close to bilinear interpolation.
    // In most cases we should have 4 points (unless some tiles are missing).
    if (pts.size()!=4) return SRTM_VAL_UNDEF;
    dPoint p1,p2,p3,p4;
    p1 = p2 = p3 = p4 = *pts.begin();

    // sort points
    for (const auto & pp:pts){
      if (p1.x + p1.y < pp.x + pp.y) p1 = pp; // max(x+y), trc
      if (p2.x - p2.y < pp.x - pp.y) p2 = pp; // max(x-y), brc
      if (p3.x - p3.y > pp.x - pp.y) p3 = pp; // min(x-y), tlc
      if (p4.x + p4.y > pp.x + pp.y) p4 = pp; // min(x+y), blc
    }

    dPoint p12 = p1.y==p2.y? (p1+p2)/2 : p1 + (p2-p1)*(p.y - p1.y)/(p2.y-p1.y);
    dPoint p34 = p3.y==p4.y? (p3+p4)/2 : p3 + (p4-p3)*(p.y - p3.y)/(p4.y-p3.y);
    dPoint p1234 = p12.x==p34.x? (p12+p34)/2 : p12 + (p34-p12)*(p.x - p12.x)/(p34.x-p12.x);
    return (int16_t)p1234.z;
  }
  return SRTM_VAL_UNDEF;
}

// Get with interpolation
double
SRTM::get_h(const dPoint& p, bool raw){
  if (raw || srtm_interp == SRTM_NEAREST) return get_nearest(p);
  if (srtm_interp == SRTM_LINEAR) { return get_interp(p); }
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

ImageR
SRTM::get_img(const dRect & rng, dPoint & blc, dPoint & step){
  iPoint key0 = floor(rng.cnt());
  // main tile - we want to copy it, because it can be removed from the cache
  auto tile0 = get_tile(key0);
  if (tile0.is_empty()) return ImageR();
  iPoint c1 = floor(tile0.ll2px(rng.tlc()));
  iPoint c2 = ceil(tile0.ll2px(rng.brc()));
  std::swap(c1.y, c2.y);
  step = tile0.step;
  blc = c1;

  ImageR img(c2.x-c1.x, c2.y-c1.y, IMAGE_16);
  // render main tile
  for (int y=c1.y; y<c2.y; y++){
    for (int x=c1.x; x<c2.x; x++){
      if (x>0 && y>0 && x<tile0.w && y<tile0.h) {
        img.set16(x-c1.x, y-c1.y, tile0.get_unsafe(iPoint(x,y)));
      }
      else {
        dPoint pt = tile0.px2ll(dPoint(x,y));
        int16_t c = (int16_t)get_interp(pt);
        if (c<SRTM_VAL_MIN) c=0;
        img.set16(x-c1.x, y-c1.y, c);
      }
    }
  }
  return img;
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

  dPoint d = get_step(range.cnt());

  // integer rectangle covering the area
  iRect irange = ceil(range/d);

  int x1  = irange.tlc().x;
  int x2  = irange.brc().x;
  int y1  = irange.tlc().y;
  int y2  = irange.brc().y;

  std::set<iPoint> set;
  for (int y=y1; y<=y2; y++){
    for (int x=x1; x<=x2; x++){
      dPoint p(x*d.x,y*d.y);
      auto h = get_h(p);
      set.emplace(rint(p));
    }
  }

  // convert points to polygons
  dMultiLine ret = border_line(set);
  return (ret - dPoint(0.5,0.5))*d;
}
