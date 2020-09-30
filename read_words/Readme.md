## read_words

Read one line from the stream and extract words, separated by spaces.

 - Comments (everything from # symbol to end of the line) are skipped.

 - Empty lines are skipped.

 - Words are splitted by SPACE or TAB symbols, or by '\' + NEWLINE sequence.

 - Words can be quoted by " or '.

 - ANSY escape sequences are supported: \a, \b, \f, \n, \r, \t, \v,
   \\, \", \'. In addition, symbols #, SPACE and TAB can be escaped.
   Escaped NEWLINE works as word separator.
   Escape sequences \<oct> and \x<hex> are supported. Octal number
   can contain 1..3 digits, hex number should always contain 2 digits.

 - Empty vector is returned only at the end of input.

 - If line_num parameter is not NULL, it should be initialized
   by 0,0 before the first call. Then starting line number of the first
   word and number of the last line read is returned there.
   In this error messages will contain line number.

 - If lc parameter is true then all Latin characters are converted to lower case.

```c++
std::vector<std::string> read_words(
   std::istream & ss, int line_num[] = NULL, const bool lc=false);
```

## read_conf

This is an example of using `read_words`.
Read options from a simple config file. All known options should be
mentioned in `known` list. If `should_exist=true` and file is missing
then error will be thrown.

Each line shold contain pairs of words:
 <parameter name> <parameter value>

```c++
Opt read_conf(const std::string & fname,
  std::list<std::string> known, bool should_exist=false);
```

------------
## Changelog:

2020.09.30 V.Zavjalov 1.5:
- read_conf function

2020.09.30 V.Zavjalov 1.4:
- Use ANSI escape sequencies (\n, \t, etc.).
- Do not allow escaping normal characters (A, c, etc.).
- Use \<oct> and \x<hex> sequences.
- Fix error in line counting.

2020.01.02 V.Zavjalov 1.3:
- symbol \n protected by \ works as word separator, not as literal \n

2019.10.23 V.Zavjalov 1.2:
- lc parameter affects only Latin characters.
  Reasons: tolower() anyway did not work with UTF characters,
  proper wide character reading is needed; together with Gtk::Main
  it produce unexpected conversions (Cyrrilic "г" to Cyrillic "У").

2019.06.01 V.Zavjalov 1.1:
- Do not stop reading if newlines is inside quotes.
- Add line counting (line_num parameter).
- Add option for case-insensetive reading (lc parameter).

2019.05.02 V.Zavjalov 1.0:
- First version
