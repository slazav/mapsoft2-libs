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
    std::ifstream ff("iofilter.test.cpp");
    IFilter flt(ff, "echo a");

    std::string l;
    std::getline(flt.stream(), l);
    assert_eq(l, "a");
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
      std::ofstream ff("test1.tmp");
      OFilter flt(ff, "echo a");
      flt.stream() << "test1\ntest2\n";
    }
    std::ifstream fi("test1.tmp");
    std::string l;
    std::getline(fi, l);
    assert_eq(l, "a");
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

  {
    IOFilter flt("tac");
    flt.ostream() << "test1\ntest2\n";

    flt.ostream() << "test3\ntest4\n"; // we have time to send something
    flt.close_input(); // without close_input it will wait forever

    std::string l;
    assert_eq(flt.getline(l, 10), 5); // timeout: 10, read 5 chars
    assert_eq(l, "test4");
    assert_eq(flt.getline(l, 10), 5);
    assert_eq(l, "test3");
    assert_eq(flt.getline(l, 10), 5);
    assert_eq(l, "test2");
    assert_eq(flt.getline(l, 10), 5);
    assert_eq(l, "test1");
    assert_eq(flt.getline(l, 10), -1); //eof
  }

  {
    IOFilter flt("tac");
    flt.ostream() << "test1\ntest2\n";

    flt.ostream() << "test3\ntest4\n"; // we have time to send something
    // without close_input it will wait forever

    std::string l;
    assert_err(flt.getline(l, 0.1), "Read timeout");
  }

  {
    IOFilter flt("sleep 5; sleep 5");
    flt.ostream() << "test1\ntest2\n";
    flt.close_input();

    std::string l;
    assert_err(flt.getline(l, 0.1), "Read timeout");
    assert_eq(l, "");
    flt.kill(); // IOFilter will wait for process termination
  }

  {
    IOFilter flt("echo a; echo b; sleep 10");

    std::string l;
    assert_eq(flt.getline(l, 0.1), 1);
    assert_eq(l, "a");

    assert_eq(flt.getline(l, 0.1), 1);
    assert_eq(l, "b");

    assert_err(flt.getline(l, 0.1), "Read timeout");
    assert_eq(l, "");

    flt.kill(); // IOFilter will wait for process termination
  }

  return 0;

}
catch(Err & e){
  std::cerr << "Error: " << e.str() << "\n";
  return 1;
}
}
