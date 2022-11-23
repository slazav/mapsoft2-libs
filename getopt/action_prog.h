#ifndef ACTION_PROG_H
#define ACTION_PROG_H

#include <vector>
#include <list>
#include <string>
#include <memory>

#include "getopt.h"
#include "help_printer.h"

///\addtogroup libmapsoft
///@{

///\defgroup ActionProg - Standard program with actions.
///@{

// The program interface is built using a list of "actions"
// derived from Action class.

// Action interface:
class Action {
  protected:
    GetOptSet options;
    std::string name, descr;
  public:

    // constructor: add standard options
    Action(const std::string & name, const std::string & descr);

    // print help message
    void help(bool pod=false);

    // implementation-specific part
    virtual void help_impl(HelpPrinter & pr) = 0;

    // parse options and run the action
    void run(int *argc, char **argv[]);

    // implementation-specific part
    virtual void run_impl(
      const std::vector<std::string> & args,
      const Opt & opts) = 0;

    // set name and description
    const std::string & get_name() const {return name;}
    const std::string & get_descr() const {return descr;}
};

// List of options
typedef std::list<std::shared_ptr<Action> >  ActionList;

// This should be the only thing called in main()
int ActionProg(int argc, char *argv[],
               const std::string & name, const std::string & descr,
               const ActionList & actions);

///@}
///@}
#endif
