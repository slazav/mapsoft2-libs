#include <iostream>
#include <iomanip>
#include <malloc.h>
#include <map>

#include "err/assert_err.h"
#include "jsonxx.h"

int main() {
  try{

    // strings
    const char strmask[] = "{\"name\": \"teststr\","
                           " \"type\": \"string\","
                           " \"minlen\":3, \"maxlen\":6,"
                           " \"charset\": \"id\"}";
    Json("abc").validate(strmask);
    assert_err(Json("ab").validate(strmask), "teststr: string is too short");
    assert_err(Json("abcdefg").validate(strmask), "teststr: string is too long");
    assert_err(Json("abc&ef").validate(strmask), "teststr: only letters, numbers and _ are allowed");
    assert_err(Json("abc").validate("{\"type\": \"number\"}"), "unexpected string");
    assert_err(Json("abc").validate("[]"), "unexpected string");


    // numbers
    const char nummask[] = "{\"name\": \"testnum\","
                           " \"type\": \"number\","
                           " \"min\":3, \"max\":6}";

    Json(6).validate(nummask);
    Json(5.9).validate(nummask);
    assert_err(Json(6.1).validate(nummask), "testnum: number is too big");
    assert_err(Json(1).validate(nummask),  "testnum: number is too small");
    assert_err(Json(-10).validate(nummask), "testnum: number is too small");
    assert_err(Json(1e99).validate(nummask), "testnum: number is too big");
    assert_err(Json(10).validate(strmask), "unexpected number");

    // boolean and null - not too useful
    Json(true).validate("{\"type\": \"boolean\"}");
    Json(false).validate("{\"type\": \"boolean\"}");
    Json().validate("{\"type\": \"null\"}");

    // objects and arrays
    Json M1 = Json::load_string(
      "{\"name\": {\"type\":\"string\", \"charset\":\"id\","
      "            \"minlen\": 3, \"maxlen\": 6 },"
       " \"value\": {\"type\": \"number\", \"min\": 3, \"max\": 6},"
       " \"arr\": [{\"type\": \"number\", \"min\": 3, \"max\": 6}],"
       " \"sarr\": [{\"type\":\"string\", \"charset\": \"id\"}],"
       " \"obj\": {\"nn\": {\"type\":\"string\"}}}"
    );

    Json::load_string(
      "{\"name\": \"myname\","
      " \"value\": 5,"
      " \"arr\": [3, 4, 5],"
      " \"sarr\": [\"a\",\"b\"],"
      " \"obj\": {\"nn\": \"123\"}}"
    ).validate(M1);

    assert_err(Json::load_string("{\"uname\": \"myname\"}").validate(M1),
      "unexpected key: uname");

    assert_err(Json::load_string("{\"name\": [\"aaa\"]}").validate(M1),
      "unexpected array");

  }
  catch(Err & e){
    std::cerr << "error: " << e.str() << "\n";
  }
  return 0;
}
