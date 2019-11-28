#include "log.h"

std::ostream *Log::log = &std::cout;
std::ofstream Log::flog;
int Log::log_level = 0;


void
Log::set_log_file(const std::string & fname){
  if (fname=="-") {
    log = &std::cout;
  }
  else {
    if (flog.is_open()) flog.close();
    flog.open(fname, std::ios::app);
    if (flog.fail())
      throw Err() << "can't open log file: " << fname;
    log = &flog;
  }
}

