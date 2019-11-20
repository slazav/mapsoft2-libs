## iofilter

Filtering std::stream's through a program

IFilter:
```
istream -> [ program -> istream ] ->
           [ program -> istream ] ->
```

OFilter:
```
[ostream -> program ] -> ostream
[ostream -> program ]
```

IOFilter:
```
-> [ostream -> program -> istream] ->
```
------------
## Changelog:

2019.11.20 V.Zavjalov 1.2:
- add IOFilter class (get istream and ostream for
  program input/output)

2019.05.08 V.Zavjalov 1.1:
- add OFilter class (attach ostream to program's output,
  get ostream for its input).
- add "one-side" filters (run program and read its output,
  run program and write to its input)

2019.05.02 V.Zavjalov 1.0:
- First version:
  IFilter class (attach istream to program's input,
  get istream from its output).