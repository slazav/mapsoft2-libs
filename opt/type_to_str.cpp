#include "type_to_str.h"

template<>
std::string
type_to_str<std::string>(const std::string & t){
   return t;
}

std::string
type_to_str_ip4(const uint32_t & v){
  std::ostringstream ss;
  ss << ((v>>24)&0xff) << "."
     << ((v>>16)&0xff) << "."
     << ((v>>8)&0xff) << "."
     << (v&0xff);
 return ss.str();
}
