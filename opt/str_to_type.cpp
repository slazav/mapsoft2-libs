#include "str_to_type.h"

/**********************************************************/

template<>
std::string str_to_type<std::string>(const std::string & s){ return s; }

// parse dec/hex numbers (internal, only for unsigned types)
template<typename T>
T str_to_type_hex(const std::string & s){
  std::istringstream ss(s);
  T val; ss >> val;
  if (!ss.eof()){
    char c; ss>>c;
    if (val!=0 || c!='x')
      throw Err() << "can't parse value: \"" << s << "\"";
    ss >> std::hex >> val;
  }
  if (ss.fail() || !ss.eof())
    throw Err() << "can't parse value: \"" << s << "\"";
  return val;
}

std::vector<int>
str_to_type_ivec(const std::string & s){
  std::istringstream ss(s);
  std::vector<int> ret;
  bool range=false;
  while (1){
    char sep;
    int n;
    ss >> std::ws;
    if (ss.eof()) break;

    ss >> n >> std::ws;
    if (ss.bad()) throw Err()
      << "can't parse integer list: " << s;

    if (range && ret.size()>0){
      auto p = *ret.rbegin();
      if (p==n) throw Err()
        << "can't parse empty range: " << s;
      auto st = n>p? +1:-1;
      for (int i=p+st; i!=n; i+=st) ret.push_back(i);
    }
    ret.push_back(n);
    if (ss.eof()) break;

    ss >> sep >> std::ws;
    if (!ss || ss.eof()) throw Err()
      << "can't parse integer list: " << s;
    if (sep==',' || sep==';') {range=false; continue; }
    if (sep==':') {range=true;  continue; }
    throw Err()
      << "can't parse integer list: " << s;
  }
  return ret;
}

std::vector<double>
str_to_type_dvec(const std::string & s){
  std::istringstream ss(s);
  std::vector<double> ret;
  while (1){
    char sep;
    double n;
    ss >> std::ws;
    if (ss.eof()) break;

    ss >> n >> std::ws;
    if (ss.bad()) throw Err()
      << "can't parse number list: " << s;

    ret.push_back(n);
    if (ss.eof()) break;

    ss >> sep >> std::ws;
    if (!ss || ss.eof()) throw Err()
      << "can't parse number list: " << s;
    if (sep==',' || sep==';') continue;
    throw Err() << "can't parse number list: " << s;
  }
  return ret;
}


// parse IP4, e.g. 127.0.0.1
uint32_t
str_to_type_ip4(const std::string & s){
  std::istringstream ss(s);
  char sep;
  uint32_t ret = 0;

  ss >> std::noskipws >> std::ws;
  for (int i=0; i<4; ++i){
    int v;
    ss >> v;
    if (!ss) throw Err()
      << "bad IP: unexpected end of output:" << s;

    if (v<0 || v>255) throw Err()
      << "bad IP: number out of range: " << s;

    ret = (ret<<8) | v;
    if (i==3) break;

    ss >> sep;
    if (sep!='.') throw Err()
      << "bad IP: expected . separator: " << s;
  }
  if (!ss.eof()) throw Err()
      << "bad IP: extra characters at the end: " << s;
  return ret;
}

// parse dec/hex numbers
template<>
int16_t str_to_type<int16_t>(const std::string & s){
  return (int16_t)str_to_type_hex<uint16_t>(s);}

template<>
uint16_t str_to_type<uint16_t>(const std::string & s){
  return str_to_type_hex<uint16_t>(s);}

template<>
int32_t str_to_type<int32_t>(const std::string & s){
  return (int32_t)str_to_type_hex<uint32_t>(s);}

template<>
uint32_t str_to_type<uint32_t>(const std::string & s){
  return str_to_type_hex<uint32_t>(s);}

template<>
uint64_t str_to_type<uint64_t>(const std::string & s){
  return str_to_type_hex<uint64_t>(s);}

template<>
int64_t str_to_type<int64_t>(const std::string & s){
  return (int64_t)str_to_type_hex<uint64_t>(s);}

