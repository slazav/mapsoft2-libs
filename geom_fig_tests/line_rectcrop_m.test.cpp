#include <string>
#include <fstream>
#include <iostream>
#include "fig/fig.h"
#include "geom/line_rectcrop.h"

using namespace std;

// Read objects from FIG file:
// * lines and polygons with depth 50 are objects to be cut,
// * polygons with depth 40 are cutting objects.
// Write output file with added result at depth 30.

int
main(int argc, char **argv){
  try{

  if (argc!=3) throw Err() << "usage: crop.test <in_file.fig> <out_file.fig>";

  const char *in_file  = argv[1];
  const char *out_file = argv[2];

  Fig F;
  read_fig(in_file, F);

  dRect cutter;
  // cutter rectangle: depth 50
  for (const auto & o:F) {
    if (o.type == FIG_POLYLINE &&
        o.sub_type==2 && o.depth==50)
      cutter = o.bbox();
  }

  dMultiLine mlo, mlc;
  for (const auto & o:F) {
    if (o.type != FIG_POLYLINE ||
       (o.sub_type!=1 && o.sub_type!=3))
      continue;
    bool closed = (o.sub_type==3);
    if (closed) mlc.push_back(o);
    else mlo.push_back(o);
  }

  mlo = rect_crop_multi(cutter, mlo, false);
  mlc = rect_crop_multi(cutter, mlc, true);

  Fig F1;
  FigObj o1 = figobj_template("2 1 0 3 16 7 20 -1 -1 0.000 0 0 7 0 0");
  o1.open();
  for (const auto & l:mlo){
    o1.clear();
    o1.set_points(l);
    if (o1.size()==0) continue;
    F1.push_back(o1);
  }
  o1.close();
  o1.pen_color = 0xFF0000;
  for (const auto & l:mlc){
    o1.clear();
    o1.set_points(l);
    if (o1.size()==0) continue;
    F1.push_back(o1);
  }

  F.insert(F.begin(), F1.begin(), F1.end());

  for (const auto & o:F1) {
    std::cerr << ">>> " << o.size() << "\n";
  }

  write_fig(out_file,F);
  return 0;

  }
  catch (Err & e){
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
}


