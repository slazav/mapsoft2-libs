///\cond HIDDEN (do not show this in Doxyden)

#include <numeric>
#include <cassert>
#include <string>
#include <vector>
#include "err/err.h"

using namespace std;

// a few internal functions for tests:
vector<string> unpack_ozi_csv(const string & str, unsigned int count = 0);
string pack_ozi_csv(const vector<string> & vec);
string convert_ozi_text(const string & str);


string merge_strings(const vector<string> & v){
  string ret;
  for (size_t i=0; i!=v.size(); ++i) ret += "[" + v[i] + "] ";
  return ret;
}

int
main() {
  try{

    assert( merge_strings(unpack_ozi_csv(",aa,b,c,ddd,,eee\r")) ==
            "[] [aa] [b] [c] [ddd] [] [eee] ");
    assert( merge_strings(unpack_ozi_csv("v,,,eee,,")) ==
            "[v] [] [] [eee] [] [] ");
    assert( merge_strings(unpack_ozi_csv("a,b", 5)) ==
            "[a] [b] [] [] [] ");
    assert( merge_strings(unpack_ozi_csv("a", 5)) ==
            "[a] [] [] [] [] ");
    assert( merge_strings(unpack_ozi_csv(",", 5)) ==
            "[] [] [] [] [] ");
    assert( merge_strings(unpack_ozi_csv("", 5)) ==
            "[] [] [] [] [] ");
    assert( merge_strings(unpack_ozi_csv("a,b,c,,d", 1)) ==
            "[a] ");

    assert( pack_ozi_csv(unpack_ozi_csv(",aa,b,c,ddd,,eee")) ==
            ",aa,b,c,ddd,,eee");
    assert( pack_ozi_csv(unpack_ozi_csv("v,,,eee,,")) ==
            "v,,,eee");
    assert( pack_ozi_csv(unpack_ozi_csv("a,b", 5)) ==
            "a,b");
    assert( pack_ozi_csv(unpack_ozi_csv("a,b,c,,d", 1)) == "a");

    vector<string> v;
    v.push_back("a,b,c");
    v.push_back("d");
    string s = pack_ozi_csv(v);
    assert( s == "a�b�c,d");
    assert( merge_strings(unpack_ozi_csv(s)) == "[a,b,c] [d] ");

    assert( convert_ozi_text("abc,def\nghi") == "abc�def ghi");

  }
  catch (Err e) {
    cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond