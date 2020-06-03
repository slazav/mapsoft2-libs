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

Downloader::Downloader(): max_conn(4), num_conn(0), worker_needed(1) {
  worker_mutex = new(Glib::Mutex);
  wakeup_cond = new(Glib::Cond);
  ready_cond = new(Glib::Cond);
  worker_thread =
    Glib::Thread::create(sigc::mem_fun(*this, &Downloader::worker), true);
  std::cerr << "Downloader: create thread\n";
}

Downloader::~Downloader(){
  worker_mutex->lock();
  worker_needed = false;
  wakeup_cond->signal();
  worker_mutex->unlock();

  worker_thread->join();
  delete(worker_mutex);
  delete(wakeup_cond);
  delete(ready_cond);
}

/**********************************/
// Add an URL to the downloading queue
void
Downloader::add(const std::string & url){
  if (data.count(url)) return;
  std::cerr << "Downloader: add url to queue: " << url << "\n";

  worker_mutex->lock();
  urls.push(url);
  status.emplace(url, 0);
  data.emplace(url, std::string());
  wakeup_cond->signal();
  worker_mutex->unlock();
}

std::string &
Downloader::get(const std::string & url){
  // put url in the queue if needed
  if (data.count(url) == 0) add(url);

  // wait for downloading
  worker_mutex->lock();
  while (status[url] < 2) {
    wakeup_cond->wait(*worker_mutex);
  }
  worker_mutex->unlock();

  // return result
  if (status[url] == 2)
    return data[url];
  else
    throw Err() << data[url];
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
  std::cerr << "Downloader: start worker thread\n";

  // Limit the amount of simultaneous connections
  curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)max_conn);

  do {

    // Add urls from queue for downloading
    while (num_conn<max_conn && urls.size()>0) {
      worker_mutex->lock();
      std::string &u = urls.front();
      status[u] = 1; // IN PROGRESS

      const char *url_ref = status.find(u)->first.c_str();
      void *data_ref = &(data.find(u)->second);

      CURL *eh = curl_easy_init();
      curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
      curl_easy_setopt(eh, CURLOPT_URL, url_ref);
      curl_easy_setopt(eh, CURLOPT_PRIVATE, data_ref);
      curl_easy_setopt(eh, CURLOPT_WRITEDATA, data_ref);
      curl_multi_add_handle(cm, eh);
      std::cerr << "Downloader: start downloading: " << u << "\n";
      urls.pop();
      num_conn++;
      worker_mutex->unlock();
    }

    worker_mutex->lock();
    curl_multi_perform(cm, &still_alive);
    while((msg = curl_multi_info_read(cm, &msgs_left))) {
      if(msg->msg == CURLMSG_DONE) {
        char *url;
        CURL *e = msg->easy_handle;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &url);
        void *str;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &str);
        std::cerr << "R: "<< msg->data.result << " - "
                  << curl_easy_strerror(msg->data.result)
                  << " <" << url << ">\n";
        if (msg->data.result==0) {
          status[url] = 2; // OK
        }
        else {
          status[url] = 3; // ERROR
          data[url] = curl_easy_strerror(msg->data.result);
        }
        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);
        wakeup_cond->signal();
      }
      else {
        // return some error?
        fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
      }
      num_conn--;
    }
    worker_mutex->unlock();

    // If queue is empty wait for wakeup_cond
    worker_mutex->lock();
    if (urls.empty() && !still_alive)
      wakeup_cond->wait(*worker_mutex);
    worker_mutex->unlock();

    if (still_alive)
      curl_multi_wait(cm, NULL, 0, 1000, NULL);

  } while(worker_needed);

  //std::cerr << "Downloader: stop worker thread\n";
  curl_multi_cleanup(cm);
  curl_global_cleanup();
}

