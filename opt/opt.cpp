#include "opt.h"
#include <algorithm>
#include <jansson.h>

/**********************************************************/

// version for const char *
std::string
Opt::get (const std::string & key, const char *def) const {
  std::map<std::string, std::string>::const_iterator it = find(key);
  if (it == end()) return std::string(def);
  return it->second;
}

void
Opt::check_unknown (const std::list<std::string> & known) const {
  std::string unknown;
  int n=0;
  for (auto i : *this){
    if (std::find(known.begin(), known.end(), i.first) == known.end())
      unknown += (n++ ? ", ": " ") + i.first;
  }
  if (n){
    throw Err() << "unknown "
                << (n==1? "option:":"options:")
                << unknown;
  }
}

Opt
Opt::clone_known(const std::list<std::string> & known) const {
  Opt ret;
  for (auto i : *this){
    if (std::find(known.begin(), known.end(), i.first) != known.end())
      ret.insert(i);
  }
  return ret;
}

void
Opt::check_conflict(const std::list<std::string> & confl) const {
  std::string res;
  int n=0;
  for (auto const & i : confl){
    if (exists(i)) res += (n++ ? ", ": " ") + i;
  }
  if (n>1)
    throw Err() << "options can not be used together:" << res;
}

// input/output operators for options
std::ostream & operator<< (std::ostream & s, const Opt & o){
  json_t *J = json_object();
  for (auto i: o){
    json_object_set_new(J, i.first.c_str(), json_string(i.second.c_str()));
  }
  char *ret = json_dumps(J, JSON_SORT_KEYS);
  json_decref(J);
  if (!ret) throw Err() << "can't write Opt object";
  s<<ret;
  free(ret);
  return s;
}

std::istream & operator>> (std::istream & s, Opt & o){

  // read the whole stream into a string
  std::ostringstream os;
  s>>os.rdbuf();
  std::string str=os.str();

  json_error_t e;
  json_t *J = json_loadb(str.data(), str.size(), 0, &e);

  o.clear(); // clear old contents
  if (!J)
    throw Err() << "JSON error: " << e.text;
  try {
    if (!json_is_object(J))
      throw Err() << "Reading Opt: a JSON object with string fields expected";

    const char *k;
    json_t *v;
    json_object_foreach(J, k, v){
      if (!json_is_string(v))
        throw Err() << "Reading Opt: a JSON object with string fields expected";
      o[k] = json_string_value(v);
    }
  }
  catch (Err & e){
    json_decref(J);
    throw;
  }
  json_decref(J);
  return s;
}
