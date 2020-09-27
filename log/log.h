#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <iostream>
#include "err/err.h"

/*************************************************/
/*
## Log class

It is a simple class for writing log messages to stdin or file.
Most functions are static members of the class and work globally.
`Log` objects are created only for formatting messages.

Usage:
```
Log(level) << data;
```

*/

class Log {
  static std::ostream *log;  // log stream
  static std::ofstream flog; // log stream for file logs
  static int log_level;
  bool empty;

public:

  /// Set log file (use "-" for stdin). By default log messages are printed using stdin.
  static void set_log_file(const std::string & fname);

  /// Set log level. All messages with larger level will be skipped.
  static void set_log_level(const int lvl) { log_level = lvl; }

  /// Get current log level
  static int get_log_level(){ return log_level; }

  /// Create log object
  Log(int l): empty(l>log_level) { }
  ~Log() { if (!empty) (*log) << "\n";}

  /// Operator << for log messages.
  template <typename T>
  Log & operator<<(const T & o){
    if (!empty) {
      (*log) << o;
      log->flush();
    }
    return *this;
  }

};

#endif
