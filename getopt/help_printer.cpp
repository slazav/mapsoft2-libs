#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "getopt.h"
#include "help_printer.h"
#include "err/err.h"

#include <sys/ioctl.h> // For ioctl, TIOCGWINSZ
#include <unistd.h> // For STDOUT_FILENO

using namespace std;



HelpPrinter::HelpPrinter(
    bool pod, const GetOptSet & opts,
    const std::string & name):
      s(std::cout), name_(name),
      pod(pod), opts_(opts),
      usage_head(false), width(80) {

  struct winsize size;
  if (ioctl(STDOUT_FILENO,TIOCGWINSZ,&size) != -1) {
    width = size.ws_col;
  } else {
    width = 80;
  }
  if (width<1) width=80;
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
HelpPrinter::opts(const std::set<std::string> & groups){

  if (pod) s << "=over 2\n\n";

  for (auto const & opt:opts_){
    // select option groups
    if (groups.count(opt.group) == 0) continue;

    // Check if we printed this option before.
    if (printed.count(opt.name) != 0) throw Err() <<
      "HelpPrinter: duplicated option in the help message: " << opt.name;
    printed.insert(opt.name);

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
  if (pod){
    s << text << "\n\n";
  }
  else{
    format(0,0,text);
    s << "\n";
  }
}


HelpPrinter::~HelpPrinter(){
  // check if we have printed all options
  for (auto const & o:opts_){
    if (printed.count(o.name) == 0)
      s << "\nWARNING: options have not been printed: " << o.name
        << " (group: " << o.group << ")\n";;
  }
}

void
HelpPrinter::format(int ind0, int ind1, const std::string & text){
  int lsp=0;
  size_t ii=0;
  size_t text_width = width>ind1 ? width-ind1 : width;
  s << string(ind0, ' ');
  for (size_t i=0; i<text.size(); i++,ii++){
    if ((text[i]==' ') || (text[i]=='\n')) lsp=i+1;
    if ((ii>=text_width) || (text[i]=='\n')){
      if (lsp <= (ssize_t)i-(ssize_t)ii) lsp = i;
      if (ii!=i) s << string(ind1, ' ');
      s << text.substr(i-ii, lsp-i+ii-1) << endl;
      ii=i-lsp;
    }
  }
  if (ii!=text.size()) s << string(ind1, ' ');
  s << text.substr(text.size()-ii, ii) << "\n";
}
