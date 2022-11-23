## getopt -- mapsoft getopt wrapper.

parse_options() allows to build a command line interface with many
option groups:
```
program [<general_options>|<global_input_options>]\
        <input_file_1> [<input_options_1>]\
        <input_file_2> [<input_options_2>] ...\
        (--out|-o) <output_file> [<output_options>]
```
(example can be found in `getopt.test.cpp`)

Structure `GetOptEl` is used as an extention `getopt_long (3)`
structure. It contains option description and group number which is used
for selecting a few sets of options from a single list.
Class `GetOptSet` is a container for options.

`parse_options()` can be used to parse a set of options until the
first non-option element.

`parse_options_all()` can be used in simple tools to collect all options
and non-option arguments from a command line.

### HelpPrinter -- class for formatting help messages

It can also write `pod` format which can be converted to `html` or `man`.

### ActionProg -- program interface built using a list of "actions"

Example:
```c++

// define an action
class Action1 : public Action{
public:
  Action1():
      Action("action1", "do something") {
    options.add("opt1", 1,'o',"MYOPTS", "some option1");
  }

  void help_impl(HelpPrinter & pr) override {
    pr.usage("[<options>] <par1> <par2>");
    pr.par("\nDo something with two parameters");
    pr.head(2, "Options");
    pr.opts({"MYOPTS"});
  }

  virtual void run_impl(const std::vector<std::string> & args,
                   const Opt & opts) override {
    if (args.size()!=2) throw Err() << get_name() << ": two parameters expected";
    do_somethins (args[0], args[1], opts);
  }
};


// run ActionProg() with list of options in main
int
main(int argc, char *argv[]){
  return ActionProg(argc, argv,
    "myprog", "program for doing something", {
    std::shared_ptr<Action>(new Action1),
  });
}

```