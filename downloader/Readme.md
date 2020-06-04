### Downloader class -- download files using libcurl

URLs can be added to the downloading queue (see `add` method).
Downloading process (a separate thread) gets URLs from the queue
and downloads them using multiple connections (see `max_conn` argument
of the costructor). Status of each url and downloaded data is stored
in the class. Following methods are available:

* `Downloader(const int max_conn=4)` -- constuctor.

* `void add(const std::string & url)` -- Add an URL to the downloading
queue. If the URL is already in the downloader do nothing.

* `void del(const std::string & url)` -- Remove an URL from the downloader.

* `void clear()` -- Remove all URLs from the downloader.

* `int get_status(const std::string & url)` -- Set current status of the
url: -1: unknown, 0: in the queue, 1: in progress, 2: done, 3: error.

* `int wait(const std::string & url)` -- For unknown urls return -1; For
others wait until status will be 2 (ok) or 3 (error) and return the
status.

* `std::string & get_data(const std::string & url)` -- Return downloaded
data if status is 2, throw relevant error otherwise.

* `std::string & get(const std::string & url)` -- High-level command:
combine add + wait + get_data commands.

