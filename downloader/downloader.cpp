#include <iostream>

#include <curl/curl.h>
#include "downloader.h"
#include "err/err.h"
#include "opt/opt.h"


void
ms2opt_add_downloader(GetOptSet & opts){
  const char *g = "DNLDR";
  opts.add("insecure",   1,0,g, "do not check TLS certificate (default: 0)");
  opts.add("user_agent", 1,0,g, "set user agent (default: \"mapsoft2 downloader\")");
  opts.add("http_ref",   1,0,g, "set http reference (default: \"https://github.com/slazav/mapsoft2\")");
}


// Write callback for libcurl.
// Userdata is a pointer to std::string, where data should be appended
static size_t
write_cb(char *data, size_t n, size_t l, void *userp) {
  *(std::string *)userp += std::string(data, n*l);
  return n*l;
}

/**********************************/
Downloader::Downloader(const int cache_size, const int max_conn, const int log_level):
       max_conn(max_conn), num_conn(0), log_level(log_level), worker_needed(true),
       data(cache_size) {
  set_opt(Opt());
  // worker_thread must not be started from initializer list
  worker_thread = std::thread(&Downloader::worker, this);
}

Downloader::~Downloader(){
  std::unique_lock<std::mutex> lk(data_mutex);
  worker_needed = false;
  lk.unlock();
  add_cond.notify_one();
  worker_thread.join();
}

void
Downloader::set_opt(const Opt & opts){
  insecure = opts.get("insecure", false);
  user_ag  = opts.get("user_agent", "mapsoft2 downloader");
  http_ref = opts.get("http_ref",   "https://github.com/slazav/mapsoft2");
}

/**********************************/
void
Downloader::add(const std::string & url){
  if (data.contains(url)) return; // already in the cache
  std::unique_lock<std::mutex> lk(data_mutex);
  data.add(url, std::make_pair(0, std::string()));
  urls.push(url);
  if (log_level>1)
    std::cerr << "Downloader: " << url << " (add to queue)\n";
  lk.unlock();
  add_cond.notify_one();
}

/**********************************/
void
Downloader::del(const std::string & url){
  if (!data.contains(url)) return;
  std::unique_lock<std::mutex> lk(data_mutex);
  data.erase(url);
  if (log_level>1)
    std::cerr << "Downloader: " << url << " (remove)\n";
}

/**********************************/
void
Downloader::clear(){
  std::unique_lock<std::mutex> lk(data_mutex);
  data.clear();
  if (log_level>1)
    std::cerr << "Downloader: clear all data\n";
}

/**********************************/
void
Downloader::clear_queue(){
  std::unique_lock<std::mutex> lk(data_mutex);
  for (auto i=data.begin(); i!=data.end(); i++)
    if (i->second.first < 2) i=data.erase(i);
  if (log_level>1)
    std::cerr << "Downloader: clear unfinished downloading\n";
}

/**********************************/
int
Downloader::get_status(const std::string & url){
  if (!data.contains(url)) return -1;
  return data.get(url).first;
}

/**********************************/
int
Downloader::wait(const std::string & url){
  if (!data.contains(url)) return -1;
  std::unique_lock<std::mutex> lk(data_mutex, std::defer_lock);
  lk.lock();
  while (data.contains(url) && data.get(url).first < 2) ready_cond.wait(lk);
  lk.unlock();
  return data.get(url).first;
}


/**********************************/
std::string &
Downloader::get_data(const std::string & url){
  if (!data.contains(url))
    throw Err() << "Downloader: unknown URL";
  auto & d = data.get(url);
  switch (d.first){
    case 0: throw Err() << "Downloader: URL in the downloading queue";
    case 1: throw Err() << "Downloader: downloading is in progress";
    case 2: return d.second;
    case 3: throw Err() << d.second;
    default: throw Err() << "Downloader: unknown status: " << d.first;
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
  if (log_level>1)
    std::cerr << "Downloader: start worker thread\n";

  // Limit the amount of simultaneous connections
  curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)max_conn);

  // lock for this thread
  std::unique_lock<std::mutex> lk(data_mutex, std::defer_lock);


  do {

    // Add urls from queue for downloading
    while (num_conn<max_conn) {

      lk.lock();
      if (urls.size() < 1) { lk.unlock(); break; }
      std::string &u = urls.front();

      // do nothing if
      // - url have been deleted with del()
      // - url is already in progress or ready
      if (get_status(u) != 0){
        urls.pop();
        lk.unlock();
        continue;
      }

      data.get(u).first = 1; // IN PROGRESS

      // url doubling (delete+add)
      if (url_store.count(u)>0){
        urls.pop();
        lk.unlock();
        continue;
      }

      // store some information for libcurl
      url_store.emplace(u);
      dat_store.emplace(u, std::string());
      const char *url_ref = url_store.find(u)->c_str();
      void *dat_ref = &(dat_store.find(u)->second);

      CURL *eh = curl_easy_init();
      curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
      curl_easy_setopt(eh, CURLOPT_URL, url_ref);
      curl_easy_setopt(eh, CURLOPT_PRIVATE, url_ref);
      curl_easy_setopt(eh, CURLOPT_WRITEDATA, dat_ref);
      curl_easy_setopt(eh, CURLOPT_USERAGENT, user_ag.c_str());
      curl_easy_setopt(eh, CURLOPT_REFERER, http_ref.c_str());
      curl_easy_setopt(eh, CURLOPT_VERBOSE, log_level>2);
      curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, insecure? 0L:1L);

      curl_multi_add_handle(cm, eh);
      if (log_level>1)
        std::cerr << "Downloader: " << u << " (start downloading)\n";
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
        long code; // HTTP response code
        CURL *e = msg->easy_handle;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
        curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
        //std::cerr << "R: "<< msg->data.result << " - "
        //          << curl_easy_strerror(msg->data.result)
        //          << " <" << url << ">\n";

        // data have not been deleted with del()
        lk.lock();
        if (data.contains(url)){
          auto & d = data.get(url);
          if (msg->data.result==0 && (code==200 || code==0)) {
            d.first  = 2; // OK
            d.second = dat_store[url];
            if (log_level>0)
              std::cerr << "Downloader: " << url
                        << " (OK, " << dat_store[url].size() << " bytes)\n";
          }
          else if (msg->data.result==0) {
            d.first  = 3; // ERROR
            d.second = "Get HTTP code " + type_to_str(code);
            if (log_level>0)
              std::cerr << "Downloader: " << url
                        << " (HTTP response code: " << code << ")\n";
          }
          else {
            d.first = 3; // ERROR
            d.second = curl_easy_strerror(msg->data.result);
            d.second += ": " + std::string(url);
            if (log_level>0)
              std::cerr << "Downloader: " << url
                        << " (failed: " << d.second << ")\n";
          }
        }

        lk.unlock();
        ready_cond.notify_one();
        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);
        num_conn--;

        // release information stored for libcurl
        dat_store.erase(url);
        url_store.erase(url);
      }
      else {
        // should not happen? return some error?
        std::cerr << "E: CURLMsg: " << msg->msg << "\n";
      }
    }

    // If libcurl has finished and queue is empty wait for wakeup_cond
    lk.lock();
    if (urls.empty() && !still_alive && worker_needed)
      add_cond.wait(lk);
    lk.unlock();

    // Wait for libcurl
    if (still_alive && worker_needed)
      curl_multi_wait(cm, NULL, 0, 1000, NULL);

  } while(worker_needed);

  if (log_level>1)
    std::cerr << "Downloader: stop worker thread\n";

  curl_multi_cleanup(cm);
  curl_global_cleanup();
}

