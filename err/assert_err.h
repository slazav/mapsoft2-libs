#ifndef ASSERT_ERR_H
#define ASSERT_ERR_H

///\addtogroup libmapsoft
///@{

#include <cmath> // fabs
#include "err.h"

// Check error.
// Note: cmd and ret should be evaluated once!
#define assert_err(cmd,ret)\
  {\
  std::string reta=(ret);\
  try{\
    cmd;\
    throw Err(-9999)\
      << "assert_err: " << __FILE__ << ":" << __LINE__ << ": error is not thrown:\n"\
      << "command: " << #cmd << "\n"\
      << "expected error: " << reta << "\n";\
  } catch (Err & e) {\
    if (e.code()==-9999) throw;\
    if (e.str()!=reta){\
      throw Err()\
        << "assert_err: " << __FILE__ << ":" << __LINE__ << ": wrong error message:\n"\
        << "command: " << #cmd << "\n"\
        << "expected error: " << reta << "\n"\
        << "actual error:   " << e.str()<< "\n";\
    }\
  }}

// Check that two values are equal.
// Note: v1,v2 should be evaluated once!
#define assert_eq(v1,v2)\
  {\
  auto v1a=(v1); auto v2a=(v2);\
  if (v1a != v2a){\
    throw Err()\
      << "assert_eq: " << __FILE__ << ":" << __LINE__ << ": arguments are not equal:\n"\
      << "v1: " << #v1 << "\n"\
      << "    " << v1a << "\n"\
      << "v2: " << #v2 << "\n"\
      << "    " << v2a << "\n";\
  }}

// Compare two double values and check that difference is less then e
// Note: v1,v2 should be evaluated once!
#define assert_feq(v1,v2,e)\
  {\
  auto v1a=(v1); auto v2a=(v2);\
  if (std::isnan(v1a-v2a) || fabs(v1a-v2a) > e){\
    throw Err()\
      << "assert_feq: " << __FILE__ << ":" << __LINE__ << ": arguments are not equal:\n"\
      << "v1: " << #v1 << "\n"\
      << "    " << v1a << "\n"\
      << "v2: " << #v2 << "\n"\
      << "    " << v2a << "\n";\
  }}

// Compare two objects with a dist(a,b) function, check thet the result is less then e.
// Note: v1,v2 should be evaluated once!
#define assert_deq(v1,v2,e)\
  {\
  auto v1a=(v1); auto v2a=(v2);\
  if (std::isnan(dist(v1a,v2a)) || dist(v1a,v2a) > e){\
    throw Err()\
      << "assert_feq: " << __FILE__ << ":" << __LINE__ << ": arguments are not equal:\n"\
      << "v1: " << #v1 << "\n"\
      << "    " << v1a << "\n"\
      << "v2: " << #v2 << "\n"\
      << "    " << v2a << "\n";\
  }}

///@}
#endif
