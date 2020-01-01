#include <iostream>
#include <iomanip>
#include <malloc.h>
#include <map>

#include "err/assert_err.h"
#include "jsonxx.h"

int main() {
  try{

    // create various types of Json objects
    Json e0;                 // not an object
    Json e1("text");         // string
    Json e2(std::string("")); // string
    Json e3(10);             // int
    Json e4(10.);            // double
    Json e5(true);           // true
    Json e6(false);          // false

    // with static functions
    Json e7(Json::null());   // null
    Json e8(Json::object()); // empty object
    Json e9(Json::array());  // empty array
    Json eA(Json::load_string("{\"foo\": true, \"bar\": \"test\"}"));
    Json eB(Json::load_file("test_data/test.json"));  // from file

    Json eA1(eA);  // copy constructor
    Json eA2 = eA; // assignment

    // exceptions
    assert_err(Json::load_file(""), "unable to open : No such file or directory");
    assert_err(Json::load_file("jsonxx.test.cpp"), "'[' or '{' expected near '#'");
    assert_err(Json::load_string("{"), "string or '}' expected near end of file");

    // check types
    assert_eq(e0.type(), JSON_NULL);
    assert_eq(e1.type(), JSON_STRING);
    assert_eq(e2.type(), JSON_STRING);
    assert_eq(e3.type(), JSON_INTEGER);
    assert_eq(e4.type(), JSON_REAL);
    assert_eq(e5.type(), JSON_TRUE);
    assert_eq(e6.type(), JSON_FALSE);
    assert_eq(e7.type(), JSON_NULL);
    assert_eq(e8.type(), JSON_OBJECT);
    assert_eq(e9.type(), JSON_ARRAY);
    assert_eq(eA.type(), JSON_OBJECT);
    assert_eq(eB.type(), JSON_OBJECT);
    assert_eq(eA1.type(), JSON_OBJECT);
    assert_eq(eA2.type(), JSON_OBJECT);

    assert_eq(e0.is_null(), true);
    assert_eq((bool)e0, false);
    assert_eq(e0.is_object(), false);

    assert_eq(e1.is_string(), true);
    assert_eq(e1.is_number(), false);
    assert_eq(e1.is_null(), false);
    assert_eq((bool)e1, true);

    assert_eq(e2.is_string(), true);

    assert_eq(e3.is_integer(), true);
    assert_eq(e3.is_number(), true);
    assert_eq(e3.is_real(), false);

    assert_eq(e4.is_real(), true);
    assert_eq(e4.is_number(), true);
    assert_eq(e4.is_integer(), false);

    assert_eq(e5.is_true(), true);
    assert_eq(e5.is_bool(), true);
    assert_eq(e5.is_false(), false);
    assert_eq(e5.is_null(), false);
    assert_eq(e5.is_object(), false);

    assert_eq(e6.is_false(), true);
    assert_eq(e6.is_bool(), true);
    assert_eq(e6.is_true(), false);

    assert_eq(e7.is_null(), true);
    assert_eq((bool)e7, false);
    assert_eq(e7.is_object(), false);

    assert_eq(e8.is_object(), true);
    assert_eq(e9.is_array(), true);

    // size (non-zero for non-empty arrays and objects)

    assert_eq(e8.size(), 0);
    assert_eq(e9.size(), 0);
    assert_eq(eA.size(), 2);
    assert_eq(eB.size(), 1);
    assert_eq(eA1.size(), 2);
    assert_eq(eA2.size(), 2);

    // extract data from json

    //    Json e0;
    //    Json e1("text");
    //    Json e2(std::string(""));
    //    Json e3(10);
    //    Json e4(10.);
    //    Json e5(true);
    //    Json e6(false);

    assert_eq(e3.as_bool(), true);
    assert_eq(e5.as_bool(), true);
    assert_eq(e6.as_bool(), false);

    assert_eq(e3.as_integer(), 10);
    assert_eq(e4.as_integer(), 10);
    assert_eq(e5.as_integer(),  1);
    assert_eq(e6.as_integer(),  0);

    assert_eq(e3.as_real(), 10.);
    assert_eq(e4.as_real(), 10.);

    assert_eq(e0.as_string(), "");
    assert_eq(e1.as_string(), "text");
    assert_eq(e2.as_string(), "");
    assert_eq(e3.as_string(), "10");
    assert_eq(e4.as_string(), "10");
    assert_eq(e5.as_string(), "true");
    assert_eq(e6.as_string(), "false");

    // work with object

    e8.set("k0", e0);
    e8.set("k1", e1);
    e8.set("k2", e2);
    e8.set("k3", e3);
    e8.set("k4", e4);
    e8.set(std::string("k5"), e5);
    e8.set("k6", e6);
    e8.set("k7", e7);

    assert_eq(e8.exists("k0"), true);
    assert_eq(e8.exists("k1"), true);
    assert_eq(e8.exists("k2"), true);
    assert_eq(e8.exists("k3"), true);
    assert_eq(e8.exists("k4"), true);
    assert_eq(e8.exists("k5"), true);
    assert_eq(e8.exists("k6"), true);
    assert_eq(e8.exists("k7"), true);
    assert_eq(e8.exists(""), false);
    assert_eq(e8.exists("k8"), false);

    assert_eq(e8.size(), 8);
    assert_eq(e8["k0"].as_string(), "");
    assert_eq(e8["k1"].as_string(), "text");
    assert_eq(e8[std::string("k2")].as_string(), "");
    assert_eq(e8["k3"].as_string(), "10");
    assert_eq(e8["k5"].is_true(), true);
    assert_eq(e8["k6"].is_false(), true);

    assert_eq(e8.get("k5").is_true(), true); // get instead of []
    assert_eq(e8.get("k6").is_false(), true);

    e8.del("k0"); // del
    e8.del(std::string("k1"));
    assert_eq(e8["k1"].is_null(), true)
    assert_eq(e8.size(), 6);

    e8.update(eA); // update
    assert_eq(e8.size(), 8);
    assert_eq(e8["bar"].as_string(), "test");
    eA.set("bar", Json("test1"));
    eA.set("barx", Json("testx"));
    e8.update_existing(eA);
    assert_eq(e8["barx"].is_null(), true);
    assert_eq(e8["bar"].as_string(), "test1");
    eA.set("bar", Json("test2"));
    e8.update_missing(eA);
    assert_eq(e8["barx"].as_string(), "testx");
    assert_eq(e8["bar"].as_string(),  "test1");

    std::map<std::string, std::string> M;  // iterators
    for (Json::iterator i = e8.begin(); i!=e8.end(); i++)
      M[i.key()] = i.val().as_string();

    assert_eq(M["k2"], "");
    assert_eq(M["k3"], "10");
    assert_eq(M["k4"], "10");
    assert_eq(M["k5"], "true");
    assert_eq(M["k6"], "false");
    assert_eq(M["k7"], "");
    assert_eq(M["bar"],  "test1");
    assert_eq(M["barx"], "testx");
    assert_eq(M.size(), e8.size());

    for (Json::iterator i = e1.begin(); i!=e1.end(); i++)
      assert_eq(false, true);

    e8.clear();
    assert_eq(e8.size(), 0);


    e8.set("m1", true);
    e8.set("m2", 1);
    e8.set("m3", "12");
    e8.set("m4", std::string("12"));
    assert_eq(e8["m1"].is_true(),    true);
    assert_eq(e8["m2"].is_integer(), true);
    assert_eq(e8["m3"].is_string(),  true);
    assert_eq(e8["m4"].is_string(),  true);

    // work with array

    //    Json e0;
    //    Json e1("text");
    //    Json e2(std::string(""));
    //    Json e3(10);
    //    Json e4(10.);
    //    Json e5(true);
    //    Json e6(false);

    e9.append(e0); // ""
    e9.append(e0); // ""
    e9.append(e0); // ""

    e9.set(1,e1); // "text"
    e9.set(2,e2); // ""

    e9.append(e3); // 10
    e9.append(e4); // 10
    e9.insert(2, e5); // true
    e9.append(e6); // false

    assert_eq(e9.size(), 7);
    assert_eq(e9[(size_t)0].as_string(), "");
    assert_eq(e9[1].as_string(), "text");
    assert_eq(e9[2].as_bool(),   true);
    assert_eq(e9[3].as_string(), "");
    assert_eq(e9[4].as_string(), "10");
    assert_eq(e9.get(5).as_string(), "10");
    assert_eq(e9.get(6).as_string(), "false");

    e9.del(2);
    assert_eq(e9.size(), 6);
    assert_eq(e9.get(5).as_string(), "false");

    e9.extend(e9);
    assert_eq(e9.size(), 12);
    assert_eq(e9.get(11).as_string(), "false");

    e9.clear();
    assert_eq(e9.size(), 0);

    // dump to string
    const char * exp1 = "{\"foo\": true, \"bar\": \"test2\", \"barx\": \"testx\"}";
    assert_eq(eA.save_string(JSON_PRESERVE_ORDER), exp1);

  } catch(Err e){
    std::cerr << "error: " << e.str() << "\n";
  }
  return 0;
}
