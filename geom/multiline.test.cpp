///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "multiline.h"
#include "opt/opt.h"
#include "err/assert_err.h"

int
main(){
  try{

  iMultiLine ml1;
  iLine l1("[[0,1],[2,3]]");
  iLine l2("[[4,5],[6,7]]");
  iMultiLine ml2;
  ml2.push_back(l1);
  ml2.push_back(l2);
  assert_eq(ml1.size(), 0);
  assert_eq(ml2.size(), 2);
  assert_eq(ml1.npts(), 0);
  assert_eq(ml2.npts(), 4);

  assert_eq(ml2, iMultiLine("[[[0,1],[2,3]],[[4,5],[6,7]]]"));
  assert_eq(ml1, iMultiLine("[]"));

  assert_eq(-ml2, -iMultiLine("[[[0,1],[2,3]],[[4,5],[6,7]]]"));
  assert_eq(-ml2,  iMultiLine("[[[0,-1],[-2,-3]],[[-4,-5],[-6,-7]]]"));

  // reading single-segment multiline:
  assert_eq(iMultiLine("[[1,1],[2,2]]"), iMultiLine("[[[1,1],[2,2]]]"));
  assert(iMultiLine("[[]]") != iMultiLine("[]")); // exception

  // +,-,*,/
  iPoint p(1,2);
  assert_eq((ml2+p), iMultiLine("[[[1,3],[3,5]],[[5,7],[7,9]]]"));
  assert_eq((ml2-p), iMultiLine("[[[-1,-1],[1,1]],[[3,3],[5,5]]]"));
  assert_eq((ml2*2), iMultiLine("[[[0,2],[4,6]],[[8,10],[12,14]]]"));
  assert_eq((ml2/2), iMultiLine("[[[0,0],[1,1]],[[2,2],[3,3]]]"));

  assert_eq((ml2+=p), iMultiLine("[[[1,3],[3,5]],[[5,7],[7,9]]]"));
  assert_eq((ml2-=p), iMultiLine("[[[0,1],[2,3]],[[4,5],[6,7]]]"));
  assert_eq((ml2*=2), iMultiLine("[[[0,2],[4,6]],[[8,10],[12,14]]]"));
  assert_eq((ml2/=2), iMultiLine("[[[0,1],[2,3]],[[4,5],[6,7]]]"));

  assert_eq(iMultiLine("[[],[[10,20,30],[1,2],[1,0,0],[0,0,2]]]") * iPoint(1,2,3),
            iMultiLine("[[],[[10,40,90],[1,4],[1,0,0],[0,0,6]]]"));
  assert_eq(iPoint(1,2,3)*iMultiLine("[[],[[10,20,30],[1,2],[1,0,0],[0,0,2]]]"),
            iMultiLine("[[],[[10,40,90],[1,4],[1,0,0],[0,0,6]]]"));
  assert_eq(iMultiLine("[[],[[10,20,30],[1,4],[1,0,0],[0,0,6]]]") / iPoint(1,2,3),
            iMultiLine("[[],[[10,10,10],[1,2],[1,0,0],[0,0,2]]]"));

  // <=>
  assert_eq(ml1, iMultiLine());
  ml1=ml2; // Multilines are equal
  assert_eq(  ml1,ml2);
  assert(!(ml1!=ml2));
  assert(ml1 >= ml2);
  assert(ml1 <= ml2);
  assert(!(ml1 > ml2));
  assert(!(ml1 < ml2));
  assert(ml1.operator==(ml2));
  assert(!(ml1.operator!=(ml2)));
  assert(ml1.operator>=(ml2));
  assert(ml1.operator<=(ml2));
  assert(!(ml1.operator>(ml2)));
  assert(!(ml1.operator<(ml2)));
  *(ml1.rbegin()->rbegin()) = iPoint(10,10); // Last segment of ml1 shorter
  assert(ml1 !=  ml2);
  assert(ml1 >  ml2);
  assert(ml1 >= ml2);
  assert(ml2 <  ml1);
  assert(ml2 <= ml1);
  ml1.rbegin()->resize(ml1.rbegin()->size()-1); // ml2 shorter
  assert(ml1 !=  ml2);
  assert(ml1 <  ml2);
  assert(ml1 <= ml2);
  assert(ml2 >  ml1);
  assert(ml2 >= ml1);
  ml2.resize(ml2.size()-1);
  assert(ml1 !=  ml2);
  assert(ml1 >  ml2);
  assert(ml1 >= ml2);
  assert(ml2 <  ml1);
  assert(ml2 <= ml1);

  // npts, length, bbox, rint
  ml2.clear();
  ml1.clear();
  ml2.push_back(l1);
  ml2.push_back(l2);

  assert_eq(ml1.length(), 0);
  assert_feq(ml2.length(), l1.length() + l2.length(), 1e-4);
  assert_feq(length(ml2), length(l1) + length(l2), 1e-4);

  assert_eq(ml1.npts(), 0);
  assert_eq(ml2.npts(), l1.npts() + l2.npts());
  assert_eq(npts(ml2), npts(l1) + npts(l2));

  assert_eq(ml1.bbox(), iRect());
  assert_eq(ml2.bbox(), expand(l1.bbox(),l2.bbox()));
  assert_eq(bbox(ml2), ml2.bbox());

  assert_eq(dMultiLine("[]").is_empty(), true);
  assert_eq(dMultiLine("[[],[]]").is_empty(), true);
  assert_eq(dMultiLine("[[],[[1,2]]]").is_empty(), false);
  assert_eq(dMultiLine("[[[1,2]]]").is_empty(), false);

  assert_eq(dMultiLine("[]").npts(), 0);
  assert_eq(dMultiLine("[[],[]]").npts(), 0);
  assert_eq(dMultiLine("[[],[[1,2]]]").npts(), 1);
  assert_eq(dMultiLine("[[[1,2]]]").npts(), 1);
  assert_eq(dMultiLine("[[[1,2],[3,4]]]").npts(), 2);
  assert_eq(dMultiLine("[[[1,2],[1,2,3]], [[1,2],[3,4],[5,6]], []]").npts(), 5);

  {
     dMultiLine ml1("[ [[0,0,0], [1,2,2]], [[0,0,0], [1,2,-2], [2,0,0]] ]");
     assert_eq(ml1.length(), 9);

     dMultiLine ml2("[ [[0,0,0], [3,4,2]], [[0,0,0], [3,4,-2], [6,0,0]] ]");
     assert_eq(ml2.length2d(), 15);
  }

  assert_eq(rint(dMultiLine("[[[1.1,1.8],[3.9,1.1]],[]]")), dMultiLine("[[[1,2],[4,1]],[]]"));
  assert_eq(flatten(iMultiLine("[[[1,8,9],[1,2,3]],[]]")), iMultiLine("[[[1,8],[1,2]],[]]"));

  // rotate2d
  { 
    iMultiLine l, l0("[[[0,0,10],[1000,0,5]], [], [[0,0]]]");
    iPoint c1(0,0), c2(500,500);
    iMultiLine lr1("[[[0,0,10],[866,-499,5]], [], [[0,0]]]");
    iMultiLine lr2("[[[-183,316,10],[683,-183,5]], [], [[-183,316]]]");

    double a=-30*M_PI/180.0;
    l=l0;
    assert_eq(rotate2d(l,c1,a), lr1);
    assert_eq(rotate2d(l,c2,a), lr2);
    assert_eq(l,l0);
    l.rotate2d(c1,a);
    assert_eq(l,lr1);
    l=l0;
    l.rotate2d(c2,a);
    assert_eq(l,lr2);
  }

  // flatten, rint, floor, ceil, abs
  {
    dMultiLine l, l0("[[[0.1,2.8,3.1],[-0.1,-3.9,-4.6]], [], [[0,0]]]");
    dMultiLine li("[[[0,3,3],[0,-4,-5]], [], [[0,0]]]");
    dMultiLine lc("[[[1,3,4],[0,-3,-4]], [], [[0,0]]]");
    dMultiLine lf("[[[0,2,3],[-1,-4,-5]], [], [[0,0]]]");
    dMultiLine lz("[[[0.1,2.8],[-0.1,-3.9]], [], [[0,0]]]");
    dMultiLine la("[[[0.1,2.8,3.1],[0.1,3.9,4.6]], [], [[0,0]]]");

    l=l0;
    assert_deq(flatten(l), lz, 1e-6);
    assert_deq(l,l0, 1e-6);
    l.flatten();
    assert_deq(l,lz, 1e-6);

    l=l0;
    assert_eq(rint(l), iMultiLine(li));
    assert_deq(l,l0, 1e-6);
    l.to_rint();
    assert_deq(l,li, 1e-6);

    l=l0;
    assert_eq(floor(l), iMultiLine(lf));
    assert_deq(l,l0, 1e-6);
    l.to_floor();
    assert_deq(l,lf, 1e-6);

    l=l0;
    assert_eq(ceil(l), iMultiLine(lc));
    assert_deq(l,l0, 1e-6);
    l.to_ceil();
    assert_deq(l,lc, 1e-6);

    l=l0;
    assert_deq(abs(l), la, 1e-6);
    assert_deq(l,l0, 1e-6);
    l.to_abs();
    assert_deq(l,la, 1e-6);

  }

  // open/close
  {
    dMultiLine l("[[[1,2],[2,3],[4,5]],[[1,1]],[],[[1,1],[2,2]]]");
    assert_deq(open(l), l, 1e-6);
    assert_deq(close(l), dMultiLine("[[[1,2],[2,3],[4,5],[1,2]],[[1,1]],[],[[1,1],[2,2],[1,1]]]"), 1e-6);
    assert_deq(close(close(l)), close(l), 1e-6);
    assert_deq(open(close(l)), l, 1e-6);
    assert_deq(close(open(l)), close(l), 1e-6);
  }

  // flip_x, flip_y
  {
    iMultiLine l1("[[[1,1],[2,2]],[[1,1],[2,2]]]");
    iMultiLine ly("[[[1,9],[2,8]],[[1,9],[2,8]]]");
    iMultiLine lx("[[[9,1],[8,2]],[[9,1],[8,2]]]");
    assert_eq(flip_y(l1,10), ly);
    assert_eq(flip_x(l1,10), lx);
    assert_eq(flip_y(flip_y(l1,10),10), l1);
    assert_eq(flip_x(flip_x(l1,10),10), l1);
    l1.flip_x(10);
    assert_eq(l1, lx);
    l1.flip_x(10);
    l1.flip_y(10);
    assert_eq(l1, ly);
  }

  // dist
  assert_deq(
    dMultiLine("[ [[0,0],[1,1],[2,2]], [[1,1]], [] ]"),
    dMultiLine("[ [[0,0],[1,1],[2,2]], [[1,1]], [] ]"), 1e-6);
  assert(dist(
    dMultiLine("[]"),
    dMultiLine("[]")) == 0);
  assert(dist(
    dMultiLine("[[[0,0],[1,1],[2,2]],[]]"),
    dMultiLine("[[[0,0],[1,1]],[]]")) == INFINITY);
  assert(dist(
    dMultiLine("[[[0,0],[1,1]]]"),
    dMultiLine("[[[0,0],[1,1]],[]]")) == INFINITY);
  assert(dist(
    dMultiLine("[[[0,0],[1,1]],[]]"),
    dMultiLine("[[[0,0],[1,1]]]")) == INFINITY);
  assert_feq(dist(
    dMultiLine("[ [[0,0],[1,4],[2,2]], [[1,1]], []]"),
    dMultiLine("[ [[0,0],[1,1],[2,2]], [[1,5]], []]")), 5, 1e-6);

  // iLine <-> dLine casting
  assert_eq(dMultiLine(str_to_type<iMultiLine>("[[[0,0],[2,0],[2,2]],[]]")),
                    str_to_type<dMultiLine>("[[[0,0],[2,0],[2,2]],[]]"));
  assert_eq(iMultiLine(str_to_type<dMultiLine>("[[[0.8,0.2],[2.1,0.2],[2.2,2.9]],[]]")),
                    str_to_type<iMultiLine>("[[[0,0],[2,0],[2,2]],[]]"));

  // input/output
  assert_eq(type_to_str(str_to_type<iMultiLine>("[[[0,0],[2,0],[2,2]],[]]")),
         "[[[0,0],[2,0],[2,2]],[]]");

  assert_eq(type_to_str(str_to_type<iMultiLine>("[[[0,0],[2,0],[2,2]],[]] ")),
         "[[[0,0],[2,0],[2,2]],[]]");

  assert_eq(type_to_str(str_to_type<iMultiLine>("[]")),
         "[]");

  assert_eq(type_to_str(str_to_type<iMultiLine>("[[]]")),
         "[[]]");
  assert_eq(type_to_str(str_to_type<iMultiLine>("[[],[]]")),
         "[[],[]]");

  assert_err(
    str_to_type<iMultiLine>("[[[0,0],[2,0],[2,2]]]a"),
    "can't parse multisegment line: \"[[[0,0],[2,0],[2,2]]]a\": end of file expected near 'a'");

  assert_err(
    str_to_type<iMultiLine>("[[[0,0],[2,0],[2,2"),
    "can't parse multisegment line: \"[[[0,0],[2,0],[2,2\": ']' expected near end of file");

  assert_err(
    str_to_type<iMultiLine>("[0,0],[2,0],[2,2]"),
    "can't parse multisegment line: \"[0,0],[2,0],[2,2]\": end of file expected near ','");

  // add_point(), add_segment(), del_last_point(), get_first
  {
    iMultiLine ml;
    ml.add_point(iPoint(1,1));
    ml.add_point(iPoint(1,2));
    ml.add_segment();
    ml.add_point(iPoint(2,1));
    ml.add_point(iPoint(2,2));
    ml.add_segment();
    ml.add_segment();
    ml.add_point(iPoint(4,1));
    assert_eq(ml.size(), 4);
    assert_eq(ml.npts(), 5);
    assert_eq(ml, iMultiLine("[[[1,1],[1,2]],[[2,1],[2,2]],[],[[4,1]]]"));
    ml.del_last_point();
    assert_eq(ml, iMultiLine("[[[1,1],[1,2]],[[2,1],[2,2]]]"));
    ml.del_last_point();
    assert_eq(ml, iMultiLine("[[[1,1],[1,2]],[[2,1]]]"));
    ml.del_last_point();
    assert_eq(ml, iMultiLine("[[[1,1],[1,2]]]"));
    ml.del_last_point();
    assert_eq(ml, iMultiLine("[[[1,1]]]"));
    ml.del_last_point();
    assert_eq(ml, iMultiLine());

    ml.add_segment();
    assert_eq(ml.size(), 1);
    assert_eq(ml.npts(), 0);
    assert_eq(ml, iMultiLine("[[]]"));

    assert_eq(iMultiLine("[[1,1]]").get_first_pt(), iPoint("[1,1]"));
    assert_eq(iMultiLine("[[], [[1,1]]]").get_first_pt(), iPoint("[1,1]"));
    assert_eq(iMultiLine("[[[1,1],[1,2]], [[2,1]]]").get_first_pt(), iPoint("[1,1]"));
    assert_err(iMultiLine().get_first_pt(), "dMultiLine::get_first: no points");

  }

  }


  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond