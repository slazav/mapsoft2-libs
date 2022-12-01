///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <sstream>
#include "opt.h"
#include "err/assert_err.h"

int
main(){
try{

  Opt O1;
  O1.put("int", 123);
  assert_eq( O1.get<int>("int"), 123 );
  assert_eq( O1.get<std::string>("int"), "123" );

  O1.put("int", "123a");
  assert_err(
    O1.get<int>("int"), "can't parse value: \"123a\"");

  O1.put("d", "123.1 ");
  assert_err(
    O1.get("d", 1.0), "can't parse value: \"123.1 \"");

  assert_eq(O1.exists("d"), true);
  assert_eq(O1.exists("e"), false);
  assert_eq(O1.find("d")->second, "123.1 ");

  O1.put<std::string>("d", "1 2 3 4 5");
  assert_eq(O1.get<std::string>("d", "1 2 3"), "1 2 3 4 5");
  assert_eq(O1.get<std::string>("x", "1 2 3"), "1 2 3");

  assert_eq(O1.get("d", "1 2 3"), "1 2 3 4 5");
  assert_eq(O1.get("x", "1 2 3"), "1 2 3");

  O1.put("d", "123.1 ");

  { // put_missing
    Opt O2;
    O2.put("a", 1);
    O2.put_missing("a", 2);
    assert_eq(O2.get("a",0), 1);
    O2.put_missing("b", 2);
    assert_eq(O2.get("b",0), 2);
    O2.put("x", 1);
    O2.put_missing(O1);
    assert_eq(O2.get("x",0), 1);
    assert_eq(O2.get("d",""), "123.1 ");
  }

  /////////////////////////////////////////////
  // check_unknown()
  std::list<std::string> k;

  k = {"int","d","a"};
  O1.check_unknown(k);

  k = {"a", "b"};
  assert_err(O1.check_unknown(k), "unknown options: d, int")

  k = {"a", "b", "d"};
  assert_err(O1.check_unknown(k), "unknown option: int");


  /////////////////////////////////////////////
  // clone_known()
  assert_eq(type_to_str(O1.clone_known({"a", "d", "x"})), "{\"d\": \"123.1 \"}");

  /////////////////////////////////////////////
  // check_conflict()
  k = {"int","b"};
  O1.check_conflict(k);
  k = {"b","c"};
  O1.check_conflict(k);

  k = {"a", "int", "d"};
  assert_err(O1.check_conflict(k), "options can not be used together: int, d");

  /////////////////////////////////////////////
  // dump and parse simple options:

  // put Opt inside Opt!
  O1.put("opts", O1);

  std::ostringstream os;
  os << O1;
  assert_eq(os.str(), "{\"d\": \"123.1 \", \"int\": \"123a\","
                     " \"opts\": \"{\\\"d\\\": \\\"123.1 \\\", \\\"int\\\": \\\"123a\\\"}\"}");

  std::istringstream is(os.str());
  Opt O2;
  O2.put("a", 1); // these fields should disappear
  O2.put("d", 2);
  is >> O2; // read O2 from is
  os.str(std::string()); // clear the stream
  os << O2;
  assert_eq(os.str(), "{\"d\": \"123.1 \", \"int\": \"123a\","
                     " \"opts\": \"{\\\"d\\\": \\\"123.1 \\\", \\\"int\\\": \\\"123a\\\"}\"}");

  os.str(std::string()); // clear the stream
  os << O2.get<Opt>("opts");
  assert_eq(os.str(), "{\"d\": \"123.1 \", \"int\": \"123a\"}");

  {
    std::istringstream is("{} ");
    is >> O1;
    assert_eq(O1.size(), 0);
  }

  // some error cases
  {
    std::istringstream is("[1,2,3]");
    assert_err(is >> O1, "Reading Opt: a JSON object with string fields expected");
  }

  {
    std::istringstream is("{");
    assert_err(is >> O1, "JSON error: string or '}' expected near end of file");
  }

  {
    std::istringstream is("{a: 1}");
    assert_err(is >> O1, "JSON error: string or '}' expected near 'a'");
  }

  {
    std::istringstream is("{b: \"2\"}");
    assert_err(is >> O1, "JSON error: string or '}' expected near 'b'");
  }

  O1.put("h1", "0xFF");
  O1.put_hex("h2", 254);
  assert_eq(O1.get("h1", std::string()), "0xFF");
  assert_eq(O1.get("h2", std::string()), "0xfe");

  assert_eq(O1.get("h1", 0), 255);
  assert_eq(O1.get("h2", 0), 254);

  Opt O3;
  O3.put("h1", 123);
  O3.put("h3", 124);
  O1.put(O3);
  assert_eq(O1.get<int>("h1"), 123);
  assert_eq(O1.get<int>("h2"), 254);
  assert_eq(O1.get<int>("h3"), 124);
  assert_eq(O1.get<int>("h4"), 0);

  Opt O4("{\"k1\":\"v1\", \"k2\":\"v2\", \"k3\":\"100\"}");
  assert_eq(O4.get("k1", std::string()), "v1");
  assert_eq(O4.get("k2", std::string()), "v2");
  assert_eq(O4.get("k3", 0), 100);

  O1.put("hex8",  "0xFF");
  O1.put("hex16", "0xFFFF");
  O1.put("hex32", "0xFFFFFFFF");
  O1.put("hex64", "0xFFFFFFFFFFFFFFFF");

  assert_eq(O1.get<uint16_t>("hex16",0), 0xFFFF);
  assert_eq(O1.get<uint32_t>("hex32",0), 0xFFFFFFFF);
  assert_eq(O1.get<uint64_t>("hex64",0), 0xFFFFFFFFFFFFFFFF);

  assert_eq(O1.get("hex8",0), 0xFF);
  assert_eq(O1.get("hex16",0), 0xFFFF);
  assert_eq(O1.get("hex32",0u), 0xFFFFFFFFu);

  assert_eq(O1.get("hex8"),  "0xFF");
  assert_eq(O1.get("hex16"), "0xFFFF");
  assert_eq(O1.get("hex32"), "0xFFFFFFFF");
  assert_eq(O1.get("hex64"), "0xFFFFFFFFFFFFFFFF");

  // signed types
  assert_eq(O1.get("hex8",0), (int)0xFF);
  assert_eq(O1.get<int16_t>("hex8",0), (int16_t)0xFF);
  assert_eq(O1.get<int16_t>("hex16",0), (int16_t)0xFFFF);
  assert_eq(O1.get<int32_t>("hex32",0), (int32_t)0xFFFFFFFF);
  assert_eq(O1.get<int64_t>("hex64",0), (int64_t)0xFFFFFFFFFFFFFFFF);

  assert_err(O1.get<uint8_t>("hex16",0), "can't parse value: \"0xFFFF\"");
  assert_err(O1.get<uint16_t>("hex32",0), "can't parse value: \"0xFFFFFFFF\"");
  assert_err(O1.get<uint32_t>("hex64",0), "can't parse value: \"0xFFFFFFFFFFFFFFFF\"");

//  assert_eq(O1.get("h1", 0.0), 255);
//  assert_eq(O1.get("h2", 0.0), 254);

  // ip
   assert_eq(str_to_type_ip4("127.0.0.1"), 0x7F000001u);
   assert_eq(str_to_type_ip4("255.255.255.255"), 0xFFFFFFFFu);
   assert_err(str_to_type_ip4("127.0.0."), "bad IP: unexpected end of output:127.0.0.");
   assert_err(str_to_type_ip4("127.0.0"), "bad IP: unexpected end of output:127.0.0");
   assert_err(str_to_type_ip4("1271.0.0.0"), "bad IP: number out of range: 1271.0.0.0");
   assert_err(str_to_type_ip4("256.0.0.0"), "bad IP: number out of range: 256.0.0.0");
   assert_err(str_to_type_ip4("127.1.1.1x"), "bad IP: extra characters at the end: 127.1.1.1x");

   assert_eq(type_to_str_ip4(0xffffffffu), "255.255.255.255");
   assert_eq(type_to_str_ip4(0x7f000001u), "127.0.0.1");
   assert_eq(type_to_str_ip4(0), "0.0.0.0");

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

}
catch (Err & e) {
  std::cerr << "Error: " << e.str() << "\n";
  return 1;
}
return 0;
}

///\endcond
