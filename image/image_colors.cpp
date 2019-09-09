#include "image_colors.h"
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>

#include <fstream>

enum methodForLargest {LARGE_NORM, LARGE_LUM};
enum methodForRep {REP_CENTER_BOX, REP_AVERAGE_COLORS, REP_AVERAGE_PIXELS};
enum methodForSplit {SPLIT_MAX_PIXELS, SPLIT_MAX_DIM, SPLIT_MAX_COLORS};


/**********************************************************/
// Create a colormap.
// Based on pnmcolormap.c from netpbm package.
std::vector<uint32_t>
image_colormap(const Image & img, const Opt & opt){

  // parameters
  int  req_colors = opt.get("cmap_colors", 256);
  if (req_colors < 0) throw Err() << "image_colormap: bad "
     " cmap_colors value: " << opt.get("cmap_colors", "");

  std::string str;

  methodForLargest method_for_largest = LARGE_NORM;
  str = opt.get("cmap_dim_method", "norm");
  if      (str == "norm")  method_for_largest = LARGE_NORM;
  else if (str == "lumin") method_for_largest = LARGE_LUM;
  else throw Err() << "image_colormap: unknown "
                      "cmap_dim_method value: " << str;

  methodForRep method_for_rep = REP_AVERAGE_PIXELS;
  str = opt.get("cmap_rep_method", "meanpix");
  if      (str == "center")  method_for_rep = REP_CENTER_BOX;
  else if (str == "meanpix") method_for_rep = REP_AVERAGE_PIXELS;
  else if (str == "meancol") method_for_rep = REP_AVERAGE_COLORS;
  else throw Err() << "image_colormap: unknown "
                      "cmap_rep_method value: " << str;

  methodForSplit method_for_split = SPLIT_MAX_DIM;
  str = opt.get("cmap_split_method", "maxdim");
  if      (str == "maxdim")  method_for_split = SPLIT_MAX_DIM;
  else if (str == "maxpix") method_for_split = SPLIT_MAX_PIXELS;
  else if (str == "maxcol") method_for_split = SPLIT_MAX_COLORS;
  else throw Err() << "image_colormap: unknown value "
                      "for cmap_split_method parameter: " << str;

  int transp_mode = 1;
  str = opt.get("cmap_alpha", "none");
  if      (str == "full") transp_mode = 0;
  else if (str == "none") transp_mode = 1;
  else if (str == "gif")  transp_mode = 2;
  else throw Err() << "image_colormap: unknown value "
                      "for cmap_alpha parameter: " << str;

  // make vector with a single box with the whole image histogram
  struct box_t {
    std::map<uint32_t, uint64_t> hist;
    int64_t pixels;
    int maxdim;
    double maxspread;
    box_t(): pixels(0), maxdim(-1), maxspread(0) {}
  };
  std::vector<box_t> bv;
  bv.push_back(box_t());
  bv[0].pixels = img.width()*img.height();

  // compute the histogram
  for (int y=0; y<img.height(); ++y){
    for (int x=0; x<img.width(); ++x){
      uint32_t c;
      // no transparency
      if (transp_mode == 1 ||
          img.type() == IMAGE_24RGB ||
          img.type() == IMAGE_16 ||
          img.type() == IMAGE_8 ||
          img.type() == IMAGE_1)
        c = img.get_rgb(x,y);
      else
        c = img.get_argb(x,y);
      if (transp_mode==2)
        c = color_rem_transp(c, 1);

      if (bv[0].hist.count(c) == 0) bv[0].hist[c] = 1;
      else ++bv[0].hist[c];
    }
  }

  // if we want all colors OR
  // if we want more colors then we have in the image
  // THEN make and return full colormap
  if (req_colors < 1 || bv[0].hist.size() < req_colors) {
    std::vector<uint32_t> ret;
    for (auto const & c:bv[0].hist) ret.push_back(c.first);
    return ret;
  }

  // component weights
  double weight[4]; // b-r-g-a!
  switch (method_for_largest){
    case LARGE_NORM:
      weight[0]=weight[1]=weight[2]=weight[3] = 1;
      break;
    case LARGE_LUM:
      weight[0] = COLOR_LUMINB;
      weight[1] = COLOR_LUMING;
      weight[2] = COLOR_LUMINR;
      weight[3] = 1;
      break;
  }

  // Step 2: median cut
  while (bv.size() < req_colors){

    // Calculate max spread and max dimension for each box if needed
    for (int i=0; i<bv.size(); ++i){
      if (bv[i].maxdim>0) continue;

      bv[i].maxspread=0;
      for (int pl = 0; pl<4; ++pl){
        uint8_t minv = 0xFF, maxv = 0;
        for (auto const & c:bv[i].hist){
          uint8_t v = (c.first >> (pl*8)) & 0xFF;
          if (minv > v) minv = v;
          if (maxv < v) maxv = v;
        }
        double spread = (maxv-minv) * weight[pl];
        if (bv[i].maxspread < spread){
          bv[i].maxspread = spread; bv[i].maxdim = pl;
        }
      }
    }

    // Find the largest box for splitting.
    // The box should contain at least two colors.
    int bi=-1; // box index
    {
      int64_t bs=0;
      double max = 0;
      for (int i=0; i<bv.size(); ++i){
        if (bv[i].hist.size() < 2) continue;

        switch (method_for_split) {
          case SPLIT_MAX_DIM:
            // find maximum by maxdim spread
            if (bv[i].maxspread>max) {max=bv[i].maxspread; bi=i;}
            break;
          case SPLIT_MAX_PIXELS:
            // find maximum by number of pixels
            if (bs < bv[i].pixels) { bs = bv[i].pixels; bi=i;}
          case SPLIT_MAX_COLORS:
           // find maximum by number of colors
           if (bs < bv[i].hist.size()) { bs = bv[i].hist.size(); bi=i;}
        }
      }
    }
    if (bi<0) break; // nothing to split


    assert(bv[bi].maxdim>=0 && bv[bi].maxdim < 4);

    // Sort the histogram using component with the
    // largest spread.
    // Here I use additional map, this needs more
    // memory then in the original code where sorting
    // is done in the histogram.
    std::map<uint8_t, uint64_t> dim_hist;
    for (auto const & c:bv[bi].hist){
      uint8_t key = (c.first >> (bv[bi].maxdim*8)) & 0xFF;
      if (dim_hist.count(key) == 0) dim_hist[key] = c.second;
      else dim_hist[key]+=c.second;
    }

    // should be at least two colors in the largest dimension
    assert(dim_hist.size()>1);

    // Find median point. It should not be the first point.
    uint64_t sum = 0;
    uint8_t median = 0;
    for (auto c = dim_hist.begin(); c!=dim_hist.end(); ++c){
      sum += c->second;
      if (c==dim_hist.begin()) continue; // not the first point
      if (sum < bv[bi].pixels/2) continue;
      median = c->first;
      break;
    }

    // note: for 2-color boxes with pixel distribution {1000,1} {1,1000}
    // median will be different, but we should split them in the same way!
    // We want to split the box before the median value unless it is the
    // first value.

    // Split boxes
    // Transfer all points which are above median to a new box
    box_t newbox; newbox.pixels = 0;
    auto it = bv[bi].hist.begin();
    uint64_t count=0;
    while (it != bv[bi].hist.end()){
      uint8_t key = (it->first >> (bv[bi].maxdim*8)) & 0xFF;
      if (key<median) {++it; continue;}
      newbox.hist[it->first] = it->second;
      newbox.pixels += it->second;
      bv[bi].pixels -= it->second;
      it = bv[bi].hist.erase(it);
    }
    bv[bi].maxdim = -1;
    bv[bi].maxspread = 0;
    bv.push_back(newbox);

    assert(bv[bi].pixels > 0);
    assert(newbox.pixels > 0);

/*
    std::cerr << "Split box: "
              << "  pixels: " << bv[bi].pixels
              << " + " << newbox.pixels << "\n"
              << "  colors: " << bv[bi].hist.size()
              << " + " << newbox.hist.size() << "\n";
*/
  }


  // Make colormap from the box vector
  std::vector<uint32_t> ret;
  for (auto const & b:bv){
    uint8_t buf[4];

    for (int pl = 0; pl < 4; pl++){
      uint64_t sum_pix=0, sum_col=0;
      int min=0xFF, max=0;
      for (auto const & c:b.hist){
        uint8_t v = (c.first >> (pl*8)) & 0xFF;
        sum_col += v;
        sum_pix += v*c.second;
        if (v<min) min = v;
        if (v>max) max = v;
      }

      switch (method_for_rep) {
        case REP_CENTER_BOX:     buf[pl] = (max+min)/2; break;
        case REP_AVERAGE_COLORS: buf[pl] = sum_col/b.hist.size(); break;
        case REP_AVERAGE_PIXELS: buf[pl] = sum_pix/b.pixels; break;
      }
    }
    ret.push_back(buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24));
  }
  return ret;
}


