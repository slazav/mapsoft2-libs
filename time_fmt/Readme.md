## time_fmt

In mapsoft unix time in ms is used.

Functions:

* `time_t parse_utc_time(const std::string & str)` --
  Convert UTC time to unix milliseconds (used in mapsoft).
  Time can be represented in different forms including ISO 8601 (used in GPX)
  and "yyyy-mm-dd HH:MM:SS".

* `std::string write_ozi_time(const time_t t)` --
   Convert mapsoft time to Ozi format (fractional number of days since 12/30/1899 12:00AM GMT).

* `time_t parse_ozi_time(const std::string & str)` --
   Convert Ozi time format to mapsoft time.

* `std::string write_fmt_time(const char *fmt, const time_t t)` -- Format time

Supported format sequences:
- %%  a literal %
- %n  a newline
- %t  a tab
- %Y  year
- %y  last two digits of year (00..99)
- %m  month (01..12)
- %d  day of month (e.g., 01)
- %H  hour (00..23)
- %M  minute (00..59)
- %S  second (00..60)
- %F  same as %Y-%m-%d
- %T  same as %H:%M:%S
- %a  abbreviated weekday name (e.g., Sun)
- %b  abbreviated month name (e.g., Jan)
- %s  seconds since 1970-01-01 00:00:00 UTC
- %f  fractional part of a second it it is non-zero (non-standard)


