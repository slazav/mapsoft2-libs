#ifndef TIMER_H
#define TIMER_H

#include <memory>
#include <signal.h> // kill
#include <time.h> // timers

#include "err/err.h"

/*
  A class for killing a process after a certain timeout
*/

void on_timer_signal(union sigval s){
  int pid = s.sival_int;
  if (pid==0) return;
  kill(pid, SIGABRT);
  return;
}

/// should be run in following order:
/// constructor -> create(pid) -> *(start(time) | stop())* ->destructor



class Timer {
  private:
    std::unique_ptr<struct sigevent> sevp;
    std::unique_ptr<timer_t> timer;

  public:
    Timer():sevp(new struct sigevent), timer(new timer_t) {}

    void create(int pid) {
      sevp->sigev_notify = SIGEV_THREAD;
      sevp->sigev_value.sival_int = pid;
      sevp->sigev_notify_function = on_timer_signal;
      sevp->sigev_notify_attributes = NULL;
      timer.reset(new timer_t);
      if (timer_create(CLOCK_MONOTONIC, sevp.get(), timer.get())!=0)
         throw Err() << "IOFilter: can't create POSIX timer";
    }

    ~Timer() { timer_delete(*timer.get()); }

    // start timer
    void start(int pid, long int msec){
      struct itimerspec new_value;
      new_value.it_interval.tv_sec = new_value.it_interval.tv_nsec = 0;
      new_value.it_value.tv_sec=msec/1000;
      new_value.it_value.tv_nsec = (msec%1000)*1000000;
      if (timer_settime(*timer.get(), 0, &new_value, NULL)!=0)
         throw Err() << "IOFilter: can't start/stop POSIX timer";
    }

    // stop the timer
    void stop(){start(0, 0);}

    // get time in ms
    long int get(){
      struct itimerspec value;
      if (timer_gettime(*timer.get(), &value)!=0)
         throw Err() << "IOFilter: can't get value from POSIX timer";
      return value.it_value.tv_sec*1000
           + value.it_value.tv_nsec/1000000;
    }
};

#endif
