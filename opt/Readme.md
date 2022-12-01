-----------------
## Opt class

Opt class is a `map<string,string>` container with functions for
getting/putting values of arbitrary types. Data types should have `<<`,
`>>` operators and a constructor without arguments.

- Converting any object to `std::string` and back:
```c
std::string str = type_to_str(v);
type v = str_to_type<type>(str);
```
Here `v` is some object of type `type`. All types except `std::string` are
passed through `std::stringstream` and `<<` or `>>` operators. For type `int`
it is possible to read hex values (prefixed by `0x`). For writing hex values into
string one can use function
```c
str = type_to_str_hex(int_v);
```

For IP4 addresses there are functions:
```c
string type_to_str_ip4(int32_t);
int32_t str_to_type_ip4(string);
```

- Creating Opt class, putting and extracting values:
```c
Opt o;
o.put<type>("key", v);
o.put_hex("key", int_v);
v = o.get<type>("key", def);
```
Here `"key"` is a string key which is used to access data.
If no value is set for this key, the `def` object is returned.

- Merging options (adding options from opts1, replacing old values):
```c
opts.put(opts1)
```

- Adding missing values (to add defaults to axisting options)

```c
opts.put_missing("key", v);
```

- Adding missing values from another Opt object:
```c
opts.put_missing(opts1);
```

- Find unknown options. List of known option names should be provided.
```opts.check_unknown (const std::list<std::string> & known) const```

- Return Opt object only with known elements (specified in the list).
```Opt opts.clone_known (const std::list<std::string> & known) const```

- Find conflicting options. List of conflicting option names should be provided.
```opts.check_conflict(const std::list<std::string> & confl) const```

Opt object can be converted to a string and back (and thus used inside Opt class).
String representation is a JSON object with string fields.
`Opt` object can be constructed from a JSON string:
```
Opt o("{\"k1\":\"v1\", \"k2\":\"v2\"}");
```


-----------------
## Changelog:

2021.12.01 V.Zavjalov 1.11:
- add str_to_type_ivec() - parsing lists of integers ("1,2, -3, 5:10, 12:1").
  Using ',' as separator, ':' as range separator

2020.09.30 V.Zavjalov 1.10:
- add str_to_type_ip4(), type_to_str_ip4() - parsing IP4

2020.09.29 V.Zavjalov 1.9:
- Add put_missing() methods -- Adding missing values.

2020.08.18 V.Zavjalov 1.8:
- Add clone_known() method -- return Opt object only with known elements

2019.11.01 V.Zavjalov 1.7.1:
- Add quotes in error messages

2019.10.22 V.Zavjalov 1.7:
- Convert hex numbers to different integer types (u?int{16,32,64}_t).
  If number is too large for the type an error is thrown.
  Example: opt.get<uint16_t>("name") will read "0xFFFF"
  but not "0xFFFFF".
- Set default template parameter of Opt::get to std::string
  (now one can simply use opt.get("name");)

2019.09.25 V.Zavjalov 1.6:
- allow reading large hex numbers (0xFFFFFFFF) as signed integer

2019.09.02 V.Zavjalov 1.5.1:
- More descriptive error message

2019.08.16 V.Zavjalov 1.5:
- Add constructor with string argument
  Opt("{\"k1\":\"v1\", \"k2\":\"v2\"}");

2019.08.13 V.Zavjalov 1.5:
- add check_conflict() method

2019.05.24 V.Zavjalov 1.4:
- allow const char* default value in opt.get()

2019.05.20 V.Zavjalov 1.3:
- put(Opt) method for merging options

2018.09.17 V.Zavjalov 1.2:
- support for hex integers (with 0x prefix)

2018.08.07 V.Zavjalov 1.1
- fix error with string values
- fix interface

2018.04.08 V.Zavjalov 1.0:
- extract from original mapsoft package
