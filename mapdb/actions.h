#ifndef MAPDB_ACTIONS
#define MAPDB_ACTIONS

#include <vector>
#include <string>
#include "mapdb/mapdb.h"
#include "getopt/help_printer.h"

// abstract class
class Action {
  protected:
    GetOptSet options;
  public:

    // constructor: add help option
    Action(){
      ms2opt_add_std(options, {"HELP"});
    }

    // action name
    virtual std::string get_name() const = 0;

    // action short description
    virtual std::string get_descr() const = 0;

    // print help message
    void help(bool pod=false){
      std::string fullname = std::string("ms2vmap ") + get_name();
      HelpPrinter pr(pod, options, fullname);
      pr.head(1, get_name() + " -- " + get_descr());
      // We do not want to show -h in the help message.
      // Now MS2OPT_STD should be empty. If not, HelpPrinter will
      // print an error.
      options.remove("help");
      help_impl(pr);
    }

    // implementation-specific part
    virtual void help_impl(HelpPrinter & pr) = 0;

    // parse options and run the action
    void run(int *argc, char **argv[]){
      std::vector<std::string> args;
      Opt opts = parse_options_all(argc, argv, options, {}, args);
      if (opts.exists("help")) {help(); throw Err();}
      run_impl(args, opts);
    };

    // implementation-specific part
    virtual void run_impl(
      const std::vector<std::string> & args,
      const Opt & opts) = 0;
};

/**********************************************************/
// list map pages
class ActionList : public Action{
public:
  ActionList(){
    const char *g = "MAPDB_LIST";
    options.add("long",  0, 'l',g, "Long listing format (name, file, bbox).");
  }
  std::string get_name() const override { return "list"; }
  std::string get_descr() const override { return "list map pages"; }
  void help_impl(HelpPrinter & pr) override {
    pr.usage("<map config>");
    pr.head(2, "Options");
    pr.opts({"MAPDB_LIST"});
  }

  virtual void run_impl(const std::vector<std::string> & args,
                   const Opt & opts) override {

    if (args.size()!=1) throw Err() << get_name()
      << ": argument expected: <map config>";
    MapDB map(args[0]);
    map.list_pages(opts.get<bool>("long", false));
  }
};

/**********************************************************/
// import map page
class ActionImport : public Action{
public:
  ActionImport(){ }

  std::string get_name() const override { return "import"; }
  std::string get_descr() const override { return "import map page from a file"; }
  void help_impl(HelpPrinter & pr) override { pr.usage("<map config> <page> <file>"); }

  virtual void run_impl(const std::vector<std::string> & args,
                   const Opt & opts) override {

    if (args.size()!=3) throw Err() << get_name()
      << ": arguments expected: <map config> <page> <file to import>";
    MapDB map(args[0]);
    map.import_page(args[1], args[2]);
  }
};

/**********************************************************/
// export map page
class ActionExport : public Action{
public:
  ActionExport(){ }

  std::string get_name() const override { return "export"; }
  std::string get_descr() const override { return "export map page to a file"; }
  void help_impl(HelpPrinter & pr) override { pr.usage("<map config> <page> <file>"); }

  virtual void run_impl(const std::vector<std::string> & args,
                   const Opt & opts) override {

    if (args.size()!=3) throw Err() << get_name()
      << ": arguments expected: <map config> <page> <file>";
    MapDB map(args[0]);
    map.export_page(args[1], args[2]);
  }
};

#endif
