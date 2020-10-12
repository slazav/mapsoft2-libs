///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err.h"

int
main(){

  Err e1(12); e1 << "123";
  Err e2(e1);
  Err e3; e3 << "234";
  e3 = e1;
  assert(e1.str()=="123");
  assert(e2.str()=="123");
  assert(e3.str()=="123");
  e3 << "234";
  assert(e3.str()=="123234");

  assert(e1.code()==12);
  assert(e2.code()==12);
  assert(e3.code()==12);

  // copy constructor test
  e3 = Err() << "aaaa";
  e3 << "mm";
  assert(e3.str()=="aaaamm");
  e3 << "xx";
  assert(e3.str()=="aaaammxx");
  try {
    try { throw e3;}
    catch (Err & e) { e << "yy"; throw;}
  }
  catch (Err & e) {
    assert(e.str() == "aaaammxxyy");
  }



  try {
    throw Err() << "text " << 123;
  }
  catch (Err & E){
    assert (E.str() == "text 123");
    assert (E.code() == -1);
  }

  try {
    throw Err(3) << "text " << 123;
  }
  catch (Err & E){
    assert (E.str()  == "text 123");
    assert (E.code() == 3);
  }
  return 0;

  // std::exception interface test
  try {
    throw Err(3) << "text " << 123;
  }
  catch (std::exception & e){
    assert (std::string(e.what())  == "text 123");
  }
  return 0;


}

///\endcond