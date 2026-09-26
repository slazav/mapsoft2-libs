///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <sstream>
#include "str_to_type.h"
#include "err/assert_err.h"

int
main(){
try{

   assert_eq(str_to_type<std::string>("123"), "123");

   assert_eq(str_to_type<int>("123"), 123);
   assert_eq(str_to_type<int>("-123"), -123);

  // assert_eq(str_to_type<int8_t>("1"), 0);
  // assert_eq(str_to_type<uint8_t>("1"), 0);

   assert_eq(str_to_type<int16_t>("0"), 0);
   assert_eq(str_to_type<int16_t>("32767"), 32767);
   assert_eq(str_to_type<int16_t>("-32767"), -32767);
   assert_eq(str_to_type<uint16_t>("65535"), 65535);

   assert_eq(str_to_type<int16_t>("32768"), -32768); // bad handling of signed values overflow
   assert_eq(str_to_type<int16_t>("32769"), -32767);
   assert_eq(str_to_type<int16_t>("-32769"), 32767);
   assert_err(str_to_type<int16_t>("65536"), "can't parse value: \"65536\""); // too big
   assert_eq(str_to_type<uint16_t>("-1"), 65535); // unsigned overflow

   // reading hex
   assert_eq(str_to_type<int>("0x1FF"),  0x1FF);
   assert_eq(str_to_type<int>("-0x1FF"), 0x1FF); // minus is lost!

   assert_eq(str_to_type<int16_t>("0x1FF"), 0x1FF);
   assert_eq(str_to_type<int16_t>("-0x1FF"), 0x1FF); // minus is lost!
   assert_eq(str_to_type<uint16_t>("-0x1FF"), 0x1FF); // minus is lost!
   assert_err(str_to_type<int16_t>("0x1FFFF"), "can't parse value: \"0x1FFFF\""); // too big

   assert_eq(str_to_type<int32_t>("0x1FFFF"), 0x1FFFF);
   assert_eq(str_to_type<int32_t>("-0x1FFFF"), 0x1FFFF); // minus is lost!
   assert_eq(str_to_type<uint32_t>("-0x1FFFF"), 0x1FFFF); // minus is lost!
   assert_err(str_to_type<int32_t>("0x1FFFFFFFF"), "can't parse value: \"0x1FFFFFFFF\""); // too big


  // ip
   assert_eq(str_to_type_ip4("127.0.0.1"), 0x7F000001u);
   assert_eq(str_to_type_ip4("255.255.255.255"), 0xFFFFFFFFu);
   assert_err(str_to_type_ip4("127.0.0."), "bad IP: unexpected end of output:127.0.0.");
   assert_err(str_to_type_ip4("127.0.0"), "bad IP: unexpected end of output:127.0.0");
   assert_err(str_to_type_ip4("1271.0.0.0"), "bad IP: number out of range: 1271.0.0.0");
   assert_err(str_to_type_ip4("256.0.0.0"), "bad IP: number out of range: 256.0.0.0");
   assert_err(str_to_type_ip4("127.1.1.1x"), "bad IP: extra characters at the end: 127.1.1.1x");

   // ivec
   assert_eq(str_to_type_ivec("") == std::vector<int>(), true);
   assert_eq(str_to_type_ivec(" ") == std::vector<int>(), true);
   assert_eq(str_to_type_ivec("1") == std::vector<int>({1}), true);
   assert_eq(str_to_type_ivec(" 1 ") == std::vector<int>({1}), true);
   assert_eq(str_to_type_ivec("1, 2,3") == std::vector<int>({1,2,3}), true);
   assert_eq(str_to_type_ivec(" 1, 2,3") == std::vector<int>({1,2,3}), true);
   assert_eq(str_to_type_ivec("1,3:5,7") == std::vector<int>({1,3,4,5,7}), true);
   assert_eq(str_to_type_ivec("1,5:3,7") == std::vector<int>({1,5,4,3,7}), true);
   assert_eq(str_to_type_ivec("1,5:6,-7,+7") == std::vector<int>({1,5,6,-7,7}), true);

   assert_err(str_to_type_ivec("1,5a"), "can't parse integer list: 1,5a");
   assert_err(str_to_type_ivec("1a,5"), "can't parse integer list: 1a,5");
   assert_err(str_to_type_ivec(",1,5"), "can't parse integer list: ,1,5");
   assert_err(str_to_type_ivec("1,5,"), "can't parse integer list: 1,5,");
   assert_err(str_to_type_ivec("1,5:5,-2"), "can't parse empty range: 1,5:5,-2");

   // dvec
   assert_eq(str_to_type_dvec("") == std::vector<double>(), true);
   assert_eq(str_to_type_dvec(" ") == std::vector<double>(), true);
   assert_eq(str_to_type_dvec("1") == std::vector<double>({1}), true);
   assert_eq(str_to_type_dvec(" 1 ") == std::vector<double>({1}), true);
   assert_eq(str_to_type_dvec("1, 2,3") == std::vector<double>({1,2,3}), true);
   assert_eq(str_to_type_dvec(" 1, 2,3") == std::vector<double>({1,2,3}), true);
   assert_eq(str_to_type_dvec("1.1, 2E-1,-3.5e-4") == std::vector<double>({1.1,2e-1,-3.5e-4}), true);
   assert_eq(str_to_type_dvec(" -1.2; 2.2;+3") == std::vector<double>({-1.2,2.2,3}), true);

   assert_err(str_to_type_dvec("1,5a"), "can't parse number list: 1,5a");
   assert_err(str_to_type_dvec("1a,5"), "can't parse number list: 1a,5");
   assert_err(str_to_type_dvec(",1,5"), "can't parse number list: ,1,5");
   assert_err(str_to_type_dvec("1,5,"), "can't parse number list: 1,5,");
   assert_err(str_to_type_dvec("1:5"), "can't parse number list: 1:5");

}
catch (Err & e) {
  std::cerr << "Error: " << e.str() << "\n";
  return 1;
}
return 0;
}

///\endcond
