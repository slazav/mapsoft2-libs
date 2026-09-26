#ifndef STR_TO_TYPE_H
#define STR_TO_TYPE_H

#include <string>
#include <cstdint>
#include <sstream>
#include <vector>
#include "err/err.h"

///\addtogroup libmapsoft
///@{

/// Convert std::string to any type (similar to boost::lexical_cast).
/// \relates Opt
template<typename T>
T str_to_type(const std::string & s){
  std::istringstream ss(s);
  T val;
  ss >> std::showbase >> val;
  if (ss.fail() || !ss.eof())
    throw Err() << "can't parse value: \"" << s << "\"";
  return val;
}

// version for std::string, much simplier
template<>
std::string str_to_type<std::string>(const std::string & s);

// version for int, supports HEX values (starting with 0x)
template<>
int str_to_type<int>(const std::string & s);

// Version for vector<int>, supports HEX values (starting with 0x)
// Use ',' or ';' as separators, ':' as range separators.
std::vector<int> str_to_type_ivec(const std::string & s);

// Version for vector<double>, separator is "," or ";".
std::vector<double> str_to_type_dvec(const std::string & s);

// parsing ip
uint32_t str_to_type_ip4(const std::string & s);

///@}
#endif
