#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <list>
#include "srtm.h"
#include "geom/line.h"
#include "geom/point_int.h"
#include "filename/filename.h"
#include <zlib.h>
#include <tiffio.h>


// load srtm data from *.hgt file
ImageR
read_file(const std::string & file, const size_t srtm_width){
  FILE *F = fopen(file.c_str(), "rb");
  if (!F) return ImageR();

  ImageR im(srtm_width,srtm_width, IMAGE_16);
  size_t length = srtm_width*srtm_width*sizeof(short);

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
read_zfile(const std::string & file, const size_t srtm_width){
  gzFile F = gzopen(file.c_str(), "rb");
  if (!F) return ImageR();

  ImageR im(srtm_width, srtm_width, IMAGE_16);
  int length = srtm_width*srtm_width*sizeof(short);

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
read_tfile(const std::string & file, const size_t srtm_width){

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

  ImageR im(srtm_width, srtm_width, IMAGE_16);

  try {

    // check format: should be 16bpp greyscale
    int photometric=0;
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
    if (photometric != PHOTOMETRIC_MINISBLACK)
      throw Err() << "unsupported photometric type: " << photometric;
    if (bpp != 2)
      throw Err() << "not a 2 byte-per-pixel tiff";

    // check size: for srtm width w it should be one of
    // (w)x(w), (w-1)x(w-1), ((w-1)/2+1)x(w), ((w-1)/2)x(w-1)
    bool addpixel;
    int xsc=1;
    if (tiff_h == (int)srtm_width-1) {
      addpixel = true;
      if (tiff_w == (int)srtm_width-1) xsc = 1;
      else if (tiff_w == ((int)srtm_width-1)/2) xsc = 2;
      else if (tiff_w == ((int)srtm_width-1)/3) xsc = 3;
      else if (tiff_w == ((int)srtm_width-1)/4) xsc = 4;
      else throw Err() << "bad image size: " << tiff_w << "x" << tiff_h;
    }
    else if (tiff_h == (int)srtm_width) {
      addpixel = false;
      if (tiff_w == (int)srtm_width) xsc = 1;
      else if (tiff_w == ((int)srtm_width-1)/2+1) xsc = 2;
      else if (tiff_w == ((int)srtm_width-1)/3+1) xsc = 3;
      else if (tiff_w == ((int)srtm_width-1)/4+1) xsc = 4;
      else throw Err() << "bad image size: " << tiff_w << "x" << tiff_h;
    }
    else throw Err() << "bad image size: " << tiff_w << "x" << tiff_h;

    // read image
    for (int y = 0; y<tiff_h; y++){
      TIFFReadScanline(tif, cbuf, y);
      for (int x = 0; x<tiff_w; x++){
        uint16_t v = (cbuf[2*x+1]<<8) + cbuf[2*x];

        if (xsc > 1){
          im.set16(xsc*x,  y,v);
          if (xsc*x+1<(int)srtm_width) im.set16(xsc*x+1,y,v);
        }
        else {
          im.set16(x,y,v);
        }
      }
      if (addpixel){
        im.set16(srtm_width-1,y, im.get16(0,y));
      }
    }
    if (addpixel){
      for (int x = 0; x<(int)srtm_width; x++)
        im.set16(x, srtm_width-1, im.get16(x,0));
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
  ImageR im = read_zfile(srtm_dir + "/" + file.str() + ".hgt.gz", srtm_width);

  // try <name>.hgt
  if (im.is_empty())
    im = read_file(srtm_dir + "/" + file.str() + ".hgt", srtm_width);

  // try <name>.tif
  if (im.is_empty())
    im = read_tfile(srtm_dir + "/" + file.str() + ".tif", srtm_width);

  if (im.is_empty())
    im = read_tfile(srtm_dir + "/" + file.str() + ".tiff", srtm_width);

  if (im.is_empty())
    std::cerr << "SRTM: can't find file: " << file.str() << "\n";

  srtm_cache.add(key, im);
  return !im.is_empty();
}


/************************************************/

SRTM::SRTM(const Opt & o): srtm_width(0), srtm_cache(SRTM_CACHE_SIZE) {
  set_opt(o);
}


void
ms2opt_add_srtm(GetOptSet & opts){
  const char *g = "SRTM";
  opts.add("srtm_dir", 1,0,g,
    "Set srtm data folder, default - $HOME/.srtm_data");
  opts.add("srtm_interp_holes", 1,0,g,
    "Interpolate holes (0|1, default 1).");
}

Opt
SRTM::get_def_opt() {
  Opt o;
  o.put("srtm_dir", std::string(getenv("HOME")? getenv("HOME"):"") + "/.srtm_data");
  o.put("srtm_interp_holes", 1);
  return o;
}

void
SRTM::set_opt(const Opt & opt){

  // Data directory. Default: $HOME/.srtm_data
  std::string dir = opt.get("srtm_dir",
    std::string(getenv("HOME")? getenv("HOME"):"") + "/.srtm_data");

  // Data width. Read a number from
  // srtm_width.txt file. Default 1201.
  size_t width = 1201;
  std::ifstream ws(dir + "/srtm_width.txt");
  ws >> width;
  if (width<=0) width = 1201;

  // set new values and clear data cache if needed
  if (width!=srtm_width || dir!=srtm_dir){
    srtm_width = width;
    srtm_dir = dir;
    size0 = 6380e3 * M_PI/srtm_width/180;
    area0 = pow(6380e3 * M_PI/srtm_width/180, 2);
    srtm_cache.clear();
  }

}


/************************************************/


// Find set of points with same value (used
// for hole interpolation in get_val) and its border.
void
SRTM::plane_and_border(const iPoint& p,
     std::set<iPoint>& set, std::set<iPoint>& brd, size_t max){

  std::queue<iPoint> q;
  short h = get_val(p.x,p.y);

  add_set_and_border(p, set,brd);
  q.push(p);

  while (!q.empty()){
    iPoint p1 = q.front();
    q.pop();
    for (int i=0; i<8; i++){
      iPoint p2 = adjacent(p1, i);
      if ((get_val(p2.x, p2.y, false) == h)&&
          add_set_and_border(p2, set, brd)) q.push(p2);
    }
    if ((max!=0)&&(set.size()>max)) break;
  }
}

/*
   -1     0      1      2deg
 1 |------|------|------|
   |      |      |      |
   |      |      |      |
   |      |      |      |
   |      |      |      |
 0 |------0------|------|
   |      |1     |      |
   |      | 2    |      |
   |      |  ..  |      |
   |      |   w-2|      |
-1 |------|------|------|

srtm_width=1201 mean 1200 point period!

*/

// Find tile number and coordinate on the tile
// (used in get_val/set_val).
inline void
get_crd(int x, int w, int &k, int &c, bool inv){
  auto ww=w-1;
  if (x>=0) k=x/ww;
  else      k=(x+1)/ww-1;
  c = x - k*ww;
  if (inv){
    if (c>0) c = ww - c;
    else k--;
  }
}

short
SRTM::get_val(const int x, const int y, const bool interp){
  // find tile number and coordinate on the tile
  iPoint key, crd;
  get_crd(x, srtm_width, key.x, crd.x, false);
  get_crd(y, srtm_width, key.y, crd.y, true);

  int h;
  {
    if ((!srtm_cache.contains(key)) && (!load(key))) return SRTM_VAL_NOFILE;
    auto im = srtm_cache.get(key);
    if (im.is_empty()) return SRTM_VAL_NOFILE;
    h = (int16_t)im.get16(crd.x, crd.y);
  }

  if (interp && h != SRTM_VAL_NOFILE){
    // already interpolated
    if (h>SRTM_INT_MIN) return h - SRTM_INT_ZERO;

    // no need to interpolate
    if (h!=SRTM_VAL_UNDEF) return h;

    // find hole and make interpolation
    std::set<iPoint> set, brd;
    plane_and_border(iPoint(x,y), set, brd, SRTM_MAX_INT_PTS);

    for (auto & p:set){
      double Srh = 0;
      double Sr  = 0;
      for (auto & b:brd){
        int bh = get_val(b.x, b.y, false);

        if (bh<SRTM_VAL_MIN) continue;
        double k = cos(double(p.y)/srtm_width/180.0*M_PI);
        dPoint dp = b-p;
        dp.x*=k;
        double r = 1.0/dp.len();
        Sr += r*r;
        Srh+= bh * r*r;
      }
      short v = Sr==0? 0:Srh/Sr;
      set_val(p.x, p.y, v + SRTM_INT_ZERO);
    }
    return get_val(x,y,true);
  }
  else {
    if (h>SRTM_INT_MIN) return SRTM_VAL_UNDEF;
    else return h;
  }
  return h;
}


short
SRTM::get_val_int4(const dPoint & p){
  double x = p.x*(srtm_width-1);
  double y = p.y*(srtm_width-1);
  int x1 = floor(x), x2 = x1+1;
  int y1 = floor(y), y2 = y1+1;

  short h1=get_val(x1,y1,interp_holes);
  short h2=get_val(x1,y2,interp_holes);

  if ((h1<SRTM_VAL_MIN)||(h2<SRTM_VAL_MIN)) return SRTM_VAL_UNDEF;
  short h12 = (int)( h1+ (h2-h1)*(y-y1) );

  short h3=get_val(x2,y1,interp_holes);
  short h4=get_val(x2,y2,interp_holes);
  if ((h3<SRTM_VAL_MIN)||(h4<SRTM_VAL_MIN)) return SRTM_VAL_UNDEF;
  short h34 = (int)( h3 + (h4-h3)*(y-y1) );
  return (short)( h12 + (h34-h12)*(x-x1) );
}

// Cubic interpolation (used in get_val_int16()).
// see http://www.paulinternet.nl/?page=bicubic
short
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


short
SRTM::get_val_int16(const dPoint & p){
  double x = p.x*(srtm_width-1);
  double y = p.y*(srtm_width-1);
  int x0 = floor(x);
  int y0 = floor(y);

  double hx[4], hy[4];

  for (int i=0; i<4; i++){
    for (int j=0; j<4; j++) hx[j]=get_val(x0+j-1, y0+i-1, interp_holes);
    int_holes(hx);
    hy[i]= cubic_interp(hx, x-x0);
  }
  int_holes(hy);
  return cubic_interp(hy, y-y0);
}

double
SRTM::get_val_smooth(const int x, const int y, const double r){
  if (r==0) return get_val(x,y, interp_holes);
  int ri = ceil(r);
  double v=0;
  double n = 0;
  for (int sx=-ri;sx<=ri;sx++){
    for (int sy=-ri;sy<=ri;sy++){
      auto h = get_val(x+sx,y+sy,interp_holes);
      if (h<SRTM_VAL_MIN) continue;
      double w = exp(-(sx*sx + sy*sy)/r);
      v += w*h;
      n += w;
    }
  }
  return v/n;
}

/************************************************/

short
SRTM::set_val(const int x, const int y, const short h){
  // find tile number and coordinate on the tile
  iPoint key, crd;
  get_crd(x, srtm_width, key.x, crd.x, false);
  get_crd(y, srtm_width, key.y, crd.y, true);

  if ((!srtm_cache.contains(key)) && (!load(key))) return SRTM_VAL_NOFILE;
  auto & im = srtm_cache.get(key);
  if (im.is_empty()) return SRTM_VAL_NOFILE;
  ((uint16_t*)im.data())[srtm_width*crd.y+crd.x] = h;
  return h;
}

/************************************************/

double
SRTM::get_slope(const int x, const int y, const bool interp){
  int h  = get_val(x,   y, interp);
  int h1 = get_val(x-1, y, interp);
  int h2 = get_val(x+1, y, interp);
  if (h1 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h2 > SRTM_VAL_MIN) h1 = 2*h - h2;
  if (h2 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h1 > SRTM_VAL_MIN) h1 = 2*h - h2;

  int h3 = get_val(x, y-1, interp);
  int h4 = get_val(x, y+1, interp);
  if (h3 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h4 > SRTM_VAL_MIN) h3 = 2*h - h4;
  if (h4 < SRTM_VAL_MIN && h > SRTM_VAL_MIN && h3 > SRTM_VAL_MIN) h4 = 2*h - h3;

  if (h1 > SRTM_VAL_MIN && h2 > SRTM_VAL_MIN && h3 > SRTM_VAL_MIN && h4 > SRTM_VAL_MIN){
    const double kx =  cos(M_PI*y/180.0/srtm_width);
    const double  U = hypot((h2-h1)/kx, h4-h3)/size0/2.0;
    return atan(U)*180.0/M_PI;
  }
  return 0;
}


double
SRTM::get_slope_int4(const dPoint & p){
  double x = p.x*(srtm_width-1);
  double y = p.y*(srtm_width-1);
  int x1 = floor(x), x2 = x1+1;
  int y1 = floor(y), y2 = y1+1;

  double h1=get_slope(x1,y1, interp_holes);
  double h2=get_slope(x1,y2, interp_holes);
  double h3=get_slope(x2,y1, interp_holes);
  double h4=get_slope(x2,y2, interp_holes);

  double h12 = h1+ (h2-h1)*(y-y1);
  double h34 = h3 + (h4-h3)*(y-y1);
  return h12 + (h34-h12)*(x-x1);
}

double
SRTM::get_slope_smooth(const int x, const int y, const double r){
  if (r==0) return get_slope(x,y, interp_holes);
  int ri = ceil(r);
  double v=0;
  double n = 0;
  for (int sx=-ri;sx<=ri;sx++){
    for (int sy=-ri;sy<=ri;sy++){
      auto h = get_slope(x+sx,y+sy,interp_holes);
      double w = exp(-(sx*sx + sy*sy)/r);
      v += w*h;
      n += w;
    }
  }
  return v/n;
}


/************************************************/

// Coordinates of 4 data cell corners: [0,0] [0,1] [1,1] [1,0]
iPoint crn (int k, int kx=1){ k%=4; return iPoint(kx*(k/2), (k%3>0)?1:0); }

// Directions of 4 data cell sides
iPoint dir (int k, int kx=1){ return crn(k+1, kx)-crn(k, kx); }

// Erase a key-value pair from multimap
void erase_kv (std::multimap<iPoint, iPoint> & mm, const iPoint & k, const iPoint &v){
  auto i = mm.lower_bound(k);
  while (i != mm.upper_bound(k)){
    if (i->second == v) i=mm.erase(i);
    else i++;
  }
}

// Merge contours.
// Sizes of pf and pb should be same, they should contain
// pairs p1->p2 and p2->p1 of coordinates multiplied by 1e6.
dMultiLine merge_cntr(std::multimap<iPoint, iPoint> & pf,
                      std::multimap<iPoint, iPoint> & pb){

  dMultiLine ret;
  while (pf.size()){
    iLine l;
    auto p1 = pf.begin()->first;
    auto p2 = pf.begin()->second;
    l.push_back(p1);
    l.push_back(p2);
    pf.erase(p1);
    pb.erase(p2);

    while (1){

     if (pf.count(p2)){
        auto p3 = pf.find(p2)->second;
        l.push_back(p3);
        erase_kv(pf, p2, p3);
        erase_kv(pb, p3, p2);
        p2 = p3;
      }

      else if (pb.count(p2)){
        auto p3 = pb.find(p2)->second;
        l.push_back(p3);
        erase_kv(pb, p2, p3);
        erase_kv(pf, p3, p2);
        p2 = p3;
      }

      else if (pf.count(p1)){
        auto p3 = pf.find(p1)->second;
        l.insert(l.begin(), p3);
        erase_kv(pf, p1, p3);
        erase_kv(pb, p3, p1);
        p1 = p3;
      }

      else if (pb.count(p1)){
        auto p3 = pb.find(p1)->second;
        l.insert(l.begin(), p3);
        erase_kv(pb, p1, p3);
        erase_kv(pf, p3, p1);
        p1 = p3;
      }

      else break;
    }
    if (l.size()) ret.push_back((dLine)l*1e-6);
  }
  return ret;
}


std::map<short, dMultiLine>
SRTM::find_contours(const dRect & range, int step, int kx, double smooth){

  int w = get_srtm_width();
  double E = 1e-3/w; // distance for merging

  if (kx<1) {
    if      (range.y>=70) kx = 3;
    else if (range.y>=60) kx = 2;
    else kx = 1;
  }

  // integer rectangle covering the area
  dRect drange = range*(w-1.0);
  drange.x /= kx; drange.w /= kx;
  iRect irange = ceil(drange);

  int x1  = irange.tlc().x;
  int x2  = irange.brc().x;
  int y1  = irange.tlc().y;
  int y2  = irange.brc().y;

  // Extract altitudes for the whole range, make image
std::cerr << "collect altitude data\n";
  ImageR img(x2-x1+1, y2-y1+1, IMAGE_16);
  for (int y=y1; y<=y2; y++){
    for (int x=x1; x<=x2; x++){
      img.set16(x-x1,y-y1, get_val(x*kx,y));
    }
  }

  // Smooth image if needed
  if (smooth!=0.0) {
std::cerr << "smooth data\n";
    int ri = ceil(smooth);
    ImageR img_s(x2-x1+1, y2-y1+1, IMAGE_16);
    for (int y=0; y<=y2-y1; y++){
      for (int x=0; x<=x2-x1; x++){
        double s0=0;
        double s1=0;
        for (int sx=-ri;sx<=ri;sx++){
          for (int sy=-ri;sy<=ri;sy++){
            if (!img.check_crd(x+sx, y+sy)) continue;
            double w = exp(-(sx*sx + sy*sy)/smooth);
            s0 += w;
            s1 += w*img.get16(x+sx,y+sy);
          }
        }
        img_s.set16(x,y,(uint16_t)(s1/s0));
      }
    }
    img = img_s;
  }


std::cerr << "find contours\n";
  std::map<short, std::multimap<iPoint, iPoint> > pf, pb;

  int count = 0;
  for (int y=y1; y<y2; y++){
    for (int x=x1; x<x2; x++){

      iPoint p(x,y);
      // Crossing of all 4 data cell sides with contours
      // (coordinate v along the 4-segment line).
      // Add -0.1m to avoid crossings at corners.
      std::multimap<short, dPoint> pts;

      for (int k=0; k<4; k++){
        iPoint p1 = p+crn(k);
        iPoint p2 = p+crn(k+1);
        auto h1 = img.get16(p1.x-x1, p1.y-y1);
        auto h2 = img.get16(p2.x-x1, p2.y-y1);

        if (h2==h1) continue;
        if ((h1<SRTM_VAL_MIN) || (h2<SRTM_VAL_MIN)) continue;
        int min = (h1<h2)? h1:h2;
        int max = (h1<h2)? h2:h1;
        min = int( floor(double(min)/step)) * step;
        max = int( ceil(double(max)/step))  * step;
        for (int hh = min; hh<=max; hh+=step){
          double v = double(hh-h1+0.1)/double(h2-h1);
          if ((v<0)||(v>1)) continue;
          dPoint cr = ((dPoint)p1 + (dPoint)(p2-p1)*v)/(w-1.0);
          cr.x*=kx;
          pts.emplace(hh, cr);
        }
      }
      // Put contours which are crossing the data cell 2 or 4 times to `ret`.
      short h=SRTM_VAL_UNDEF;
      dPoint p1, p2;

      for (auto const & i:pts){
        if (h!=i.first){
          h  = i.first;
          p1 = i.second;
          continue;
        }
        else {
          p2 = i.second;
          pf[h].emplace(p1*1e6, p2*1e6);
          pb[h].emplace(p2*1e6, p1*1e6);
          h = SRTM_VAL_UNDEF;
        }
      }
    }
  }

  // merge contours
  std::map<short, dMultiLine> ret;
  for(const auto & pp : pf){
    auto h = pp.first;
    ret[h] = merge_cntr(pf[h], pb[h]);
  }
  return ret;
}

dMultiLine
SRTM::find_slope_contours(const dRect & range, double val, int kx, double smooth){
  int w = get_srtm_width();
  double E = 1e-3/w; // distance for merging

  if (kx<1) {
    if      (range.y>=70) kx = 3;
    else if (range.y>=60) kx = 2;
    else kx = 1;
  }

  // integer rectangle covering the area
  dRect drange = range*(w-1.0);
  drange.x /= kx; drange.w /= kx;
  iRect irange = ceil(drange);
//  irange.x *= kx; irange.w *= kx;

  int x1  = irange.tlc().x;
  int x2  = irange.brc().x;
  int y1  = irange.tlc().y;
  int y2  = irange.brc().y;

  // Extract slopes for the whole range.
  // make image with slopes
std::cerr << "get slope data\n";
  ImageR img(x2-x1+1, y2-y1+1, IMAGE_FLOAT);
  for (int y=y1; y<=y2; y++){
    for (int x=x1; x<=x2; x++){
      img.setF(x-x1,y-y1,get_slope(x*kx,y));
    }
  }

  // Smooth slopes if needed
  if (smooth!=0.0) {
std::cerr << "smooth slope data\n";
    int ri = ceil(smooth);
    ImageR img_s(x2-x1+1, y2-y1+1, IMAGE_FLOAT);
    for (int y=0; y<=y2-y1; y++){
      for (int x=0; x<=x2-x1; x++){
        double s0=0;
        double s1=0;
        for (int sx=-ri;sx<=ri;sx++){
          for (int sy=-ri;sy<=ri;sy++){
            if (!img.check_crd(x+sx, y+sy)) continue;
            double v = img.getF(x+sx,y+sy);
            double w = exp(-(sx*sx + sy*sy)/smooth);
            s0 += w;
            s1 += w*v;
          }
        }
      img_s.setF(x,y,s1/s0);
      }
    }
    img = img_s;
  }

  // Find contours
  // Add one extra point on each side to put zero values there
  // and get closed contours.

  // Put each pair of points into two arrays
  // for both directions
  std::multimap<iPoint, iPoint> pf, pb;

std::cerr << "find slope contours\n";
  for (int y=y1-1; y<=y2; y++){
    for (int x=x1-1; x<=x2; x++){

      iPoint p(x,y);
      // Crossing of all 4 data cell sides with contours
      // (coordinate v along the 4-segment line).
      dLine pts;
      for (int k=0; k<4; k++){
        iPoint p1 = p+crn(k);
        iPoint p2 = p+crn(k+1);
        float v1 = 0, v2 = 0;
        if (p1.y>=y1 && p1.y<=y2 && p1.x>=x1 && p1.x<=x2)
          v1 = img.getF(p1.x-x1, p1.y-y1);

        if (p2.y>=y1 && p2.y<=y2 && p2.x>=x1 && p2.x<=x2)
          v2 = img.getF(p2.x-x1, p2.y-y1);

        float min = (v1<v2)? v1:v2;
        float max = (v1<v2)? v2:v1;
        if (min < val && max >= val){
          double v = (val-v1)/(v2-v1);
          dPoint cr = ((dPoint)p1 + (dPoint)(p2-p1)*v)/(w-1.0);
          cr.x*=kx;
          pts.push_back(cr);
        }
      }
      // Put contours which are crossing the data cell 2 or 4 times to `ret`.
      for (size_t i=1; i<pts.size(); i+=2){
        dPoint p1 = pts[i-1], p2 = pts[i];
        pf.emplace(1e6*p1, 1e6*p2);
        pb.emplace(1e6*p2, 1e6*p1);
      }
    }
  }

  std::cerr << "merge slope contours\n";
  auto ret = merge_cntr(pf, pb);
  return ret;
}


std::map<dPoint, short>
SRTM::find_peaks(const dRect & range, int DH, size_t PS){
  int w = get_srtm_width();
  int x1  = int(floor((w-1)*range.tlc().x));
  int x2  = int( ceil((w-1)*range.brc().x));
  int y1  = int(floor((w-1)*range.tlc().y));
  int y2  = int( ceil((w-1)*range.brc().y));

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
      short h0 = get_val(x,y, false);
      if (h0 < SRTM_VAL_MIN) continue;

      std::set<iPoint> pts, brd;
      add_set_and_border(p, pts, brd);
      do{
        // find maximum of the border
        short max = SRTM_VAL_UNDEF;
        iPoint maxpt;
        for (auto const & b:brd){
          short h1 = get_val(b.x, b.y, false);
          // original point is too close to data edge
          if ((h1<SRTM_VAL_MIN) && (dist(b,p)<1.5)) {max = h1; break;}
          if (h1>max) {max = h1; maxpt=b;}
        }
        if (max < SRTM_VAL_MIN) break;

        // if max is higher then original point:
        if (max > h0) { break; }

        // if we descended more then DH or covered area more then PS:
        if ((h0 - max > DH ) || (pts.size() > PS)) {
          ret[dPoint(p)/(double)(w-1)] = h0;
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
  int w = get_srtm_width();
  int x1  = int(floor((w-1)*range.tlc().x));
  int x2  = int( ceil((w-1)*range.brc().x));
  int y1  = int(floor((w-1)*range.tlc().y));
  int y2  = int( ceil((w-1)*range.brc().y));
  std::set<iPoint> set;
  for (int y=y2; y>y1; y--){
    for (int x=x1; x<x2-1; x++){
      short h = get_val(x,y,false);
      if (h!=SRTM_VAL_UNDEF) continue;
      set.insert(iPoint(x,y));
    }
  }
  // convert points to polygons
  dMultiLine ret = border_line(set);
  return (ret - dPoint(0.5,0.5))/(double)(w-1);
}
