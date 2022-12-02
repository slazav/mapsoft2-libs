///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "poly_tools.h"
#include "opt/opt.h"

int
main(){
  try{

    // point_in_polygon (int)
    for (int b=0; b<2; b++){
      iLine L("[[0,0],[8,0],[4,8]]");
      assert_eq(point_in_polygon(iPoint("[-1,-1]"),L,b), false);
      assert_eq(point_in_polygon(iPoint("[7,3]"),L,b), false);
      assert_eq(point_in_polygon(iPoint("[1,1]"),L,b), true);

      // borders
      assert_eq(point_in_polygon(iPoint("[0,0]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[8,0]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[4,0]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[4,8]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[1,2]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[7,2]"),L,b), b);
    }

    {
      iLine L("[[-10,-10], [64,-10], [128,64], [-10,100]]");
      assert_eq(point_in_polygon(iPoint("[130,64]"),L,1), false);
      assert_eq(point_in_polygon(iPoint("[120,64]"),L,1), true);
    }

    {
      iLine L("[[0,0], [10,0], [5,10], [0,0]]");
      assert_eq(point_in_polygon(iPoint("[10,10]"),L,1), false);
      assert_eq(point_in_polygon(iPoint("[0,10]"),L,1), false);
      assert_eq(point_in_polygon(iPoint("[5,10]"),L,1), true);
    }

    // point_in_polygon (double)
    for (int b=0; b<2; b++){
      dLine L("[[0,0],[8,0],[4,8]]");
      assert_eq(point_in_polygon(dPoint("[-1,-1]"),L,b), false);
      assert_eq(point_in_polygon(dPoint("[7,3]"),L,b), false);
      assert_eq(point_in_polygon(dPoint("[1,1]"),L,b), true);
      // borders
      assert_eq(point_in_polygon(dPoint("[0,0]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[8,0]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[4,0]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[4,8]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[1,2]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[7,2]"),L,b), b);

    }

    // point_in_polygon (double, multiline)
    for (int b=0; b<2; b++){
      dMultiLine L("[[[0,0],[6,0],[6,4],[4,4],[4,8],[6,8],[6,12],[4,12],[4,16],[0,12]], "
                    "[[3,4],[2,4],[2,8],[3,8],[3,10],[1,10],[1,2],[3,2],[3,4]], "
                    "[[8,4],[10,4],[10,2],[12,2],[12,8],[8,8]]]");

      assert_eq(point_in_polygon(dPoint("[-1,-1]"),L,b), false);
      assert_eq(point_in_polygon(dPoint("[1,-1]"),L,b), false);
      assert_eq(point_in_polygon(dPoint("[1,1]"),L,b), true);
      assert_eq(point_in_polygon(dPoint("[2,3]"),L,b), false);
      assert_eq(point_in_polygon(dPoint("[1,15]"),L,b), false);
      assert_eq(point_in_polygon(dPoint("[0.5,4]"),L,b), true);
      assert_eq(point_in_polygon(dPoint("[7,4]"),L,b), false);
      assert_eq(point_in_polygon(dPoint("[9,5]"),L,b), true);


      // borders
      assert_eq(point_in_polygon(dPoint("[3,0]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[3,3]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[2,6]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[5,4]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[4,7]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[4,13]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[2,14]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[6,0]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[4,4]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[4,8]"),L,b), b);
      assert_eq(point_in_polygon(dPoint("[8,4]"),L,b), b);
    }

    // same, integer
    for (int b=0; b<2; b++){
      iMultiLine L("[[[0,0],[6,0],[6,4],[4,4],[4,8],[6,8],[6,12],[4,12],[4,16],[0,12]], "
                    "[[3,4],[2,4],[2,8],[3,8],[3,10],[1,10],[1,2],[3,2],[3,4]], "
                    "[[8,4],[10,4],[10,2],[12,2],[12,8],[8,8]]]");

      assert_eq(point_in_polygon(iPoint("[-1,-1]"),L,b), false);
      assert_eq(point_in_polygon(iPoint("[1,-1]"),L,b), false);
      assert_eq(point_in_polygon(iPoint("[1,1]"),L,b), true);
      assert_eq(point_in_polygon(iPoint("[2,3]"),L,b), false);
      assert_eq(point_in_polygon(iPoint("[1,15]"),L,b), false);
      assert_eq(point_in_polygon(iPoint("[7,4]"),L,b), false);
      assert_eq(point_in_polygon(iPoint("[9,5]"),L,b), true);


      // borders
      assert_eq(point_in_polygon(iPoint("[3,0]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[3,3]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[2,6]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[5,4]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[4,7]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[4,13]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[2,14]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[6,0]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[4,4]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[4,8]"),L,b), b);
      assert_eq(point_in_polygon(iPoint("[8,4]"),L,b), b);
    }

    /*****************************************************/

    // rect_in_polygon (int)
    {
      iLine L("[[0,0],[8,0],[4,8]]");
      assert_eq(rect_in_polygon(iRect("[-2,-2,1,1]"),L), 0);
      assert_eq(rect_in_polygon(iRect("[7,3,1,1]"),L), 0);
      assert_eq(rect_in_polygon(iRect(),L), 0);
      assert_eq(rect_in_polygon(iRect("[1,1,0,0]"),L), 2);
      assert_eq(rect_in_polygon(iRect("[0,0,1,1]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[4,1,1,1]"),L), 2);
      assert_eq(rect_in_polygon(iRect("[-1,-1,10,10]"),L), 3);
      // borders
      assert_eq(rect_in_polygon(iRect("[0,0,0,0]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[-1,-1,1,1]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[8,0,1,1]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[6,1,1,1]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[4,-1,1,1]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[4,8,1,1]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[0,2,1,1]"),L), 1);
      assert_eq(rect_in_polygon(iRect("[7,2,1,1]"),L), 1);
    }


    // rect_in_polygon (double)
    {
      dLine L("[[0,0],[8,0],[4,8]]");
      assert_eq(rect_in_polygon(dRect("[-2,-2,1,1]"),L), 0);
      assert_eq(rect_in_polygon(dRect("[7,3,1,1]"),L), 0);
      assert_eq(rect_in_polygon(dRect(),L), 0);
      assert_eq(rect_in_polygon(dRect("[1,1,0,0]"),L), 2);
      assert_eq(rect_in_polygon(dRect("[0,0,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[4,1,1,1]"),L), 2);
      assert_eq(rect_in_polygon(dRect("[-1,-1,10,10]"),L), 3);

      // borders
      assert_eq(rect_in_polygon(dRect("[0,0,0,0]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[-1,-1,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[6,1,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[8,0,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[4,-1,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[4,8,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[0,2,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[7,2,1,1]"),L), 1);
    }

    // rect_in_polygon (double, MultiLine)
    for (int b=0; b<2; b++){
      dMultiLine L("[ [[0,0],[8,0],[4,8]], [[3,1],[2,3],[4,7],[6,3],[5,1]]]");
      assert_eq(rect_in_polygon(dRect("[-2,-2,1,1]"),L), 0);
      assert_eq(rect_in_polygon(dRect("[7,3,1,1]"),L), 0);
      assert_eq(rect_in_polygon(dRect("[3,3,1,1]"),L), 0); // in the hole
      assert_eq(rect_in_polygon(dRect(),L), 0);
      assert_eq(rect_in_polygon(dRect("[1,1,0,0]"),L), 2);
      assert_eq(rect_in_polygon(dRect("[0,0,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[2,1,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[2,4,1,1]"),L), 1);
      assert_eq(rect_in_polygon(dRect("[6,0.5,1,1]"),L), 2);
      assert_eq(rect_in_polygon(dRect("[-1,-1,10,10]"),L), 3);
      assert_eq(rect_in_polygon(dRect("[-1,0.1,10,10]"),L), 1);
    }


    // join_polygons, remove_holes
    {
      dMultiLine ml("["
        "[[0,0],[10,0],[10,10],[0,10]],"
        "[[1,1],[2,1],[1,2]],"
        "[[11,1],[12,1],[11,2]],"
        "[[9,8],[9,9],[8,9]]"
       "]");

      assert_eq(join_polygons(ml), dLine(
        "[[0,0],[1,1],[2,1],[1,2],[1,1],[0,0],"
        "[10,0],[11,1],[12,1],[11,2],[11,1],[10,0],"
        "[10,10],[9,9],[8,9],[9,8],[9,9],[10,10],[0,10]]"));

      remove_holes(ml);
      assert_eq(ml, dMultiLine(
        "[[[0,0],[1,1],[2,1],[1,2],[1,1],[0,0],"
        "[10,0],[10,10],[9,9],[8,9],[9,8],[9,9],[10,10],[0,10]],"
        "[[11,1],[12,1],[11,2]]]"));

      ml = dMultiLine("[]");
      assert_eq(join_polygons(ml), dLine("[]"));
      remove_holes(ml);
      assert_eq(ml, dMultiLine("[]"));

      ml = dMultiLine("[[]]");
      assert_eq(join_polygons(ml), dLine("[]"));
      remove_holes(ml);
      assert_eq(ml, dMultiLine("[[]]"));
    }

     // figure_line, figure_bbox
    {
      assert_eq(figure_line<int>("[1,2]"), iMultiLine("[[1,2]]")); // Point
      assert_eq(figure_line<int>("[1,2,3]"), iMultiLine("[[1,2,3]]")); // Point
      assert_eq(figure_line<int>("[1,2,3,4]"), iMultiLine("[[1,2],[4,2],[4,6],[1,6],[1,2]]")); // Rect
      assert_eq(figure_line<int>("[1,2,0,0]"), iMultiLine("[[1,2],[1,2],[1,2],[1,2],[1,2]]")); // zero-size Rect
      assert_eq(figure_line<int>("[[1,2],[3,4]]"), iMultiLine("[[1,2],[3,4]]")); // Line
      assert_eq(figure_line<int>("[[[1,2],[3,4]],[]]"), iMultiLine("[[[1,2],[3,4]],[]]")); // MultiLine
      assert_eq(figure_line<int>(""), iMultiLine());
      assert_eq(figure_line<int>("[]"), iMultiLine()); // empty Line/Multiline
      assert_eq(figure_line<int>("[[]]"), iMultiLine("[[]]")); // MultiLine with one empty segment

      assert_err(figure_line<int>("aaa"), "can't read figure: aaa"); // Rect


      assert_eq(figure_bbox<int>("[1,2]"), iRect(1,2,0,0)); // Point
      assert_eq(figure_bbox<int>("[1,2,3]"), iRect(1,2,0,0)); // Point (z ignored)
      assert_eq(figure_bbox<int>("[1,2,3,4]"), iRect(1,2,3,4)); // Rect
      assert_eq(figure_bbox<int>("[1,2,0,0]"), iRect(1,2,0,0)); // Rect
      assert_eq(figure_bbox<int>("[[1,2],[3,4]]"), iRect(1,2,2,2)); // Line
      assert_eq(figure_bbox<int>("[[[1,2],[3,4]],[]]"), iRect(1,2,2,2)); // MultiLine
      assert_eq(figure_bbox<int>(""), iRect());
      assert_eq(figure_bbox<int>("[]"), iRect()); // empty Line
      assert_eq(figure_bbox<int>("[[]]"), iRect()); // empty MultiLine

      assert_err(figure_bbox<int>("aaa"), "can't read figure: aaa"); // Rect

    }

    // nearest_pt
    {
      dPoint p0d;
      iPoint p0i;
      assert_feq(nearest_vertex(iLine("[[0,0],[2,1],[4,2]]"), iPoint(1,2), &p0i), sqrt(2), 1e-6);
      assert_eq(p0i, dPoint(2,1));

      assert_feq(nearest_vertex(dLine("[[0,0],[2,1],[4,2]]"), dPoint(1,2), &p0d), sqrt(2), 1e-6);
      assert_eq(p0d, dPoint(2,1));

      assert_feq(nearest_vertex(dMultiLine("[[], [[0,0],[2,1],[4,2]]]"), dPoint(1,2), &p0d), sqrt(2), 1e-6);
      assert_eq(p0d, dPoint(2,1));

      assert_feq(nearest_vertex(dMultiLine("[[], [[0,0],[2,1],[4,2]]]"), dPoint(1,2)), sqrt(2), 1e-6);
      assert_err(nearest_vertex(dMultiLine("[[], []]"), dPoint(1,2), &p0d), "Can't find nearest point: empty line");
    }


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond