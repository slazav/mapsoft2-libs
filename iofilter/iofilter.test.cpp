/* Test program! */
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cassert>

#include "err/assert_err.h"
#include "iofilter.h"



int
main(){
try{
  {
    // We have an istream which reads from a file.
    // We want to read from this stream, not directly but
    // through some filter program.
    std::ifstream ff("iofilter.test.cpp");
    IFilter flt(ff, "cat");

    std::string l;
    std::getline(flt.stream(), l);
    assert_eq(l, "/* Test program! */");

    // additional processes should exist (possibly in defunct state)
    // while IFilter is alive
  }

  {
    // same, but just read from a process
    IFilter flt("echo abc");
    std::string l;
    std::getline(flt.stream(), l);
    assert_eq(l, "abc");
  }

  {
    // We have an ostream which writes to a file.
    // We want to write to this stream through a
    // filter program (here it is tac)
    {
      std::ofstream ff("test1.tmp");
      OFilter flt(ff, "tac");
      flt.stream() << "test1\ntest2\n";
    }

    std::ifstream fi("test1.tmp");
    std::string l;
    std::getline(fi, l);
    assert_eq(l, "test2");
    std::getline(fi, l);
    assert_eq(l, "test1");
    unlink("test1.tmp");
  }

  {
    {
      OFilter flt("tac > test2.tmp");
      flt.stream() << "test1\ntest2\n";
    }
    std::ifstream fi("test2.tmp");
    std::string l;
    std::getline(fi, l);
    assert_eq(l, "test2");
    std::getline(fi, l);
    assert_eq(l, "test1");
    unlink("test2.tmp");
  }


  {
    IOFilter flt("tac");
    flt.ostream() << "test1\ntest2\n";
    flt.close_input();

    std::string l;
    std::getline(flt.istream(), l);
    assert_eq(l, "test2");
    std::getline(flt.istream(), l);
    assert_eq(l, "test1");
  }

  return 0;

}
catch(Err e){
  std::cerr << "Error: " << e.str() << "\n";
  return 1;
}
}
