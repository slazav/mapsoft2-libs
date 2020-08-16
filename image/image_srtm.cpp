#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include "image_srtm.h"
#include <zlib.h>

// load srtm data from *.hgt file
ImageR
read_file(const std::string & file, const size_t srtm_width){
  FILE *F = fopen(file.c_str(), "rb");
  if (!F) return ImageR();

  ImageR im(srtm_width,srtm_width, IMAGE_16);
  int length = srtm_width*srtm_width*sizeof(short);

  if (length != fread(im.data(), 1, length, F)){
    throw Err() << "ImageSRTM: bad .hgt file: " << file;
    return ImageR();
  }
  for (int i=0; i<length/2; i++){ // swap bytes
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

  if (length != gzread(F, im.data(), length)){
    throw Err() << "ImageSRTM: bad .hgt.gz file: " << file;
    return ImageR();
  }
  for (int i=0; i<length/2; i++){ // swap bytes
    uint16_t tmp=((uint16_t*)im.data())[i];
    ((uint16_t*)im.data())[i] = (tmp >> 8) + (tmp << 8);
  }
  gzclose(F);
  return im;
}

bool
ImageSRTM::load(const iPoint & key){

  if ((key.x < -180) || (key.x >= 180) ||
      (key.y <  -90) || (key.y >=  90)) return false;

  char EW = key.x<0 ? 'W':'E';
  char NS = key.y<0 ? 'S':'N';

  // create filename
  std::ostringstream file;
  file << NS << std::setfill('0') << std::setw(2) << abs(key.y)
       << EW << std::setw(3) << abs(key.x) << ".hgt";

  // try <name>.hgt.gz
  ImageR im = read_zfile(srtm_dir + "/" + file.str() + ".gz", srtm_width);

  // try <name>.hgt
  if (im.is_empty())
    im = read_file(srtm_dir + "/" + file.str(), srtm_width);

  if (im.is_empty())
    std::cerr << "ImageSRTM: can't find file: " << file.str() << "\n";

  srtm_cache.add(key, im);
  return !im.is_empty();
}


/************************************************/

ImageSRTM::ImageSRTM(const Opt & o): srtm_cache(SRTM_CACHE_SIZE), srtm_width(0) {
  set_opt(o);
}

void
ImageSRTM::set_opt(const Opt & opt){

  // Data directory. Default: $HOME/.srtm_data
  std::string dir = opt.get("srtm_dir",
    std::string(getenv("HOME")? getenv("HOME"):"") + "/.srtm_data");

  // Data width. If no option is set, read a number from
  // srtm_width.txt file. Default 1201.
  int width = opt.get<int>("srtm_width", 0);
  if (width <= 0) {
    std::ifstream ws(srtm_dir + "/srtm_width.txt");
    if (!ws) width = 1201;
    else ws >> width;
    if (width<=0) width = 1201;
  }

  // set new values and clear data cache if needed
  if (width!=srtm_width || dir!=srtm_dir){
    auto lk = std::unique_lock<std::mutex>(cache_mutex);
    srtm_width = width;
    srtm_dir = dir;
    size0 = 6380e3 * M_PI/srtm_width/180;
    area0 = pow(6380e3 * M_PI/srtm_width/180, 2);
    srtm_cache.clear();
  }

  // srtm drawing mode:
  auto m = opt.get("srtm_draw_mode", "default");
  if      (m == "none"){
    draw_mode = SRTM_DRAW_NONE;
  }
  else if (m == "heights" || m == "default") {
    draw_mode = (m == "heights")? SRTM_DRAW_HEIGHTS : SRTM_DRAW_DEFAULT;
    R = Rainbow(
      opt.get("srtm_hmin", 0.0),
      opt.get("srtm_hmax", 5000.0),
      RAINBOW_NORMAL);
  }
  else if (m == "slopes"){
    draw_mode = SRTM_DRAW_SLOPES;
    R = Rainbow(
      opt.get("srtm_smin", 30.0),
      opt.get("srtm_smax", 55.0),
      RAINBOW_BURNING);
  }
  else throw Err() << "unknown value of srtm_draw_mode parameter "
    << "(none, heights, or slopes expected): " << m;

}

// find tile number and coordinate on the tile
inline void
get_crd(int x, int w, int &k, int &c){
  if (x>=0) k=x/(w-1);
  else      k=(x+1)/(w-1)-1;
  c = x - k*(w-1);
}

short
ImageSRTM::get_val(const int x, const int y, const bool interp){

  // find tile number and coordinate on the tile
  iPoint key, crd;
  get_crd(x, srtm_width, key.x, crd.x);
  get_crd(y, srtm_width, key.y, crd.y);
  crd.y = srtm_width-crd.y-1;

  int h;
  {
    auto lk = std::unique_lock<std::mutex>(cache_mutex);
    if ((!srtm_cache.contains(key)) && (!load(key))) return SRTM_VAL_NOFILE;

    auto im = srtm_cache.get(key);
    if (im.is_empty()) return SRTM_VAL_NOFILE;
    h = im.get16(crd.x, crd.y);
  }

/*
  if (interp && h != SRTM_VAL_NOFILE){
    if (h>SRTM_INT_MIN) return h - SRTM_INT_ZERO;
    if (h!=SRTM_VAL_UNDEF) return h;

    // find hole and make interpolation
    set<iPoint> pset = plane(p);
    set<iPoint> bord = border(pset);

    set<iPoint>::iterator pi, bi;
    for (pi = pset.begin(); pi != pset.end(); pi++){
      double Srh = 0;
      double Sr  = 0;
      for (bi = bord.begin(); bi != bord.end(); bi++){
        int bh = geth(*bi);

        if (bh>SRTM_VAL_MIN){
          double k = cos(double(pi->y)/srtm_width/180.0*M_PI);
          double r = 1.0/(pow(k*(bi->x - pi->x),2) + pow(double(bi->y - pi->y),2));
          Sr += r;
          Srh+= bh * r;
        }
      }
      seth(*pi, (short)Srh/Sr + SRTM_INT_ZERO);
    }
    return geth(p,true);
  }
  else {
*/
    if (h>SRTM_INT_MIN) return SRTM_VAL_UNDEF;
    else return h;
/*
  }
*/

  return h;
}

double
ImageSRTM::get_slope(const int x, const int y, const bool interp){
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
