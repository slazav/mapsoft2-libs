///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <sstream>
#include "type_to_str.h"
#include "err/assert_err.h"

int
main(){
try{

   assert_eq(type_to_str<const char *>("123"), "123");
   assert_eq(type_to_str<std::string>("123"), "123");
   assert_eq(type_to_str<int>(123), "123");
   assert_eq(type_to_str<unsigned int>(123), "123");

   assert_eq(type_to_str<char>('A'), "A");
   assert_eq(type_to_str<unsigned char>('A'), "A");

   assert_eq(type_to_str<int16_t>(0x7FFF), "32767");
   assert_eq(type_to_str<int16_t>(-32767), "-32767");
   assert_eq(type_to_str<uint16_t>(0xFFFF), "65535");

   assert_eq(type_to_str<int32_t>(0x7FFFFFFF), "2147483647");
   assert_eq(type_to_str<int32_t>(-2147483647), "-2147483647");
   assert_eq(type_to_str<uint32_t>(0xFFFFFFFF), "4294967295");

   assert_eq(type_to_str<int64_t>(0x7FFFFFFFFFFFFFFF), "9223372036854775807");
   assert_eq(type_to_str<int64_t>(-9223372036854775807), "-9223372036854775807");
   assert_eq(type_to_str<uint64_t>(0xFFFFFFFFFFFFFFFF), "18446744073709551615");

  // hex
//   assert_eq(type_to_str_hex<uint8_t>(0xFF), "0xFF");
   assert_eq(type_to_str_hex<uint16_t>(0xFFFF), "0xffff");
   assert_eq(type_to_str_hex<uint32_t>(0xFFFFFFFF), "0xffffffff");
   assert_eq(type_to_str_hex<uint64_t>(0xFFFFFFFFFFFFFFFF), "0xffffffffffffffff");

  // ip
   assert_eq(type_to_str_ip4(0xffffffffu), "255.255.255.255");
   assert_eq(type_to_str_ip4(0x7f000001u), "127.0.0.1");
   assert_eq(type_to_str_ip4(0), "0.0.0.0");

}
catch (Err & e) {
  std::cerr << "Error: " << e.str() << "\n";
  return 1;
}
return 0;
}

///\endcond
