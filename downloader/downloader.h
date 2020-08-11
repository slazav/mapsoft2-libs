#ifndef DOWNLOADER_H
#define DOWNLOADER_H

/*
Download manager. Download files using libcurl, use parallel
downloading. Results are stored in a cache with fixed size.

Interface:

  Downloader(cache_size=64, max_conn=4) -- Constructor.

  add(url) -- Add an URL to the downloading queue.

  del(url) -- Remove an URL from downloader.

  clear()  -- Clear all data.

  get_status(url) -- Get current status of the url:
    -1: unknown, 0: in the queue, 1: in progress, 2: done, 3: error.

  wait(url) -- For unknown urls return -1; For others wait until
     status will be 2 (ok) or 3 (error) and return the status.

  get_data(url) -- Return downloaded data if status is 2, throw error otherwise.

  get(url) -- High-level command: combine add + wait + get_data methods.

*/

#include <string>
#include <map>
#include <set>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "cache/cache.h"

class Downloader {
  private:
    int max_conn; // number of parallel connections
    int num_conn; // current number of connections

    // Data cache: url ->(status,data)
    // status values: 0: waiting, 1: in progress, 2: ok, 3: error
    Cache<std::string, std::pair<int, std::string> > data;

    // Queue for downloading. Added by add() function, processed by
    // the worker thread
    std::queue<std::string> urls;

    // Used in the worker thread to store URL for libcurl
    std::set<std::string> url_store;

    // Used in the worker thread to store data obtained from libcurl
    std::map<std::string, std::string> dat_store;

    bool worker_needed; // flag used to stop the second thread
    std::thread worker_thread;
    std::mutex data_mutex;
    std::condition_variable add_cond; // notify worker_thread about adding new URL
    std::condition_variable ready_cond; // notify the main thread when data is ready

    int log_level;        // log level (0 - no messages; 1 - only download results;
                          // 2 - adding/removing urls to queue; 3 - libcurl messages)
    std::string user_ag;  // user agent
    std::string http_ref; // http referer

  public:

  Downloader(const int cache_size=64, const int max_conn=4, const int log_level=1);
  ~Downloader();

  // Add an URL to the downloading queue.
  // If the URL is already in queue, downloading or finished, do nothing.
  void add(const std::string & url);

  // Remove an URL from queue and data cache.
  void del(const std::string & url);

  // Clear all data.
  void clear();

  // get current status of the url:
  // -1: unknown, 0: in the queue, 1: in progress, 2: done, 3: error
  int get_status(const std::string & url);

  // For unknown urls return -1; For others wait until
  // status will be 2 (ok) or 3 (error) and return the status
  int wait(const std::string & url);

  // Return downloaded data if status is 2, throw relevant error otherwise.
  std::string & get_data(const std::string & url);

  // High-level command: combine add + wait + get_data methods.
  std::string & get(const std::string & url);

  private:
    // the separate thread for downloading
    void worker();

};

#endif
