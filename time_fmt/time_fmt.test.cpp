///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include "time_fmt.h"
#include "err/err.h"
#include "err/assert_err.h"

int
main(){
  try{
     assert_eq(write_fmt_time("%FT%T%fZ", 0), "1970-01-01T00:00:00Z");
     assert_eq(write_fmt_time("%FT%T%fZ", 1533473911000), "2018-08-05T12:58:31Z");
     assert_eq(write_fmt_time("%FT%T%fZ", 1533473911001), "2018-08-05T12:58:31.001Z");
     assert_eq(write_fmt_time("%FT%T%fZ", 1533473911123), "2018-08-05T12:58:31.123Z");

     // On i586 arch we have 2038-year problem. Valid time range is 1901...2038:
     assert_eq(write_fmt_time("%FT%T%fZ", +(int64_t)0x7FFFFFFF*1000), "2038-01-19T03:14:07Z");
     assert_eq(write_fmt_time("%FT%T%fZ", -(int64_t)0x7FFFFFFF*1000), "1901-12-13T20:45:53Z");

     assert_eq(parse_utc_time("2018/08/05	12:58:31.1"), 1533473911100);

     assert_err(parse_utc_time("1970-01-01T00:00:00Z a"),
       "Unsupported time format: \"1970-01-01T00:00:00Z a\"");

     assert_eq(parse_utc_time("1970"), 0);
     assert_eq(parse_utc_time("1971"), (int64_t)365*3600*24*1000);
     assert_eq(parse_utc_time("1970-01"), 0);
     assert_eq(parse_utc_time("1970-02"), (int64_t)31*3600*24*1000);
     assert_eq(parse_utc_time("1970-01-01"), 0);
     assert_eq(parse_utc_time("1970-01-02"), 3600*24*1000);

     assert_eq(parse_utc_time("1970-01-01T00:00:00Z"), 0);
     assert_eq(parse_utc_time("2018-08-05T12:58:31Z"), 1533473911000);
     assert_eq(parse_utc_time("2018-08-05T12:58:31.001Z"), 1533473911001);
     assert_eq(parse_utc_time("2018-08-05T12:58:31.123Z"), 1533473911123);
     assert_eq(parse_utc_time("1901-12-13T20:45:53Z"), -(int64_t)0x7FFFFFFF*1000);
     assert_eq(parse_utc_time("2038-01-19T03:14:07Z"), +(int64_t)0x7FFFFFFF*1000);

     //assert_eq(parse_utc_time("2455-12-09T09:45:10Z"), 15334739110000); // only 64-bit arch

     assert_eq(parse_utc_time("2018/08/05T12:58:31"), 1533473911000);
     assert_eq(parse_utc_time("2018-08-05 12:58:31Z"), 1533473911000);
     assert_eq(parse_utc_time("2018-08-05 12:58:31"), 1533473911000);
     assert_eq(parse_utc_time("2018-08-05 12:58:31 "), 1533473911000);
     assert_eq(parse_utc_time("2018/08/05 12:58:31 	"), 1533473911000);
     assert_eq(parse_utc_time("2018/08/05	12:58:31 "), 1533473911000);
     assert_eq(parse_utc_time("2018/08/05	12:58:31.001 "), 1533473911001);
     assert_eq(parse_utc_time("2018/08/05	12:58:31.1"), 1533473911100);

     assert_eq(parse_utc_time("2019-05-20T02:00:00+02:00"), 1558310400000);
     assert_eq(parse_utc_time("2019-05-20T02:00:00+02"), 1558310400000);
     assert_eq(parse_utc_time("2019-05-20T02:00:00+2"), 1558310400000);
     assert_eq(parse_utc_time("2019-05-20T02:00:00+2:30"), 1558308600000);
     assert_eq(parse_utc_time("2019-05-20T02:00:00-2:30"), 1558326600000);


     assert_eq(parse_utc_time(write_fmt_time("%FT%T%fZ", 0)), 0);
     assert_eq(parse_utc_time(write_fmt_time("%FT%T%fZ", 123456789)), 123456789);
     assert_eq(parse_utc_time(write_fmt_time("%FT%T%fZ", 123456789000)), 123456789000);

     assert_eq( write_ozi_time(-2209161600000), "0.0000000");
     assert_eq( write_ozi_time(0), "25569.0000000");
     assert_eq( write_ozi_time(1533473911001), "43317.5406366");
     assert_eq( write_ozi_time(15334739115959), "203054.4064347");

     assert_eq( parse_ozi_time(write_ozi_time(-2209161600000)), -2209161600000);
     assert_eq( parse_ozi_time(write_ozi_time(0)), 0);

     // Accuracy of ozi format: 1e-7 * 3600*24 = 8.6ms
     assert( abs(parse_ozi_time(write_ozi_time(1533473911001)) - 1533473911001) < 10);
     assert( abs(parse_ozi_time(write_ozi_time(15334739115959)) - 15334739115959) < 10);


     // write_fmt_time
     int64_t t1 = parse_utc_time("2018/08/05 12:58:31.01");
     int64_t t2 = parse_utc_time("2018/08/05 12:58:31");
     assert_eq(write_fmt_time("%Y-%m-%d %H:%M:%S%f", t1), "2018-08-05 12:58:31.010");
     assert_eq(write_fmt_time("%Y-%m-%d %H:%M:%S%f", t2), "2018-08-05 12:58:31");
     assert_eq(write_fmt_time("%y", t1), "18");
     assert_eq(write_fmt_time("%F", t1), "2018-08-05");
     assert_eq(write_fmt_time("%T", t1), "12:58:31");
     assert_eq(write_fmt_time("", t1), "");
     assert_eq(write_fmt_time("a%", t1), "a");
     assert_eq(write_fmt_time("%%%t%n", t1), "%\t\n");
     assert_eq(atoi(write_fmt_time("%s", t1).c_str()), t1/1000);

     t1 = parse_utc_time("2019/05/23 12:00:00");
     assert_eq(write_fmt_time("%a", t1), "Thu");
     assert_eq(write_fmt_time("%b", t1), "May");

     // incomplete date
     assert_eq(parse_utc_time("2019/05/23 12:10:00"), parse_utc_time("2019/05/23 12:10"));
     assert_eq(parse_utc_time("2019/05/23 12:00:00"), parse_utc_time("2019/05/23 12"));
     assert_eq(parse_utc_time("2019/05/23 00:00:00"), parse_utc_time("2019/05/23"));
     assert_eq(parse_utc_time("2019/05/01 00:00:00"), parse_utc_time("2019/05"));
     assert_eq(parse_utc_time("2019/01/01 00:00:00"), parse_utc_time("2019"));

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