// Reduce number of colors
Image
image_remap(const Image & img, const std::vector<uint32_t> & cmap, const Opt & opt){

  std::string str;
  int transp_mode = 1;
  str = opt.get("cmap_alpha", "none");
  if      (str == "full") transp_mode = 0;
  else if (str == "none") transp_mode = 1;
  else if (str == "gif")  transp_mode = 2;
  else throw Err() << "image_remap: unknown value "
                      "for cmap_alpha parameter: " << str;

  // we return 8bpp image, palette length should be 1..256
  if (cmap.size() < 1 || cmap.size() > 256)
    throw Err() << "image_remap: palette length is out of range";

  // Construct the new image
  Image img1(img.width(), img.height(), IMAGE_8PAL);
  for (int y=0; y<img.height(); ++y){
    for (int x=0; x<img.width(); ++x){
      // get color
      uint32_t c;
      // no transparency
      if (transp_mode == 1 ||
          img.type() == IMAGE_24RGB ||
          img.type() == IMAGE_16 ||
          img.type() == IMAGE_8 ||
          img.type() == IMAGE_1)
        c = img.get_rgb(x,y);
      else
        c = img.get_argb(x,y);
      if (transp_mode==2)
        c = color_rem_transp(c, 1);

      // find nearest palette value
      double d0 = +HUGE_VAL;
      int i0 = 0;
      for (int i=0; i<cmap.size(); ++i){
        double d = color_dist(c, cmap[i]);
        if (d0>d) {d0=d; i0 = i;}
      }
      img1.set8(x,y,i0);
    }
  }

  // fill image colormap
  img1.cmap = cmap;

  return img1;
}

