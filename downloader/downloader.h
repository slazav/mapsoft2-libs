#ifndef DOWNLOADER_H
#define DOWNLOADER_H

/*
Downloader class. Download files using libcurl, use parallel
downloading and cache.
*/

// https://curl.haxx.se/libcurl/c/10-at-a-time.html

#include <string>
#include <map>
#include <queue>
#include <glibmm.h>

class Downloader {
  private:
    int max_conn; // number of parallel connections
    int num_conn; // number of connections

    // url -> status (0:waiting, 1: in progress, 2:ok, 3:error)
    // Also used to store url data for libcurl
    std::map<std::string, int> status;

    // url -> data
    std::map<std::string, std::string> data;

    // queu for downloading
    std::queue<std::string> urls;

    bool worker_needed;
    Glib::Thread *worker_thread;
    Glib::Mutex  *worker_mutex;
    Glib::Cond   *wakeup_cond;  // to wake up worker thread after adding an url
    Glib::Cond   *ready_cond;   // to tell main thread that data is available 

  public:

  Downloader();
  ~Downloader();

  // Add an URL to the downloading queue
  void add(const std::string & url);

  // Start downloading if needed, wait until it
  // is finished, get result.
  std::string & get(const std::string & url);

  void worker();

};

#endif