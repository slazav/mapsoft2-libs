#ifndef FILENAME_H
#define FILENAME_H

#include <string>
#include <vector>
#include <list>

/* Check if the file has specified extension (case insensitive).
   Example: file_ext_check("file.jpg", ".jpg") => true
   Example: file_ext_check("file.jpg", {".jpg", ".jpeg"}) => true
*/
bool file_ext_check(const std::string &fname, const char *ext);
bool file_ext_check(const std::string &fname, const std::list<const char *> & ext);

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

/* Extract file name without (known) extension and all path componens */
std::string file_get_basename(const std::string &fname, const std::string & ext);

/* Extract file name without all extensions and path components */
std::string file_get_basename(const std::string &fname);

// check if the file exists
bool file_exists(const std::string & fname);

// delete file or directory
void file_remove(const std::string & fname);

// Check if file_src newer then file_dst.
// Returns true if:
//  - both files exist, modification time of file_src larger then that of file_dst,
//  - file_dst does not exist.
bool file_newer(const std::string & file_src, const std::string & file_dst);

// make directory if it does not exist
void file_mkdir(const std::string & name);

// list all files in the dir
std::vector<std::string> file_ls(const std::string & dir);

// find pathnames matching patterns
std::vector<std::string> file_glob(const std::vector<std::string> & patts, int glob_flags = 0);

#endif
