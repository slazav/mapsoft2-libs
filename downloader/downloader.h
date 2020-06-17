#ifndef DOWNLOADER_H
#define DOWNLOADER_H

/*
Download manager. Download files using libcurl, use parallel
downloading. Results are stored in memory.

Usecases:

1. get() an URL, use data, del() the URL (data will be destroyed)

2. add() all needed URLs for parallel downloading,
   get() them, use data
   del() all URLs one by one, or clear() all of them

3. Use clean_list mechanism:
- update_clean_list method removes all URLs mentioned in
  the clean_list, and adds all known URLs to the clean list.
- add() method adds an URL to the downloading queue and removes
  it from the clean list.

Using clean_list mechanism:
- add() all URLs which will be needed.
- run update_clean_list(). All data except added since last call to
  update_clean_list is removed
- get() and use URLs
*/

#include <string>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class Downloader {
  private:
    int max_conn; // number of parallel connections
    int num_conn; // current number of connections

    // url -> status (0:waiting, 1: in progress, 2:ok, 3:error)
    // Also used to store url string for libcurl
    std::map<std::string, int> status;

    // clean list (accessed only from main thread)
    std::map<std::string, int> clean_list;

    // url -> data
    std::map<std::string, std::string> data;

    // queue for downloading
    std::queue<std::string> urls;

    bool worker_needed; // flag used to stop the second thread
    std::thread worker_thread;
    std::mutex data_mutex;
    std::unique_lock<std::mutex> lk; // lock for the main thread
    std::condition_variable add_cond; // notify worker_thread about adding new URL
    std::condition_variable ready_cond; // notify the main thread when data is ready

  public:

  Downloader(const int max_conn=4);
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


  // Update clean_list:
  // - remove all urls which are in the clean_list
  // - add all known urls to the clean_list
  void update_clean_list();

  private:
    // the separate thread for downloading
    void worker();

};

#endif
