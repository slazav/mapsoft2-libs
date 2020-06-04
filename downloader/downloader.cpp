#include <iostream>

#include <curl/curl.h>
#include "downloader.h"
#include "err/err.h"

// Write callback for libcurl.
// Userdata is a pointer to std::string, where data should be appended
static size_t
write_cb(char *data, size_t n, size_t l, void *userp) {
  //std::cerr << "  get: " << (*(std::string *)userp).size() << " + " << n*l << "\n";
  *(std::string *)userp += std::string(data, n*l);
  return n*l;
}

/**********************************/
Downloader::Downloader(const int max_conn):
       max_conn(max_conn), num_conn(0), worker_needed(1),
       lk(data_mutex, std::defer_lock),
       worker_thread(&Downloader::worker, this) {
       //std::cerr << "Downloader: create thread\n";
}

Downloader::~Downloader(){
  lk.lock();
  worker_needed = false;
  lk.unlock();
  add_cond.notify_one();
  worker_thread.join();
}

/**********************************/
void
Downloader::add(const std::string & url){
  if (status.count(url)) return;
  lk.lock();
  urls.push(url);
  status.emplace(url, 0);
  data.emplace(url, std::string());
  lk.unlock();
  add_cond.notify_one();
  //std::cerr << "Downloader: add url to queue: " << url << "\n";
}

/**********************************/
void
Downloader::del(const std::string & url){
  if (status.count(url) == 0) return;
  lk.lock();
  // Remove all data with status 2 or 3.
  // Other data is used in the downloader and will be
  // deleted there.
  if (status[url] >= 2) data.erase(url);
  status.erase(url);
  lk.unlock();
}

/**********************************/
void
Downloader::clear(){
  lk.lock();
  // Remove all data with status 2 or 3.
  // Other data is used in the downloader and will be
  // deleted there.
  for (auto & s:status) if (s.second >= 2) data.erase(s.first);
  // remove all URLs from status hash
  status.clear();
  lk.unlock();
}

/**********************************/
int
Downloader::get_status(const std::string & url){
  if (status.count(url)==0) return -1;
  return status[url];
}

/**********************************/
int
Downloader::wait(const std::string & url){
  if (status.count(url)==0) return -1;
  lk.lock();
  while (status[url] < 2) ready_cond.wait(lk);
  lk.unlock();
  return status[url];
}


/**********************************/
std::string &
Downloader::get_data(const std::string & url){
  if (status.count(url)==0)
    throw Err() << "Downloader: unknown URL";
  switch (status[url]){
    case 0: throw Err() << "Downloader: URL in the downloading queue";
    case 1: throw Err() << "Downloader: downloading is in progress";
    case 2: return data[url];
    case 3: throw Err() << data[url];
    default: throw Err() << "Downloader: unknown status: " << status[url];
  }
}

/**********************************/
std::string &
Downloader::get(const std::string & url){
  add(url);
  wait(url);
  return get_data(url);
}

/**********************************/
void
Downloader::worker(){
  CURLMsg *msg;
  int msgs_left = -1;
  int still_alive = 1;

  // Create libcurl handler
  curl_global_init(CURL_GLOBAL_ALL);
  CURLM *cm = curl_multi_init();
  //std::cerr << "Downloader: start worker thread\n";

  // Limit the amount of simultaneous connections
  curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)max_conn);

  // lock for this thread
  std::unique_lock<std::mutex> lk(data_mutex, std::defer_lock);

  do {

    // Add urls from queue for downloading
    while (num_conn<max_conn && urls.size()>0) {
      lk.lock();
      std::string &u = urls.front();

      // url have been deleted with del()
      if (status.count(u) == 0){
        data.erase(u);
        urls.pop();
        lk.unlock();
        continue;
      }

      status[u] = 1; // IN PROGRESS

      const char *url_ref = status.find(u)->first.c_str();
      void *data_ref = &(data.find(u)->second);

      CURL *eh = curl_easy_init();
      curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
      curl_easy_setopt(eh, CURLOPT_URL, url_ref);
      curl_easy_setopt(eh, CURLOPT_PRIVATE, url_ref);
      curl_easy_setopt(eh, CURLOPT_WRITEDATA, data_ref);
      curl_multi_add_handle(cm, eh);
      //std::cerr << "Downloader: start downloading: " << u << "\n";
      urls.pop();
      num_conn++;
      lk.unlock();
    }

    lk.lock();
    curl_multi_perform(cm, &still_alive);
    lk.unlock();

    while((msg = curl_multi_info_read(cm, &msgs_left))) {

      if(msg->msg == CURLMSG_DONE) {
        char *url;
        CURL *e = msg->easy_handle;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
        //std::cerr << "R: "<< msg->data.result << " - "
        //          << curl_easy_strerror(msg->data.result)
        //          << " <" << url << ">\n";

        // data have not been deleted with del()
        if (status.count(url) != 0){
          if (msg->data.result==0) {
            lk.lock();
            status[url] = 2; // OK
            lk.unlock();
          }
          else {
            lk.lock();
            status[url] = 3; // ERROR
            data[url] = curl_easy_strerror(msg->data.result);
            data[url] += ": " + std::string(url);
            lk.unlock();
          }
        }
        else {
          lk.lock();
          data.erase(url);
          lk.unlock();
        }
        ready_cond.notify_one();
        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);
        num_conn--;
      }
      else {
        // should not happen? return some error?
        std::cerr << "E: CURLMsg: " << msg->msg << "\n";
      }
    }

    // If libcurl has finished and queue is empty wait for wakeup_cond
    lk.lock();
    if (urls.empty() && !still_alive)
      add_cond.wait(lk);
    lk.unlock();

    // Wait for libcurl
    if (still_alive)
      curl_multi_wait(cm, NULL, 0, 1000, NULL);

  } while(worker_needed);

  //std::cerr << "Downloader: stop worker thread\n";
  curl_multi_cleanup(cm);
  curl_global_cleanup();
}

