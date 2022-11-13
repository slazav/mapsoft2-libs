///\cond HIDDEN (do not show this in Doxyden)

#include <unistd.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include "err/assert_err.h"
#include "string_pack.h"

// Packing and unpacking of VMapObj

using namespace std;


int
main(){
  try{

    // low-level functions for packing and unpacking strings and coordinates
    {
      std::ostringstream s1;
      string_pack_str(s1, "str1", "text1");
      string_pack_str(s1, "str2", "text2");
      string_pack_str(s1, "str3", "\\text3\ntext3a\\");

      dMultiLine crds1("[[[-180,-90],[0,0],[180,90]],[],[[37.11,56.20],[37.22,56.11]]]");
      string_pack_crds(s1, "crds", crds1);

      dRect bbox("[100,20,10,10]");
      string_pack_bbox(s1, "bbox", bbox);

      dPoint pt(10,10);
      string_pack_pt(s1, "ptpt", pt);

      int i=123;
      string_pack<int>(s1, "iint", i);

      dMultiLine crds2;
      std::istringstream s2(s1.str());
      while (1){
        std::string tag = string_unpack_tag(s2);
        if (tag == "") break;
        else if (tag == "str1") {assert_eq(string_unpack_str(s2), "text1");}
        else if (tag == "str2") {assert_eq(string_unpack_str(s2), "text2");}
        else if (tag == "str3") {assert_eq(string_unpack_str(s2), "\\text3\ntext3a\\");}
        else if (tag == "crds") {crds2.push_back(string_unpack_crds(s2));}
        else if (tag == "bbox") {assert_eq(string_unpack_bbox(s2), bbox);}
        else if (tag == "ptpt") {assert_eq(string_unpack_pt(s2), pt);}
        else if (tag == "iint") {assert_eq(string_unpack<int>(s2), i);}
        else throw Err() << "Unknown tag: " << tag;
      }
      assert_deq(crds1, crds2, 1e-7);
    }

    // same for writing human-readable text file
    {
      std::ostringstream s1;
      string_write_str(s1, "str1", "text1");
      string_write_str(s1, "str2", "text2");
      string_write_str(s1, "str3", "\\text3\ntext3a\\");

      dMultiLine crds1("[[[-180,-90],[0,0],[180,90]],[],[[37.11,56.20],[37.22,56.11]]]");
      string_write_crds(s1, "crds", crds1);

      dRect bbox("[100,20,10,10]");
      string_write_bbox(s1, "bbox", bbox);

      dPoint pt(10,10);
      string_write_pt(s1, "pt", pt);

      int i=123;
      string_write<int>(s1, "int", i);

      dMultiLine crds2;
      std::istringstream s2(s1.str());
      while (1){
        std::string tag = string_read_tag(s2);
        if (tag == "") break;
        else if (tag == "str1") {assert_eq(string_read_str(s2), "text1");}
        else if (tag == "str2") {assert_eq(string_read_str(s2), "text2");}
        else if (tag == "str3") {assert_eq(string_read_str(s2), "\\text3\ntext3a\\");}
        else if (tag == "crds") {crds2.push_back(string_read_crds(s2));}
        else if (tag == "bbox") {assert_eq(string_read_bbox(s2), bbox);}
        else if (tag == "pt")   {assert_eq(string_read_pt(s2), pt);}
        else if (tag == "int")  {assert_eq(string_read<int>(s2), i);}
        else throw Err() << "Unknown tag: " << tag;
      }
      assert_deq(crds1, crds2, 1e-7);
    }


  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
