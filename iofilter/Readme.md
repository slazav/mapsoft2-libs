## iofilter

Filtering std::stream's through a program

#### IFilter

Problem: we have an istream. We want to read from this stream, not
directly but through some filter program.

```c++
IFilter flt(istr, prog); // run prog as a filter for stream istr
flt.stream() >> foo;     // read from filtered stream
```

```
istream -> [ program -> istream ] ->
           [ program -> istream ] ->
```

The filter program should exit when input data ends
(corresponding istream is not controlled by IFilter).
Destructor of IFilter just waits until it happens.

#### OFilter

Problem: we have an ostream. We want to write to this stream, not
directly but through some filter program.

```c++
OFilter flt(ostr, prog); // run prog as a filter for stream ostr
flt.stream() << foo;     // write to filtered stream
```

```
-> [ostream -> program ] -> ostream
-> [ostream -> program ]
```

The filter program should exit when input data ends. Destructor of
OFilter closes the stream (which is controlled by OFilter) and waits for
the child process.

#### IOFilter

Run a program and have access to its stdin and stdout:
```c++
IOFilter flt(prog); // run prog as a filter for stream ostr
flt.ostream() << foo;  // write to filtered stream
flt.close_input();     // close filter's input: we send what we wanted.
flt.stream() >> foo;   // read from filtered stream
```

```
-> [ostream -> program -> istream] ->
```

The filter program should exit when input data ends.
Destructor of IOFilter closes the stream and waits for the
child process. It is usually needed to close the filter input
stream without destroing IOFilter object (to finish reading from it).
This can be done with `close_input()` method.

### Timeout

Reading with timeout can be done using function:
```c++
int IOFilter::getline(std::string & line, double timeout);
```

Function returns number of bytes read of -1 in eof
On timeout and other errors an `Err()` exception will be thrown.

Killing a running filter program (by SIGTERM signal)
can be done using function:
```c++
void IOFilter::kill();
```


### SIGPIPE problem

If the filter is terminated the program will recieve SIGPIPE signal from
the kernel. To avoid program exit with errorcode 141 constructors
of all filter classes set SIGPIPE to SIG_IGN (ignore).

------------
## Changelog:

2020.09.12 V.Zavjalov 1.5:
- fix overflow error in OFilter

2019.11.24 V.Zavjalov 1.4:
- add timers to IOFilter
- ignore SIGPIPE to avoid main program exit when the filter program terminates

2019.11.21 V.Zavjalov 1.3:
- fix process handling; add close_input function;
- update documentation

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
