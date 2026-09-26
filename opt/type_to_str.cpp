#include "type_to_str.h"

template<>
std::string type_to_str(const uint8_t & t){
  std::ostringstream ss;
  ss << (int)t;
  return ss.str();
}

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

template<>
std::string
type_to_str_hex(const uint8_t & t){
  const char *hex = "0123456789abcdef";
  std::ostringstream ss;
  ss << "0x";
  ss << hex[t>>4];
  ss << hex[t & 0xF];
  return ss.str();
}
