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
#include <thread>
#include <mutex>
#include <condition_variable>

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
    std::thread worker_thread;
    std::mutex data_mutex;
    std::unique_lock<std::mutex> lk; // lock for the main thread
    std::condition_variable data_cond;

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