///\cond HIDDEN (do not show this in Doxyden)

#include <unistd.h>
#include "log.h"
#include "err/err.h"
#include "err/assert_err.h"

int
main(){
  unlink("log1.tmp");
  unlink("log2.tmp");

  try {

    Log::set_log_file("log1.tmp");
    Log(0) << "string " << 0;
    Log(1) << "string " << 1;
    assert_eq(Log::get_log_level(), 0);

    Log::set_log_file("log2.tmp");
    Log::set_log_level(2);
    Log(1) << "string " << 1;
    Log(2) << "string " << 2;
    Log(3) << "string " << 3;

    assert_eq(Log::get_log_level(), 2);
    assert_err(Log::set_log_file("tmp/log2.tmp"),
      "can't open log file: tmp/log2.tmp");

    std::ifstream s1("log1.tmp");
    std::ifstream s2("log2.tmp");

    std::string s;
    getline(s1, s);
    assert_eq(s, "string 0");
    getline(s1, s);
    assert_eq(s, "");

    getline(s2, s);
    assert_eq(s, "string 1");
    getline(s2, s);
    assert_eq(s, "string 2");
    getline(s2, s);
    assert_eq(s, "");

  }
  catch (Err E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  unlink("log1.tmp");
  unlink("log2.tmp");
  return 0;
}

///\endcond