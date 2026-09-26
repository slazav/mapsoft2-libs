#ifndef TYPE_TO_STR_H
#define TYPE_TO_STR_H

#include <map>
#include <string>
#include <cstdint>
#include <sstream>
#include <list>
#include <vector>
#include "err/err.h"

///\addtogroup libmapsoft
///@{

/// Convert any type to std::string (similar to boost::lexical_cast).
/// \relates Opt
template<typename T>
std::string type_to_str(const T & t){
  std::ostringstream ss;
  ss << t;
  return ss.str();
}

/// version for std::string, much simplier
template<>
std::string type_to_str<std::string>(const std::string & t);

/// version for hex values
/// \relates Opt
template<typename T>
std::string type_to_str_hex(const T & t){
  std::ostringstream ss;
  ss << std::hex << std::showbase << t;
  return ss.str();
}

// version for ip
std::string type_to_str_ip4(const uint32_t & v);

///@}
#endif
