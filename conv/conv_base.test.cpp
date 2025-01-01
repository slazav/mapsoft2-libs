///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "conv_base.h"

class MyConv : public ConvBase {
  public:
  void frw_pt(dPoint & p) const {
    if (sc_src.x!=1.0) p.x*=sc_src.x;
    if (sc_src.y!=1.0) p.y*=sc_src.y;
    if (sc_src.z!=1.0) p.y*=sc_src.z;
    p.x*=p.x;
    p.y*=2;
    if (sc_dst.x!=1.0) p.x*=sc_dst.x;
    if (sc_dst.y!=1.0) p.y*=sc_dst.y;
    if (sc_dst.z!=1.0) p.y*=sc_dst.z;
  }
  void bck_pt(dPoint & p) const {
    if (sc_dst.x!=1.0) p.x/=sc_dst.x;
    if (sc_dst.y!=1.0) p.y/=sc_dst.y;
    if (sc_dst.z!=1.0) p.y/=sc_dst.z;
    p.x=sqrt(p.x);
    p.y/=2;
    if (sc_src.x!=1.0) p.x/=sc_src.x;
    if (sc_src.y!=1.0) p.y/=sc_src.y;
    if (sc_src.z!=1.0) p.y/=sc_src.z;
  }
};

int
main(){
  try{

    ConvBase cnv0;

    dPoint p(10,10);
    cnv0.frw(p);  assert_eq(p, dPoint(10,10));
    cnv0.bck(p);  assert_eq(p, dPoint(10,10));
    assert_deq(cnv0.frw_pts(p), dPoint(10,10), 1e-8);
    assert_deq(cnv0.bck_pts(p), dPoint(10,10), 1e-8);

    cnv0.rescale_src(3);
    cnv0.frw(p); assert_eq(p, dPoint(30,30));
    cnv0.bck(p); assert_eq(p, dPoint(10,10));
    assert_deq(cnv0.frw_pts(p), dPoint(30,30),         1e-8);
    assert_deq(cnv0.bck_pts(p), dPoint(10/3.0,10/3.0), 1e-8);

    cnv0.rescale_dst(3);
    cnv0.frw(p);  assert_eq(p, dPoint(90,90));
    cnv0.bck(p);  assert_eq(p, dPoint(10,10));
    assert_deq(cnv0.frw_pts(p), dPoint(90,90),         1e-8);
    assert_deq(cnv0.bck_pts(p), dPoint(10/9.0,10/9.0), 1e-8);

    cnv0.rescale_dst(1/9.0);
    cnv0.frw(p);  assert_eq(p, dPoint(10,10));
    cnv0.bck(p);  assert_eq(p, dPoint(10,10));
    assert_deq(cnv0.frw_pts(p), dPoint(10,10), 1e-8);
    assert_deq(cnv0.bck_pts(p), dPoint(10,10), 1e-8);


    MyConv cnv;

    { // test frw and bck functions for Point
      dPoint p1(2,2);
      cnv.frw(p1);  assert_eq(p1, dPoint(4,4));
      cnv.frw(p1);  assert_eq(p1, dPoint(16,8));
      cnv.bck(p1);  assert_eq(p1, dPoint(4,4));
      cnv.bck(p1);  assert_eq(p1, dPoint(2,2));
    }

    { // test Line conversions
      dLine l1("[[0,0],[10,10]]");
      cnv.frw(l1);  assert_eq(l1, dLine("[[0,0],[100,20]]"));
      cnv.frw(l1);  assert_eq(l1, dLine("[[0,0],[10000,40]]"));
      cnv.bck(l1);  assert_eq(l1, dLine("[[0,0],[100,20]]"));
      cnv.bck(l1);  assert_eq(l1, dLine("[[0,0],[10,10]]"));
    }

    { // test frw_acc/bck_acc line conversions
       dLine l1("[[0,0],[10,10]]");
       cnv.rescale_dst(10);
       assert_eq(iLine(cnv.frw_acc(l1, 2)),
              iLine("[[0,0],[250,100],[1000,200]]"));
       assert_eq(iLine(cnv.frw_acc(l1, 1)),
              iLine("[[0,0],[62,50],[390,125],[1000,200]]"));
       assert_eq(iLine(cnv.frw_acc(l1, 0.08)),
              iLine("[[0,0],[0,6],[8,18],[22,29],[65,50],[121,69],[261,102],[401,126],[666,163],[1000,200]]"));

       l1=dLine("[[0,0],[100,20]]");
       assert_eq(iLine(10.0*cnv.bck_acc(l1, 1)),
              iLine("[[0,0],[31,10]]"));
       assert_eq(iLine(10.0*cnv.bck_acc(l1, 0.2)),
              iLine("[[0,0],[7,0],[23,5],[31,10]]"));
       assert_eq(iLine(10.0*cnv.bck_acc(l1, 0.05)),
              iLine("[[0,0],[1,0],[5,0],[9,0],[14,2],[20,4],[26,7],[31,10]]"));

       assert_eq(cnv.bck_acc(l1, 0.5), cnv.bck_acc(l1));

       cnv.rescale_dst(0.1);

     }
     { // test frw_acc/bck_acc multiline
       dLine l1("[[0,0],[10,10]]");
       dMultiLine ml1, ml2;
       ml1.push_back(l1);
       ml1.push_back(l1);

       ml2.push_back(cnv.bck_acc(l1,1));
       ml2.push_back(cnv.bck_acc(l1,1));
       assert_eq(cnv.bck_acc(ml1, 1), ml2);
    }

    { // test frw_acc/bck_acc line conversions
       dLine l1("[[0,0],[0,0],[10,10],[10,10,1]]");
       cnv.rescale_dst(10);
       assert_eq(iLine(cnv.frw_acc(l1, 2)),
              iLine("[[0,0],[250,100],[1000,200],[1000,200,1]]"));
       assert_eq(iLine(cnv.frw_acc(l1, 1)),
              iLine("[[0,0],[62,50],[390,125],[1000,200],[1000,200,1]]"));
       assert_eq(iLine(cnv.frw_acc(l1, 0.08)),
              iLine("[[0,0],[0,6],[8,18],[22,29],[65,50],[121,69],[261,102],[401,126],[666,163],[1000,200],[1000,200,1]]"));
       assert_eq(iLine(cnv.frw_acc(l1, 0)),
              iLine("[[0,0],[0,0],[1000,200],[1000,200,1]]"));

       l1=dLine("[[0,0],[100,20]]");
       assert_eq(iLine(10.0*cnv.bck_acc(l1, 1)),
              iLine("[[0,0],[31,10]]"));
       assert_eq(iLine(10.0*cnv.bck_acc(l1, 0.2)),
              iLine("[[0,0],[7,0],[23,5],[31,10]]"));
       assert_eq(iLine(10.0*cnv.bck_acc(l1, 0.05)),
              iLine("[[0,0],[1,0],[5,0],[9,0],[14,2],[20,4],[26,7],[31,10]]"));
       assert_eq(iLine(10.0*cnv.bck_acc(l1, 0)),
              iLine("[[0,0],[31,10]]"));

       assert_eq(cnv.bck_acc(l1, 0.5), cnv.bck_acc(l1));

       cnv.rescale_dst(0.1);
    }

    { // test frw_acc/bck_acc rect conversions
      // cnv is not so interesting here, rectangle converts to rectungle:
      assert_eq(cnv.frw_acc(dRect(0,0,10,10),0.005), dRect(0,0,100,20));
      assert_eq(cnv.bck_acc(dRect(0,0,100,20),0.005), dRect(0,0,10,10));

      // empty rectangle
      assert_eq(cnv.bck_acc(dRect()), dRect());
      assert_eq(cnv.frw_acc(dRect()), dRect());

    }

    { //scales
      ConvBase cnv0;
      cnv0.rescale_src(2);
      cnv0.rescale_dst(3);
      assert_err(cnv0.scales(dRect()),
        "ConvBase::scales: zero-size box");
      assert_err(cnv0.scales(dRect(1,2,0,0)),
        "ConvBase::scales: zero-size box");
      assert(dist2d(cnv0.scales(dRect(0,0,5,5)), dPoint(6,6)) < 1e-15);
    }

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
