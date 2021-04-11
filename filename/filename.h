#ifndef FILENAME_H
#define FILENAME_H

#include <string>
#include <vector>

/* Check if the file has specified extension (case insensitive).
   Example: file_ext_check("file.jpg", ".jpg") => true
*/
bool file_ext_check(const std::string &fname, const char *ext);


/* Replace last file extension (if any) with a new one.
   Example: file_ext_repl("file.jpg", ".gif") => "file.gif"
*/
std::string file_ext_repl(const std::string &fname, const char *ext);

/* Extract directory names from a filename, a list which can be used to
   make/delete all dirs:
   Example: d1/d2/d3/../d4/f -> d1/d2/d3/../d4, d1/d2/d3, d1/d2, d1
*/
std::vector<std::string> file_get_dirs(const std::string &fname, const bool inverse = false);


/*
Calculate path, relative to ref_name (e.g. to put image name into .map or .fig file)
Examples (one makes a.map and a.png; what should be written in a.map?):
  a.png        b.map        -> a.png
  d1/d2/a.png  d1/d2/b.map  -> a.png
  d1/d2/a.png  d1/b.map     -> d2/a.png
  d1/a.png     d2/b.map     -> ../d1/a.png
  d1/a.png     d1/d2/b.map  -> ../a.png
  /d1/a.png    <anything>   -> /d1/a.png
  a.png        /d1/b.map    -> $(cwd)/a.png
*/
std::string file_rel_path(const std::string &fname, const std::string &ref_name);


/* Extract directory prefix from path */
std::string file_get_prefix(const std::string &fname);

/* Extract file name from path */
std::string file_get_name(const std::string &fname);

/* Extract file name without (known) extension from path */
std::string file_get_basename(const std::string &fname, const std::string & ext);

// check if the file exists
bool file_exists(const std::string & fname);

#endif
