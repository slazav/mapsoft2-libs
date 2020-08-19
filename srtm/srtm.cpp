#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include "srtm.h"
#include "geom/point_int.h"
#include <zlib.h>

// load srtm data from *.hgt file
ImageR
read_file(const std::string & file, const size_t srtm_width){
  FILE *F = fopen(file.c_str(), "rb");
  if (!F) return ImageR();

  ImageR im(srtm_width,srtm_width, IMAGE_16);
  int length = srtm_width*srtm_width*sizeof(short);

  if (length != fread(im.data(), 1, length, F)){
    throw Err() << "SRTM: bad .hgt file: " << file;
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
    throw Err() << "SRTM: bad .hgt.gz file: " << file;
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
SRTM::load(const iPoint & key){

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
    std::cerr << "SRTM: can't find file: " << file.str() << "\n";

  srtm_cache.add(key, im);
  return !im.is_empty();
}


/************************************************/

SRTM::SRTM(const Opt & o): srtm_cache(SRTM_CACHE_SIZE), srtm_width(0) {
  set_opt(o);
}

void
SRTM::set_opt(const Opt & opt){

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
}

Opt
SRTM::get_opt() const{
  Opt o;
  o.put("srtm_dir", srtm_dir);
  // do not return srtm_width, we want to use autodetection.
  return o;
}


/************************************************/


// Find set of points with same value (used
// for hole interpolation in get_val) and its border.
void
SRTM::plane_and_border(const iPoint& p,
     std::set<iPoint>& set, std::set<iPoint>& brd, int max){

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

// Find tile number and coordinate on the tile
// (used in get_val/set_val).
inline void
get_crd(int x, int w, int &k, int &c){
  if (x>=0) k=x/(w-1);
  else      k=(x+1)/(w-1)-1;
  c = x - k*(w-1);
}

short
SRTM::get_val(const int x, const int y, const bool interp){
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
SRTM::get_val_int4(const dPoint & p, const bool interp){
  double x = p.x*(srtm_width-1);
  double y = p.y*(srtm_width-1);
  int x1 = floor(x), x2 = x1+1;
  int y1 = floor(y), y2 = y1+1;

  short h1=get_val(x1,y1,interp);
  short h2=get_val(x1,y2,interp);

  if ((h1<SRTM_VAL_MIN)||(h2<SRTM_VAL_MIN)) return SRTM_VAL_UNDEF;
  short h12 = (int)( h1+ (h2-h1)*(y-y1) );

  short h3=get_val(x2,y1,interp);
  short h4=get_val(x2,y2,interp);
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
SRTM::get_val_int16(const dPoint & p, const bool interp){
  double x = p.x*(srtm_width-1);
  double y = p.y*(srtm_width-1);
  int x0 = floor(x);
  int y0 = floor(y);

  double hx[4], hy[4];

  for (int i=0; i<4; i++){
    for (int j=0; j<4; j++) hx[j]=get_val(x0+j-1, y0+i-1, interp);
    int_holes(hx);
    hy[i]= cubic_interp(hx, x-x0);
  }
  int_holes(hy);
  return cubic_interp(hy, y-y0);
}

/************************************************/

short
SRTM::set_val(const int x, const int y, const short h){
  // find tile number and coordinate on the tile
  iPoint key, crd;
  get_crd(x, srtm_width, key.x, crd.x);
  get_crd(y, srtm_width, key.y, crd.y);
  crd.y = srtm_width-crd.y-1;

  auto lk = std::unique_lock<std::mutex>(cache_mutex);
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
SRTM::get_slope_int4(const dPoint & p, const bool interp){
  double x = p.x*(srtm_width-1);
  double y = p.y*(srtm_width-1);
  int x1 = floor(x), x2 = x1+1;
  int y1 = floor(y), y2 = y1+1;

  double h1=get_slope(x1,y1, interp);
  double h2=get_slope(x1,y2, interp);
  double h3=get_slope(x2,y1, interp);
  double h4=get_slope(x2,y2, interp);

  double h12 = h1+ (h2-h1)*(y-y1);
  double h34 = h3 + (h4-h3)*(y-y1);
  return h12 + (h34-h12)*(x-x1);
}

/************************************************/

// Coordinates of 4 data cell corners
iPoint crn (int k){ k%=4; return iPoint(k/2, (k%3>0)?1:0); }

// Directions of 4 data cell sides
iPoint dir (int k){ return crn(k+1)-crn(k); }

std::map<short, dMultiLine>
SRTM::find_contours(const dRect & range, int step){
  int w = get_srtm_width();
  int x1  = int(floor((w-1)*range.tlc().x));
  int x2  = int( ceil((w-1)*range.brc().x));
  int y1  = int(floor((w-1)*range.tlc().y));
  int y2  = int( ceil((w-1)*range.brc().y));

  std::map<short, dMultiLine> ret;
  int count = 0;
  for (int y=y2; y>y1; y--){
    for (int x=x1; x<x2; x++){

      iPoint p(x,y);
      // Crossing of all 4 data cell sides with contours
      // (coordinate v along the 4-segment line).
      // Add -0.1m to avoid crossings at corners.
      std::multimap<short, double> pts;

      for (int k=0; k<4; k++){
        iPoint p1 = p+crn(k);
        iPoint p2 = p+crn(k+1);
        short h1 = get_val(p1.x, p1.y);
        short h2 = get_val(p2.x, p2.y);
        if ((h1<SRTM_VAL_MIN) || (h2<SRTM_VAL_MIN)) continue;
        int min = (h1<h2)? h1:h2;
        int max = (h1<h2)? h2:h1;
        min = int( floor(double(min)/step)) * step;
        max = int( ceil(double(max)/step))  * step;
        if (h2==h1) continue;
        for (int hh = min; hh<=max; hh+=step){
          double v = double(hh-h1+0.1)/double(h2-h1);
          if ((v<0)||(v>1)) continue;
          pts.insert(std::pair<short, double>(hh,v+k));
        }
      }
      // Put contours which are crossing the data cell twice to `ret`.
      short h=SRTM_VAL_UNDEF;
      double v1,v2;

      for (auto const & i:pts){
        if (h!=i.first){
          h  = i.first;
          v1 = i.second;
        } else{
          v2 = i.second;
          // convert v coordinates to points:
          dPoint p1=(dPoint(p + crn(int(v1))) + dPoint(dir(int(v1)))*double(v1-int(v1)))/(double)(w-1);
          dPoint p2=(dPoint(p + crn(int(v2))) + dPoint(dir(int(v2)))*double(v2-int(v2)))/(double)(w-1);

          // We found segment p1-p2 with height h
          // first try to append it to existing line in ret[h]
          bool done=false;
          for (auto & l:ret[h]){
            int e=l.size()-1;
            if (e<=0) continue; // we have no 1pt lines!
            if (dist(l[0], p1) < 1e-4){ l.insert(l.begin(), p2); done=true; break;}
            if (dist(l[0], p2) < 1e-4){ l.insert(l.begin(), p1); done=true; break;}
            if (dist(l[e], p1) < 1e-4){ l.push_back(p2); done=true; break;}
            if (dist(l[e], p2) < 1e-4){ l.push_back(p1); done=true; break;}
          }
          if (!done){ // insert new line into ret[h]
            dLine hor;
            hor.push_back(p1);
            hor.push_back(p2);
            ret[h].push_back(hor);
          }
          h=SRTM_VAL_UNDEF;
          count++;
        }
      }
    }
  }

  // merge contours (similar code is in point_int.cpp/border_line)
  double e = 1e-4;
  for(auto & d:ret){
    for (auto i1 = d.second.begin(); i1!=d.second.end(); i1++){
      for (auto i2 = i1+1; i2!=d.second.end(); i2++){
        dLine tmp;
        if      (dist(*(i1->begin()),*(i2->begin()))<e){
          tmp.insert(tmp.end(), i1->rbegin(), i1->rend());
          tmp.insert(tmp.end(), i2->begin()+1, i2->end());
        }
        else if (dist(*(i1->begin()),*(i2->rbegin()))<e){
          tmp.insert(tmp.end(), i1->rbegin(), i1->rend());
          tmp.insert(tmp.end(), i2->rbegin()+1, i2->rend());
        }
        else if (dist(*(i1->rbegin()),*(i2->begin()))<e){
          tmp.insert(tmp.end(), i1->begin(), i1->end());
          tmp.insert(tmp.end(), i2->begin()+1, i2->end());
        }
        else if (dist(*(i1->rbegin()),*(i2->rbegin()))<e){
          tmp.insert(tmp.end(), i1->begin(), i1->end());
          tmp.insert(tmp.end(), i2->rbegin()+1, i2->rend());
        }
        else continue;
        i1->swap(tmp);
        d.second.erase(i2);
        i2=i1;
      }
    }
  }

  return ret;
}

std::map<dPoint, short>
SRTM::find_peaks(const dRect & range, int DH, int PS){
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
      dPoint p1 = dPoint(x,y)/(double)(w-1);
      if (h!=SRTM_VAL_UNDEF) continue;
      set.insert(iPoint(x,y));
    }
  }
  // convert points to polygons
  dMultiLine ret = border_line(set);
  return (ret - dPoint(0.5,0.5))/(double)(w-1);
}
