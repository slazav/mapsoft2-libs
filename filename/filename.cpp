#include <cstring>
#include <string>
#include <vector>
#include "err/err.h"

bool
file_ext_check(const std::string &fname, const char *ext){
  int lf=fname.size();
  int le=strlen(ext);
  return (lf>=le) && (strncasecmp(fname.c_str() + (lf-le), ext, le)==0);
}

std::string
file_ext_repl(const std::string &fname, const char *ext){
  int i = fname.rfind('.');
  std::string base = fname;
  if (i>=0) base = std::string(fname.begin(), fname.begin()+i);
  return base + ext;
}

#include <iostream>
#include <algorithm>

std::vector<std::string>
file_get_dirs(const std::string &fname, const bool inverse){
  std::vector<std::string> ret;
  std::string s = fname;
  std::string tail;

  while (1) {
    int i = s.rfind('/');
    if (i<=0) break;
    s = std::string(s.begin(), s.begin()+i);

    i = s.rfind('/');
    tail = std::string(s.begin()+i+1, s.end());
    if (tail!="." && tail!=".." && tail!="")
      ret.push_back(s);
  };
  if (inverse) std::reverse(ret.begin(), ret.end());
  return ret;
}

#include <unistd.h>
std::string
file_rel_path(const std::string &fname, const std::string &ref_name){
  // if filename is empty, or has absolute path, return it:
  if (fname.size()==0 || fname[0]=='/') return fname;

  // if reference is empty or contains absolute path, return absolute path for fname:
  if (ref_name.size()==0 || ref_name[0]=='/'){
     char * cwd = getcwd(NULL, 0);
     if (!cwd) throw Err() << "can't get cwd: " << strerror(errno);
     return std::string(cwd) + "/" + fname;
  }

  // reference dirs
  auto dirs = file_get_dirs(ref_name, 0);

  size_t i;
  for (i = 0; i < dirs.size(); ++i){
    auto d = fname.substr(0, dirs[i].size()+1);
    if (d == dirs[i] + "/") break;
  }

  std::string ret;
  for (size_t j = 0; j<i; ++j) ret += "../";
  if (i<dirs.size()) return ret += fname.substr(dirs[i].size()+1);
  else ret += fname;

  return ret;
}


std::string
file_get_prefix(const std::string &fname){
  int i = fname.rfind('/');
  return i<0? "" : std::string(fname.begin(), fname.begin()+i+1);
}

std::string
file_get_name(const std::string &fname){
  int i = fname.rfind('/');
  return i<0? fname : std::string(fname.begin()+i+1, fname.end());
}

std::string
file_get_basename(const std::string &fname, const std::string &ext){
  // remove path:
  auto ret = file_get_name(fname);

  // remove extension
  if (ret.size() >= ext.size() &&
      ret.substr(ret.size()-ext.size()) == ext)
    ret = ret.substr(0, ret.size()-ext.size());
  return ret;
}

std::string
file_get_basename(const std::string &fname){
  // remove path:
  auto ret = file_get_name(fname);

  // remove all extensions
  int i = ret.find('.', 1);
  return i<0? ret : ret.substr(0,i);
}

#include <sys/types.h>
#include <sys/stat.h>
bool
file_exists(const std::string & fname){
  struct stat st_buf;
  return stat(fname.c_str(), &st_buf) == 0;
}

bool
file_newer(const std::string & file_src, const std::string & file_dst){
  struct stat st_buf1, st_buf2;
  if (stat(file_src.c_str(), &st_buf1) != 0) return false;
  if (stat(file_dst.c_str(), &st_buf2) != 0) return true;
  auto ts1 = st_buf1.st_mtim.tv_sec;
  auto tn1 = st_buf1.st_mtim.tv_nsec;
  auto ts2 = st_buf2.st_mtim.tv_sec;
  auto tn2 = st_buf2.st_mtim.tv_nsec;

  if (ts1==ts2) return tn1 > tn2;
  return ts1 > ts2;
}