/***********************************************************/

int
image_classify_alpha(const Image & img){
  int ret=0;
  if (img.type() != IMAGE_32ARGB) throw Err() <<
    "image_classify: only 32-bpp images are supported";

  for (int y=0; y<img.height(); y++){
    for (int x=0; x<img.width(); x++){
      uint32_t c = img.get32(x, y);
      int a = (c >> 24) & 0xFF;

      if (a>0 && a<255) {ret=2; break;}
      if (a==0) ret=1;
    }
  }
  return ret;
}

int
image_classify_color(const Image & img, uint32_t *colors, int clen){
  int ret=0;
  if (img.type() != IMAGE_32ARGB &&
      img.type() != IMAGE_24RGB) throw Err() <<
    "image_classify: only 32-bpp images are supported";

  int nc=0; // number of colors
  for (int i=0; i<clen; ++i) colors[i]=0;

  for (int y=0; y<img.height(); ++y){
    for (int x=0; x<img.width(); ++x){
      uint32_t c = img.get_argb(x, y);
      int r = (c >> 16) & 0xFF;
      int g = (c >> 8) & 0xFF;
      int b = c & 0xFF;

      if (r!=g || r!=b) ret=1;

      if (nc<=clen){
        bool found=false;
        for (int i=0; i<nc; i++)
          if (c==colors[i]){ found=true; break;}
        if (!found){
          if (nc<clen) colors[nc] = c;
          nc++;
        }
      }

    }
  }
  if (nc>clen) ret+=1;
  return ret;
}