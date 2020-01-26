///\cond HIDDEN (do not show this in Doxyden)

#include <sstream>
#include "err/assert_err.h"
#include "mp.h"


int
main(){
  try {

    MP m;
    m.push_back(MPObj());

    std::ostringstream ss;
    write_mp(ss, m);


    // defaults
    assert_eq(ss.str(),
      "[IMG ID]\r\n"
      "ID=0\r\n"
      "Name=\r\n"
      "LblCoding=9\r\n"
      "Codepage=1251\r\n"
      "Elevation=M\r\n"
      "TreSize=6000\r\n"
      "RgnLimit=1024\r\n"
      "PreProcess=F\r\n"
      "Levels=4\r\n"
      "Level0=22\r\n"
      "Level1=21\r\n"
      "Level2=19\r\n"
      "Level3=17\r\n"
      "Zoom0=0\r\n"
      "Zoom1=1\r\n"
      "Zoom2=2\r\n"
      "Zoom3=3\r\n"
      "MG=Y\r\n"
      "PolygonEvaluate=Y\r\n"
      "Transparent=N\r\n"
      "[END-IMG ID]\r\n"
      "[POI]\r\n"
      "Type=0xffffffff\r\n"
      "[END]\r\n"
      "\r\n"
    );

    // comment, multi-line name
    m.Comment = "a\n\nb\n";
    m.begin()->Comment = "a\n\nb\n";
    m.Name    = "\n\nname\nname1";
    m.begin()->Label = "\n\nname\nname1";
    m.begin()->Opts.put("Key1", "val1\nval2");

    ss.str("");
    write_mp(ss, m);

    assert_eq(ss.str(),
      ";a\r\n"
      ";\r\n"
      ";b\r\n"
      ";\r\n"
      "[IMG ID]\r\n"
      "ID=0\r\n"
      "Name=  name name1\r\n"
      "LblCoding=9\r\n"
      "Codepage=1251\r\n"
      "Elevation=M\r\n"
      "TreSize=6000\r\n"
      "RgnLimit=1024\r\n"
      "PreProcess=F\r\n"
      "Levels=4\r\n"
      "Level0=22\r\n"
      "Level1=21\r\n"
      "Level2=19\r\n"
      "Level3=17\r\n"
      "Zoom0=0\r\n"
      "Zoom1=1\r\n"
      "Zoom2=2\r\n"
      "Zoom3=3\r\n"
      "MG=Y\r\n"
      "PolygonEvaluate=Y\r\n"
      "Transparent=N\r\n"
      "[END-IMG ID]\r\n"
      ";a\r\n"
      ";\r\n"
      ";b\r\n"
      ";\r\n"
      "[POI]\r\n"
      "Type=0xffffffff\r\n"
      "Label=  name name1\r\n"
      "Key1=val1 val2\r\n"
      "[END]\r\n"
      "\r\n"
    );

  }
  catch (Err E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
