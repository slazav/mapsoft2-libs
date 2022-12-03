#include <string>
#include <fstream>
#include <iostream>
#include "fig/fig.h"
#include "geom/poly_tools.h"

using namespace std;

// Read objects from FIG file:
// * lines and polygons with depth 50 are objects to be cut,
// * polygons with depth 40 are cutting objects.
// Write output file with added result at depth 30.

int
main(int argc, char **argv){
  try{

  if (argc!=3) throw Err() << "usage: line_filter_v1 <in_file.fig> <out_file.fig>";

  const char *in_file  = argv[1];
  const char *out_file = argv[2];

  Fig F;
  read_fig(in_file, F);
  double e = 10.; // Fig units

  Fig F1;
  for (const auto & o:F) {
    if (o.type != FIG_POLYLINE ||
       (o.sub_type!=1 && o.sub_type!=3))
      continue;

    FigObj o1 = figobj_template("2 1 0 3 16 7 200 -1 -1 0.000 0 0 7 0 0");
    iLine pts(o);
    line_filter_v1(pts, e, -1);
    if (pts.size()==0) continue;
    o1.set_points(pts);
    F1.push_back(o1);
  }

  F.insert(F.begin(), F1.begin(), F1.end());
  write_fig(out_file,F);
  return 0;

  }
  catch (Err & e){
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
}


