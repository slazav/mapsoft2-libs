#include <iostream>
#include "action_prog.h"


// Action interface:
Action::Action(const std::string & name, const std::string & descr):
    name(name), descr(descr){
  ms2opt_add_std(options, {"HELP"});
}

void
Action::help(bool pod){
  std::string fullname = std::string("ms2vmap ") + name;
  HelpPrinter pr(pod, options, fullname);
  pr.head(1, name + " -- " + descr);
  // We do not want to show -h in the help message:
  options.remove("help");
  help_impl(pr);
}

// parse options and run the action
void
Action::run(int *argc, char **argv[]){
  std::vector<std::string> args;
  Opt opts = parse_options_all(argc, argv, options, {}, args);
  if (opts.exists("help")) {help(); throw Err();}
  run_impl(args, opts);
};


// This is the top-level help message
void
usage(const ActionList & actions, const GetOptSet & options,
      const std::string & name, const std::string & descr, bool pod){
  HelpPrinter pr(pod, options, name);

  pr.name(descr);
  pr.usage("(-h|--help|--pod)");
  pr.usage("<action> (-h|--help)");
  pr.usage("<action> [<action arguments and options>]");

  pr.head(1, "General options:");
  pr.opts({"HELP", "POD"});
  pr.head(1, "Actions:");

  // print list of actions
  for (auto const & a: actions)
    pr.usage(a->get_name() + " -- " + a->get_descr());

  // Print help message for each action (pod only).
  if (pod) for (auto const & a: actions) a->help(pod);

  throw Err();
}

// This should be the only thing called in main()
int
ActionProg(int argc, char *argv[],
           const std::string & name, const std::string & descr,
           const ActionList & actions){
  try{

    GetOptSet options;
    ms2opt_add_std(options, {"HELP", "POD"});

    // general options -- up to first non-option argument
    Opt O = parse_options(&argc, &argv, options, {}, NULL);
    if (O.exists("help")) usage(actions, options, name, descr, false);
    if (O.exists("pod"))  usage(actions, options, name, descr,  true);
    if (argc<1) usage(actions, options, name, descr, false);

    // run the action
    for (auto const & a: actions)
      if (a->get_name() == argv[0]) {a->run(&argc, &argv); return 0;}

    throw Err() << name << ": unknown action: " << argv[0];

  }
  catch (Err & e) {
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

