#include <string>
#include <iostream>
#include <memory>
#include <exception>
#include <ext/stdio_filebuf.h>
#include <unistd.h> // pipe
#include <signal.h> // kill
#include <sys/wait.h> // wait

#include "timer.h"
#include "iofilter.h"
#include "err/err.h"

// Note: One can make a stream from fd using __gnu_cxx::stdio_filebuf,
//       but it is difficult to extract fd from a stream (and dup2 it
//       for using as stdin in exec)
// https://www.ginac.de/~kreckel/fileno/
// https://blog.eldruin.com/inter-process-communication-pipe-fork-cpp-stl-like-streams/
//
// Here I use a two-fork method: one process copies data from istream to pipe,
// second process execute a program:
//  istream -> [pid1] -> fd1 -> [pid2] -> df2 -> new_istream
//                      1   0            1   0
// Next problem is returning the stream: if it is constructed from fd
// using __gnu_cxx::stdio_filebuf the buffer should be also kept
// until the stream is alive.
//
// Here I use Pimpl technique to hide all class details.
/***********************************************************/


class IFilter::Impl{
  private:
    std::unique_ptr<std::istream> istrp;
    std::unique_ptr<std::istream> ostrp;
    __gnu_cxx::stdio_filebuf<char> filebuf;
    int fd1[2], fd2[2];
    int pid1, pid2;

  public:

  std::istream & stream() {return *ostrp;}

  ~Impl() {
    int st;
    if (pid1!=0) waitpid(pid1, &st, 0);
    if (pid2!=0) waitpid(pid2, &st, 0);
    filebuf.close();
    ::close(fd2[0]); // should be after filebuf.close();
    pid1=pid2=0;
  }

