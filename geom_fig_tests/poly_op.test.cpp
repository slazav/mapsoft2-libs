#include <cassert>
#include <fstream>
#include <iostream>
#include "fig/fig.h"
#include "geom/poly_op.h"

using namespace std;

int
main(int argc, char **argv){
  try {

  if (argc!=3) throw Err() << "usage: poly_op.test <in_file.fig> <out_file.fig>";
  const char *in_file  = argv[1];
  const char *out_file = argv[2];

  Fig F;
  ifstream in(in_file);
  read_fig(in, F);

  int w = int(10.0*Fig::cm2fig);
  iRect cutter(0,0,w,w);

  // push all fig objects into single multiline
  dMultiLine ml1, ml2;
  Fig::const_iterator f;
  for (const auto & f: F){
    std::cerr << "color: " << f.pen_color << "\n";
    if (f.pen_color == 0x0000FF) ml1.push_back(f);
    if (f.pen_color == 0xFF0000) ml2.push_back(f);
  }

  FigObj o;
  read_figobj_header(o, "2 3 0 1 4 7 20 -1 -1 0.000 1 0 0 0 0 0");
  o.pen_color = 0x00FF00;
  o.depth     = 40;

  for (const auto & l1:ml1){
    for (const auto & l2:ml2){
//      for (const auto & l:poly_op(l1,l2, POLY_ADD)){
//        o.pen_color = 0x00FF00;
//        o.depth     = 40;
//        o.clear();
//        o.set_points(l);
//        F.push_back(o);
//      }
//      for (const auto & l:poly_op(l1,l2, POLY_SUB)){
      auto l = join_polygons(poly_op(l1,l2, POLY_SUB));
      if (l.size()){
        o.pen_color = 0x00FFFF;
        o.depth     = 30;
        o.clear();
        o.set_points(l);
        F.push_back(o);
      }
    }
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


