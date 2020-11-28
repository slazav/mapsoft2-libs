## filename -- functions for working with filenames

* Check if the file has specified extension (case insensitive).
```
bool file_ext_check(const std::string &fname, const char *ext);
```

Example:
``` c++
file_ext_check("file.jpg", ".jpg") => true
```

* Replace last file extension (if any) with a new one.
Example: file_ext_repl("file.jpg", ".gif") => "file.gif"
``` c++
std::string file_ext_repl(const std::string &fname, const char *ext);
```

* Extract directory names from a filename, a list which can be used to
 make/delete all dirs:
```c++
std::vector<std::string> file_get_dirs(const std::string &fname);
```
Example: d1/d2/d3/../d4/f -> d1/d2/d3/../d4, d1/d2/d3, d1/d2, d1

* Calculate path, relative to ref_name (e.g. to put image name into .map or .fig file)
```c++
std::string file_rel_path(const std::string &fname, const std::string &ref_name);
```

* Extract directory prefix from path:
```c++
std::string file_get_prefix(const std::string &fname);
```
Example: d1/d2/d3/../d4/f.ext -> d1/d2/d3/../d4/

* Extract file name from path:
```c++
std::string file_get_name(const std::string &fname);
```
Example: d1/d2/d3/../d4/f.ext -> f.ext

* Extract file name without (known) extension from path:
```c++
std::string file_get_basename(const std::string &fname, const std::string & ext);
``
Example: ("d1/d2/d3/../d4/f.ext", ".ext") -> f

* Check if file exists
```c++
bool file_exists(const std::string & fname);
```