  /***********************************************************/
  Impl(std::istream & istr, const std::string & prog): pid1(0), pid2(0){

    if (pipe(fd1)<0 || pipe(fd2)<0) throw Err() << "iofilter: pipe error";
    if ( (pid1 = fork()) < 0 ) throw Err() << "iofilter: fork1 error";

    signal(SIGPIPE, SIG_IGN); // ignore sigpipe (to avoid program exit)

    /******** process 1 ********/
    if (pid1 == 0) {
      ::close(fd1[0]);
      ::close(fd2[0]);
      ::close(fd2[1]);

      // just copy data from istr to the pipe and exit
      char buf[BUFSIZ];
      while (!istr.eof()){
        istr.read(buf, BUFSIZ);
        size_t size = istr.gcount();
        if (size == 0) continue;
        if (write(fd1[1], buf, size)!=size)
          std::cerr << "iofilter: write error\n";
      }
      ::close(fd1[1]);
      std::_Exit(0);
    }

    if ( (pid2 = fork()) < 0 ) throw Err() << "iofilter: fork2 error";

    /******** process 2 ********/
    if (pid2 == 0) {
      ::close(fd1[1]);
      ::close(fd2[0]);
      try {
        // attach stdin/stdout to pipes and execute the filter
        if (fd1[0] != STDIN_FILENO &&
            dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
              throw Err() << "iofilter: dup2 to stdin error";

        if (fd2[1] != STDOUT_FILENO &&
            dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
              throw Err() << "iofilter: dup2 to stdout error";

        if (execl("/bin/sh", "sh", "-c", prog.c_str(), (char *)0) < 0 )
           throw Err() << "iofilter: exec error";
      }
      catch (Err e) {
        std::cerr << e.str() << "\n";
      }
      ::close(fd1[0]);
      ::close(fd2[1]);
      std::_Exit(0);
    }

    /******** parent process ********/

    ::close(fd1[0]);
    ::close(fd1[1]);
    ::close(fd2[1]);

    filebuf = __gnu_cxx::stdio_filebuf<char>(fd2[0], std::ios::in);
    ostrp = std::unique_ptr<std::istream>(new std::istream(&filebuf));
  }

  /***********************************************************/
  // simple constructor, just read output of the program
  Impl(const std::string & prog): pid1(0), pid2(0){

    if (pipe(fd2)<0) throw Err() << "iofilter: pipe error";

    if ((pid1 = fork()) < 0 ) throw Err() << "iofilter: fork2 error";

    /******** child process ********/
    if (pid1 == 0) {
      ::close(fd2[0]);
      try {
        // attach stdout to the pipe and execute the program
        if (fd2[1] != STDOUT_FILENO &&
            dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
              throw Err() << "iofilter: dup2 to stdout error";

        if (execl("/bin/sh", "sh", "-c", prog.c_str(), (char *)0) < 0 )
           throw Err() << "iofilter: exec error";
      }
      catch (Err e) {
        std::cerr << e.str() << "\n";
      }
      ::close(fd2[1]);
      std::_Exit(0);
    }

    /******** parent process ********/

    ::close(fd2[1]);

    filebuf = __gnu_cxx::stdio_filebuf<char>(fd2[0], std::ios::in);
    ostrp = std::unique_ptr<std::istream>(new std::istream(&filebuf));
  }
};

/***********************************************************/

IFilter::IFilter(std::istream & istr, const std::string & prog):
   impl(new Impl(istr, prog)) {}

IFilter::IFilter(const std::string & prog):
   impl(new Impl(prog)) {}

IFilter::~IFilter() {}

std::istream &
IFilter::stream(){ return impl->stream(); }

/***********************************************************/
/***********************************************************/
// Same for OFilter
//
//  new_ostream -> fd2 -> [pid2] -> fd1 -> [pid1] -> ostream
//                1   0            1   0


class OFilter::Impl{
  private:
    std::unique_ptr<std::ostream> istrp;
    std::unique_ptr<std::ostream> ostrp;
    __gnu_cxx::stdio_filebuf<char> filebuf;
    int fd1[2], fd2[2];
    int pid1, pid2;

  public:

  std::ostream & stream() {return *ostrp;}

  ~Impl() {
    // Note the difference with IFilter: there the program
    // should stop at the end of input (which we do not control).
    // We do waitpid, then close unused streams.
    // Here we close the stream (which we control) and then do waitpid.
    filebuf.close(); // close the process input stream
    ::close(fd2[1]); // should be after filebuf.close();
    int st;
    if (pid1!=0) waitpid(pid1, &st, 0);
    if (pid2!=0) waitpid(pid2, &st, 0);
    pid1=pid2=0;
  }

  /***********************************************************/
  Impl(std::ostream & ostr, const std::string & prog): pid1(0), pid2(0){

    if (pipe(fd1)<0 || pipe(fd2)<0) throw Err() << "iofilter: pipe error";
    if ( (pid1 = fork()) < 0 ) throw Err() << "iofilter: fork1 error";
    signal(SIGPIPE, SIG_IGN); // ignore sigpipe (to avoid program exit)

    /******** process 1 ********/
    if (pid1 == 0) {
      ::close(fd1[1]);
      ::close(fd2[0]);
      ::close(fd2[1]);

      // just copy data from pipe to ostr and exit
      char buf[BUFSIZ];
      while (size_t size = read(fd1[0], buf, BUFSIZ)){
        ostr.write(buf, size);
        if (ostr.fail())
          std::cerr << "iofilter: write error\n";
      }
      ostr.flush();
      ::close(fd1[0]);
      std::_Exit(0);
    }

    if ( (pid2 = fork()) < 0 ) throw Err() << "iofilter: fork2 error";

    /******** process 2 ********/
    if (pid2 == 0) {
      ::close(fd1[0]);
      ::close(fd2[1]);
      try {
        // attach stdin/stdout to pipes and execute the filter
        if (fd2[0] != STDIN_FILENO &&
            dup2(fd2[0], STDIN_FILENO) != STDIN_FILENO)
              throw Err() << "iofilter: dup2 to stdin error";

        if (fd1[1] != STDOUT_FILENO &&
            dup2(fd1[1], STDOUT_FILENO) != STDOUT_FILENO)
              throw Err() << "iofilter: dup2 to stdout error";

        if (execl("/bin/sh", "sh", "-c", prog.c_str(), (char *)0) < 0 )
           throw Err() << "iofilter: exec error";
      }
      catch (Err e) {
        std::cerr << e.str() << "\n";
      }
      ::close(fd1[1]);
      ::close(fd2[2]);
      std::_Exit(0);
    }

    /******** parent process ********/

    ::close(fd1[0]);
    ::close(fd1[1]);
    ::close(fd2[0]);

    filebuf = __gnu_cxx::stdio_filebuf<char>(fd2[1], std::ios::out);
    ostrp = std::unique_ptr<std::ostream>(new std::ostream(&filebuf));
  }

  /***********************************************************/
  // simple constructor, just read output of the program
  Impl(const std::string & prog): pid1(0), pid2(0){

    if (pipe(fd2)<0) throw Err() << "iofilter: pipe error";

    if ((pid2 = fork()) < 0 ) throw Err() << "iofilter: fork2 error";

    /******** child process ********/
    if (pid2 == 0) {
      ::close(fd2[1]);
      try {
        // attach stdout to the pipe and execute the program
        if (fd2[0] != STDIN_FILENO &&
            dup2(fd2[0], STDIN_FILENO) != STDIN_FILENO)
              throw Err() << "iofilter: dup2 to stdin error";

        if (execl("/bin/sh", "sh", "-c", prog.c_str(), (char *)0) < 0 )
           throw Err() << "iofilter: exec error";
      }
      catch (Err e) {
        std::cerr << e.str() << "\n";
      }
      ::close(fd2[0]);
      std::_Exit(0);
    }

    /******** parent process ********/

    ::close(fd2[0]);

    filebuf = __gnu_cxx::stdio_filebuf<char>(fd2[1], std::ios::out);
    ostrp = std::unique_ptr<std::ostream>(new std::ostream(&filebuf));
  }
};

/***********************************************************/

OFilter::OFilter(std::ostream & ostr, const std::string & prog):
   impl(new Impl(ostr, prog)) {}

OFilter::OFilter(const std::string & prog):
   impl(new Impl(prog)) {}

OFilter::~OFilter() {}

std::ostream &
OFilter::stream(){ return impl->stream(); }

/***********************************************************/
/***********************************************************/
// same for IOFilter
class IOFilter::Impl{
  private:
    std::unique_ptr<std::istream> istrp;
    std::unique_ptr<std::ostream> ostrp;
    __gnu_cxx::stdio_filebuf<char> ifilebuf, ofilebuf;
    Timer timer;
    int pid;
    int fd1[2], fd2[2];

  public:

  std::istream & istream() {return *istrp;}
  std::ostream & ostream() {return *ostrp;}

  ~Impl() {
    close_input();
    int st=0;
    if (pid!=0) waitpid(pid, &st, 0);
    ifilebuf.close();
    ::close(fd2[0]);
    pid=0;
  };

  /***********************************************************/
  Impl(const std::string & prog): pid(0){

    if (pipe(fd1)<0 || pipe(fd2)<0) throw Err() << "iofilter: pipe error";
    if ( (pid = fork()) < 0 ) throw Err() << "iofilter: fork1 error";
    signal(SIGPIPE, SIG_IGN); // ignore sigpipe (to avoid program exit)

    /******** child process ********/
    if (pid == 0) {
      ::close(fd1[1]);
      ::close(fd2[0]);
      try {
        // attach stdin/stdout to pipes and execute the filter
        if (fd1[0] != STDIN_FILENO &&
            dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
              throw Err() << "iofilter: dup2 to stdin error";

        if (fd2[1] != STDOUT_FILENO &&
            dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
              throw Err() << "iofilter: dup2 to stdout error";

        if (execl("/bin/sh", "sh", "-c", prog.c_str(), (char *)0) < 0 )
           throw Err() << "iofilter: exec error";
      }
      catch (Err e) {
        std::cerr << e.str() << "\n";
      }
      ::close(fd1[0]);
      ::close(fd2[1]);
      std::_Exit(0);
    }

    /******** parent process ********/

    ::close(fd1[0]);
    ::close(fd2[1]);
    timer.create(pid);

    ofilebuf = __gnu_cxx::stdio_filebuf<char>(fd1[1], std::ios::out);
    ostrp = std::unique_ptr<std::ostream>(new std::ostream(&ofilebuf));
    ifilebuf = __gnu_cxx::stdio_filebuf<char>(fd2[0], std::ios::in);
    istrp = std::unique_ptr<std::istream>(new std::istream(&ifilebuf));
  }

  void close_input(){
    ofilebuf.close(); // close the process input stream
    ::close(fd1[1]); // should be after filebuf.close()
  }

  void timer_start(int usec){ timer.start(pid, usec); }
  void timer_stop(){ timer.stop(); }
};

/***********************************************************/

IOFilter::IOFilter(const std::string & prog):
   impl(new Impl(prog)) {}

IOFilter::~IOFilter() {}

std::istream &
IOFilter::istream(){ return impl->istream(); }

std::ostream &
IOFilter::ostream(){ return impl->ostream(); }

void IOFilter::close_input(){ impl->close_input(); }

void IOFilter::timer_start(int usec){ impl->timer_start(usec); }

void IOFilter::timer_stop(){ impl->timer_stop(); }

