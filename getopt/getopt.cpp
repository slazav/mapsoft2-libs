#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <getopt.h>
#include "getopt.h"
#include "err/err.h"


using namespace std;

/**********************************************/

void
ms2opt_add_std(GetOptSet & opts){
  const char *g = "STD";
  opts.add("help",    0,'h', g, "Show help message.");
  opts.add("pod",     0, 0 , g, "Show help message as POD template.");
  opts.add("verbose", 0,'v', g, "Be verbose.\n");
}

void
ms2opt_add_out(GetOptSet & opts){
  const char *g = "OUT";
  opts.add("out", 1, 'o', g, "Output file.");
}

/**********************************************/
/* Simple getopt_long wrapper.
Parse cmdline options up to the first non-option argument
or last_opt. For the long_options structure see getopt_long (3).
All options are returned as Opt object.
*/
Opt
parse_options(int * argc, char ***argv,
              struct option long_options[], const char * last_opt){
  Opt O;
  int c;
  opterr=0; // no error printing by getopt_long
  optind=0; // to check that '+' in optstring is supported

  // build optstring
  string optstring="+:"; // note "+" and ":" in optstring
  int i = 0;
  while (long_options[i].name){
    if (long_options[i].val != 0){ optstring+=long_options[i].val;
      if (long_options[i].has_arg==1)  optstring+=":";
      if (long_options[i].has_arg==2)  optstring+="::";
    }
    if (long_options[i].flag)
      throw Err() << "non-zero flag in option structure";
    i++;
  }

  while(1){
    int option_index = 0;

    c = getopt_long(*argc, *argv, optstring.c_str(), long_options, &option_index);
    if (c == -1) break;

    // Here we should care about multi-letter options
    // in case of -xx option optind will be different from -x or --xx.
    // For one-letter options optopt should be used instead of (*argv)[optind-1].
    if (c == '?' && optopt!=0) throw Err() << "unknown option: -" << (char)optopt;
    if (c == '?') throw Err() << "unknown option: " << (*argv)[optind-1];
    if (c == ':') throw Err() << "missing argument: " << (*argv)[optind-1];

    if (c!=0){ // short option -- we must manually set option_index
      int i = 0;
      while (long_options[i].name){
        if (long_options[i].val == c) option_index = i;
        i++;
      }
    }
    // This usually can not happen, but let's check:
    if (!long_options[option_index].name)
      throw Err() << "unknown option: " << (*argv)[optind-1];

    std::string key = long_options[option_index].name;
    std::string val = long_options[option_index].has_arg? optarg:"1";
    O.put<string>(key, val);

    if (last_opt && O.exists(last_opt)) break;

  }
  *argc-=optind;
  *argv+=optind;
  optind=0;

  return O;
}


/**********************************************/
Opt
parse_options(int *argc, char ***argv,
              const GetOptSet & ext_options,
              const std::set<std::string> & groups,
              const char * last_opt) {

  // build long_options structure
  option * long_options = new option[ext_options.size()+1];
  int j = 0;
  for (auto const & opt:ext_options) {
    if (groups.size()>0 && groups.count(opt.group) == 0) continue;
    long_options[j].name    = opt.name.c_str();
    long_options[j].has_arg = opt.has_arg;
    long_options[j].flag    = NULL;
    long_options[j].val     = opt.val;
    j++;
  }
  long_options[j].name    = NULL;
  long_options[j].has_arg = 0;
  long_options[j].flag    = NULL;
  long_options[j].val     = 0;

  Opt O;
  try { O = parse_options(argc, argv, long_options, last_opt); }
  catch (Err & e) {
    delete[] long_options;
    throw;
  }
  delete[] long_options;
  return O;
}


Opt
parse_options_all(int *argc, char ***argv,
              const GetOptSet & ext_options,
              const std::set<std::string> & groups,
              vector<string> & non_opts){

  Opt O = parse_options(argc, argv, ext_options, groups);
  while (*argc>0) {
    non_opts.push_back(*argv[0]);
    Opt O1 = parse_options(argc, argv, ext_options, groups);
    O.insert(O1.begin(), O1.end());
  }
  return O;
}
