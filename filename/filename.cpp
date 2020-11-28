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

std::vector<std::string>
file_get_dirs(const std::string &fname){
  std::vector<std::string> ret;
  std::string s = fname;
  std::string tail;

  while (1) {
    int i = s.rfind('/');
    if (i<=0) return ret;
    s = std::string(s.begin(), s.begin()+i);

    i = s.rfind('/');
    tail = std::string(s.begin()+i+1, s.end());
    if (tail!="." && tail!=".." && tail!="")
      ret.push_back(s);
  };
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
  auto dirs = file_get_dirs(ref_name);

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

#include <sys/types.h>
#include <sys/stat.h>
bool
file_exists(const std::string & fname){
  struct stat st_buf;
  return stat(fname.c_str(), &st_buf) == 0;
}

