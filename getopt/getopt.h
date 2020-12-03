#ifndef GETOPT_H
#define GETOPT_H

#include <getopt.h>
#include <vector>
#include <string>
#include <set>
#include "opt/opt.h"

///\addtogroup libmapsoft
///@{

///\defgroup Getopt Mapsoft getopt wrapper.
///@{


/********************************************************************/

// Command-line option, extention of getopt_long structure
struct GetOptEl{
  std::string name;    ///< see man getopt_long
  int         has_arg; ///< see man getopt_long
  int         val;     ///< see man getopt_long
  std::string group;   ///< this setting is used to select group of options
  std::string desc;    ///< description, used in print_options()
};

// Container for GetOptEl
class GetOptSet: public std::vector<GetOptEl> {
public:

  // Add new option if it was not added yet:
  void add(const std::string & name,
           const int has_arg,
           const int val,
           const std::string & group,
           const std::string & desc){

    // Do nothing if option with same name or value was added in the
    // same group (same option lists may be included from a few places).
    // Throw error if options from different groups have same name or value.
    auto o = get(name);
    if (!o) o = get(val);
    if (o){
      if (o->group == group) return;
      throw Err() << "duplicated options: "
         << name << "(" << (val>0? (char)val:' ') << ")" << ": " << group << ", " << desc << " -- "
         << o->name << "(" << (o->val>0? (char)o->val:' ') << ")" << ": " << o->group << ", " << o->desc;
    }
    push_back({name, has_arg, val, group, desc});
  }

  // Add new or replace old option
  void replace(const std::string & name,
           const int has_arg,
           const int val,
           const std::string & group,
           const std::string & desc){
    auto o = get(name);
    if (o) *o = {name, has_arg, val, group, desc};
    else push_back({name, has_arg, val, group, desc});
  }

  // get option by name (or NULL)
  GetOptEl * get(const std::string & name) {
    for (auto & o:*this)
      if (o.name == name) return &o;
    return NULL;
  }

  // get option by value (or NULL)
  GetOptEl * get(const int val) {
    if (val == 0) return NULL;
    for (auto & o:*this)
      if (o.val == val) return &o;
    return NULL;
  }


  // check if the option exists
  bool exists(const std::string & name) const{
    for (auto const & o:*this)
      if (o.name == name) return true;
    return false;
  }

  // check if the option value exists (or zero)
  bool exists(const int val) const{
    if (val == 0) return false;
    for (auto const & o:*this)
      if (o.val == val) return true;
    return false;
  }


  // remove option if it exists
  void remove(const std::string & name){
    auto i=begin();
    while (i!=end()){
      if (i->name == name) i=erase(i);
      else ++i;
    }
  }

};

/********************************************************************/

// add STD group of options
void ms2opt_add_std(GetOptSet & opts);

// add OUT group of options
void ms2opt_add_out(GetOptSet & opts);

/********************************************************************/

/**
Main getopt wrapper. Parse cmdline options up to the first non-option
argument or to last_opt. Uses GetOptSet structure. Groups argument
is used to select option groups. If it is empty then all known options
are read. All options are returned as Opt object. */
Opt parse_options(int *argc, char ***argv,
                  const GetOptSet & ext_options,
                  const std::set<std::string> & groups,
                  const char * last_opt = NULL);


/** Parse mixed options and non-option arguments (for simple programs).
If groups set is empty, then read all known options. */

Opt
parse_options_all(int *argc, char ***argv,
              const GetOptSet & ext_options,
              const std::set<std::string> & groups,
              std::vector<std::string> & non_opts);

/********************************************************************/
// Class for formatting help messages and man pages
class HelpPrinter{
private:
  std::ostream & s;
  std::string name_;
  bool pod;
  const GetOptSet & opts_;
  bool usage_head; // has usage header been already printed?
  int width;
  std::set<std::string> printed; // options we already printed

public:
  HelpPrinter(bool pod, const GetOptSet & opts,
              const std::string & name);

  // set text width (for option formatting)
  void set_width(int w) {width = w;}

  // print name section
  void name(const std::string & descr);

  // print usage line (header is printed before the first one)
  void usage(const std::string & text);


  // print some groups of options
  void opts(const std::set<std::string> & groups);

  // print header
  void head(int level, const std::string & text);

  // print a paragraph of text
  void par(const std::string & text);

  // finish printing, check if all options have been printed
  ~HelpPrinter();

private:
  // format text
  void format(int i0, int i1, const std::string & text);
};

///@}
///@}
#endif
