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
  int m = MS2OPT_STD;
  opts.add("help",    0,'h', m, "Show help message.");
  opts.add("pod",     0, 0 , m, "Show help message as POD template.");
  opts.add("verbose", 0,'v', m, "Be verbose.\n");
}

void
ms2opt_add_out(GetOptSet & opts){
  opts.add("out", 1, 'o', MS2OPT_OUT, "Output file.");
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
              int mask,
              const char * last_opt) {

  // build long_options structure
  option * long_options = new option[ext_options.size()+1];
  int j = 0;
  for (auto const & opt:ext_options) {
    if ((opt.group & mask) == 0) continue;
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
  catch (Err e) {
    delete[] long_options;
    throw e;
  }
  delete[] long_options;
  return O;
}


Opt
parse_options_all(int *argc, char ***argv,
              const GetOptSet & ext_options,
              int mask, vector<string> & non_opts){

  Opt O = parse_options(argc, argv, ext_options, mask);
  while (*argc>0) {
    non_opts.push_back(*argv[0]);
    Opt O1 = parse_options(argc, argv, ext_options, mask);
    O.insert(O1.begin(), O1.end());
  }
  return O;
}

/**********************************************/

#include <sys/ioctl.h> // For ioctl, TIOCGWINSZ
#include <unistd.h> // For STDOUT_FILENO

HelpPrinter::HelpPrinter(
    bool pod, const GetOptSet & opts,
    const std::string & name):
    s(std::cout),
    pod(pod), opts_(opts), name_(name),
    printed(0), usage_head(false), width(80) {

  struct winsize size;
  ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
  width = size.ws_col;
}

void
HelpPrinter::name(const std::string & descr){
  if (pod)
    s << "=head1 NAME\n\n"
      << name_ << " -- " << descr << "\n\n";
  else
    s << name_ << " -- " << descr << "\n";
}

void
HelpPrinter::usage(const std::string & text){
  if (pod){
    if (!usage_head) s << "=head1 SYNOPSIS\n\n";
    s << "\t" << name_ << " " << text << "\n\n";
  }
  else {
    if (!usage_head) s << "Usage:\n";
    s << "\t" << name_ << " " << text << "\n";
  }
  usage_head = true;
}

void
HelpPrinter::opts(unsigned int mask){

  if (pod) s << "=over 2\n\n";

  for (auto const & opt:opts_){
    if ((opt.group & mask) == 0) continue;

    ostringstream oname;

    if (opt.val)
      oname << " -" << (const char)opt.val << ",";
    oname << " --" << opt.name;

    if (opt.has_arg == 1) oname << " <arg>";
    if (opt.has_arg == 2) oname << " [<arg>]";

    if (!pod){
//      const int option_width = 25;
//      s << setw(option_width) << oname.str() << " -- ";
//      format(0, option_width+4, opt.desc);
      s << oname.str() << "\n";
      format(8, 8, opt.desc);
      s << "\n";
    }
    else {
      s << "=item B<< " << oname.str() << " >>\n\n"
        << opt.desc << "\n\n";
    }
  }
  if (pod) s << "=back\n\n";

  // Check if we printed these options before, then add them to
  // printed var.
  if (printed & mask) throw Err() <<
    "HelpPrinter: duplicated options in the help message";
  printed|=mask;
}

void
HelpPrinter::head(int level, const std::string & text){

  if (pod){
    std::string t(text);
    if (level==1) std::transform(t.begin(),t.end(),t.begin(), ::toupper);
    s << "=head" << level << " " << t << "\n\n";
  }
  else {
    s << "\n" << text << "\n\n";
  }
}

void
HelpPrinter::par(const std::string & text){
  if (pod)
    s << text << "\n\n";
  else
    format(0,0,text);
}


HelpPrinter::~HelpPrinter(){
  // check if we have printed all options
  unsigned int all=0;
  for (auto const & o:opts_) all |= o.group;
  if (all != printed)
    s << "\nWARNING: not all options have been printed: "
      << std::hex << (all^printed) << "\n";
}

void
HelpPrinter::format(int ind0, int ind1, const std::string & text){
  int lsp=0;
  int ii=0;
  const int text_width = width-ind1;
  s << string(ind0, ' ');
  for (int i=0; i<text.size(); i++,ii++){
    if ((text[i]==' ') || (text[i]=='\n')) lsp=i+1;
    if ((ii>=text_width) || (text[i]=='\n')){
      if (lsp <= i-ii) lsp = i;
      if (ii!=i) s << string(ind1, ' ');
      s << text.substr(i-ii, lsp-i+ii-1) << endl;
      ii=i-lsp;
    }
  }
  if (ii!=text.size()) s << string(ind1, ' ');
  s << text.substr(text.size()-ii, ii) << "\n";
}
