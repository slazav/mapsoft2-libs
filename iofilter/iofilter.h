#ifndef IOFILTER_H
#define IOFILTER_H

#include <string>
#include <iostream>
#include <memory>


/*
  A class for filterng an std::istream through a program.

  constructor:

  IFilter flt(stream, progname) -- run a program and send the stream to its stdin.
  IFilter flt(progname)         -- run a program without sending anything to stdin.

  flt.stream() - returns reference to a new std::istream attached to stdout of the program.
*/

class IFilter {
  private:
    class Impl;
    std::unique_ptr<Impl> impl;

  public:
    IFilter(std::istream & istr, const std::string & prog);

    IFilter(const std::string & prog);

    ~IFilter();

    std::istream & stream();
};

/**************************/
class OFilter {
  private:
    class Impl;
    std::unique_ptr<Impl> impl;

  public:
    OFilter(std::ostream & ostr, const std::string & prog);

    OFilter(const std::string & prog);

    ~OFilter();

    std::ostream & stream();
};

/**************************/

class IOFilter {
  private:
    class Impl;
    std::unique_ptr<Impl> impl;

  public:
    IOFilter(std::ostream & ostr, const std::string & prog);

    IOFilter(const std::string & prog);

    ~IOFilter();

    std::istream & istream();
    std::ostream & ostream();
    void close_input();
    void timer_start(double sec); // set timer to kill the filter process after `sec` seconds
    void timer_stop(); // stop the timer
    double timer_get(); // get timer value in seconds
    bool   timer_expired();   // is timer has value 0?

};

#endif