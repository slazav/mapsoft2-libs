## iconv

Wrapper for libiconv.

Usage:
```c++
IConv cnv("UTF8", "KOI8-R");
std::string s = cnv(text);
```
-----------
## Changelog:

2019.09.02 V.Zavjalov 1.5:
- Use std::string constructor arguments
  instead of const char*

2019.05.20 V.Zavjalov 1.4:
- Use operator() instead of cnv method.
  Allow to use IConv as a constant object.

2019.05.17 V.Zavjalov 1.3:
- Allow trivial conversion
  (contructor without arguments).

2019.05.01 V.Zavjalov 1.2:
- Use shared_ptr for memory handling.
- Use "pipl" to hide iconv interface.

2018.04.09 V.Zavjalov 1.1:
- Hide system iconv.h from the interface.

2018.04.08 V.Zavjalov 1.0:
- First version, taken from mapsoft.
