///\cond HIDDEN (do not show this in Doxyden)
// Program for working with map database

#include <unistd.h>
#include <cassert>
#include <iostream>
#include <string>
#include <memory>

#include "err/err.h"
#include "getopt/getopt.h"
#include "getopt/help_printer.h"

#include "actions.h"

using namespace std;
GetOptSet options;

// all actions
std::vector<std::shared_ptr<Action> > actions = {
  std::shared_ptr<Action>(new ActionList),
  std::shared_ptr<Action>(new ActionImport),
  std::shared_ptr<Action>(new ActionExport),
};

/**********************************************************/
void usage(bool pod=false){
  HelpPrinter pr(pod, options, "ms2mapdb");

  pr.name("working with vector maps");
  pr.usage("(-h|--help|--pod)");
  pr.usage("<action> (-h|--help)");
  pr.usage("<action> [<action arguments and options>]");

  pr.head(1, "General options:");
  pr.opts({"HELP", "POD"});
  pr.head(1, "Actions:");

  // print list of actions
  for (auto const & a: actions){
    pr.usage(a->get_name() + " <arguments> -- " + a->get_descr());
  }

  // print halp message for each action:
  if (pod){
    for (auto const & a: actions) a->help(pod);
  }

  throw Err();
}
/**********************************************************/

int
main(int argc, char *argv[]){
  try{

    ms2opt_add_std(options, {"HELP", "POD"});

    // general options -- up to first non-option argument
    Opt O = parse_options(&argc, &argv, options, {}, NULL);
    if (O.exists("help")) usage();
    if (O.exists("pod"))  usage(true);
    if (argc<1) usage();

    // run the action
    for (auto const & a: actions){
      if (a->get_name() == argv[0]) {a->run(&argc, &argv); return 0;}
    }

    throw Err() << "ms2mapdb: unknown action: " << argv[0];

  }
  catch (Err & e) {
    if (e.str()!="") cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
