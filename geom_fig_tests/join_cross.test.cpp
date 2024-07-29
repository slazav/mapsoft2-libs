#include <cassert>
#include <fstream>
#include <iostream>
#include "fig/fig.h"
#include "geom/poly_tools.h"

using namespace std;

int
main(int argc, char **argv){
  try {

  if (argc!=3) throw Err() << "usage: poly_cross.test <in_file.fig> <out_file.fig>";
  const char *in_file  = argv[1];
  const char *out_file = argv[2];

  Fig F;
  ifstream in(in_file);
  read_fig(in, F);

  int w = int(10.0*Fig::cm2fig);
  iRect cutter(0,0,w,w);

  // push all fig objects into single multiline
  dMultiLine ml;
  for (const auto & f: F) ml.push_back(f);
  join_cross(ml);


  FigObj o;
  read_figobj_header(o, "2 3 0 1 4 7 20 -1 -1 0.000 1 0 0 0 0 0");

  for (const auto & l:ml){
    o.clear();
    o.set_points(l);
    F.push_back(o);
  }

  ofstream out(out_file);
  write_fig(out,F);

  return 0;
  }
  catch (Err & e){
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
}


