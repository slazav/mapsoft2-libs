#ifndef HELP_PRINTER_H
#define HELP_PRINTER_H

#include <getopt.h>
#include <vector>
#include <string>
#include <set>
#include "opt/opt.h"

///\addtogroup libmapsoft
///@{

///\defgroup HelpPrinter Help formatter for Mapsoft getopt wrapper.
///@{

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
