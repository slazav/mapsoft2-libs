### Downloader class -- download files using libcurl

URLs can be added to the downloading queue (see `add` method).
Downloading process (a separate thread) gets URLs from the queue
and downloads them using multiple connections (see `max_conn` argument
of the costructor). Status of each url and downloaded data is stored
in the class. Following methods are available:

* `Downloader(cache_size=64, max_conn=4)` -- Constructor.

* `add(url)` -- Add an URL to the downloading queue.

* `del(url)` -- Remove an URL from downloader.

* `clear()`  -- Clear all data.

* `get_status(url)` -- Get current status of the url:
  `-1`: unknown, `0`: in the queue, `1`: in progress, `2`: done, `3`: error.

* `wait(url)` -- For unknown urls return -1; For others wait until
   status will be 2 (ok) or 3 (error) and return the status.

* `get_data(url)` -- Return downloaded data if status is 2, throw error otherwise.

* `get(url)` -- High-level command: combine add + wait + get_data methods.

