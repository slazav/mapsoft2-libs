///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <string>
#include <vector>
#include "getopt.h"
#include "help_printer.h"
#include <cstring>
#include "err/assert_err.h"

using namespace std;

// non-standard options
GetOptSet options;


void
usage(bool pod=false){
  HelpPrinter pr(pod, options, "getopt.test");

  pr.name("example of mapsoft-style getopt");
  pr.head(1, "Usage:");
  pr.usage("[<general_options>|<global_input_options>]\\\n"
           "         <input_file_1> [<input_options_1>]     \\\n"
           "         <input_file_2> [<input_options_2>] ... \\\n"
           "         (--out|-o) <output_file> [<output_options>]");

  cerr << "Each input file should be read and filtered according\n"
     << "with global and file-specific input options.\n"
     << "Then all data should be processed and written to the\n"
     << "output file according with output options.\n"
  ;
  pr.head(1, "General options:");
  pr.opts({"STD", "OUT"});
  pr.head(1, "Input options:");
  pr.opts({"MY_INP"});
  pr.head(1, "Common options (can be used as input and output options):");
  pr.opts({"MY_CMN"});
  pr.head(1, "Output options:");
  pr.opts({"MY_OUT"});
  throw Err();
}


int
main(int argc, char *argv[]){
  try{

    options.add("inp1",  1,'I', "MY_INP", "input option with argument");
    options.add("inp2",  0,'J', "MY_INP", "input option w/o argument");
    options.add("cmn1",  1,'C', "MY_CMN", "common option with argument");
    options.add("cmn2",  0,'D', "MY_CMN", "common option w/o argument");
    options.add("out1",  1,'O', "MY_OUT", "output option with argument");
    options.add("out2",  0,'P', "MY_OUT", "output option w/o argument");

    // adding duplicated options:
    assert_err(
      options.add("out2",  1,'Q', "MY_OUT1", "output option w/o argument"),
      "duplicated options: out2(Q): MY_OUT1, output option w/o argument -- "
                          "out2(P): MY_OUT, output option w/o argument"
    );
    assert_err(
      options.add("out2-1", 1,'P', "MY_OUT1", "output option w/o argument"),
      "duplicated options: out2-1(P): MY_OUT1, output option w/o argument -- "
                          "out2(P): MY_OUT, output option w/o argument"
    );

    // do nothing, duplicated option in the same group
    options.add("out2",  0,'P', "MY_OUT", "this description will not be used");

    // standard options: "STD", "OUT"
    ms2opt_add_std(options);
    ms2opt_add_out(options);

    if (argc<2) usage();

    Opt O = parse_options(&argc, &argv, options,
      {"STD", "OUT", "MY_INP", "MY_CMN"}, "out");
    if (O.exists("help")) usage();
    if (O.exists("pod"))  usage(true);
    bool verb = O.exists("verbose");

    Opt GO(O); // global options
    string ofile = O.get("out", "");

    // read input files
    while (ofile == "") {
      if (argc<1) break;
      const char * ifile = argv[0];

      // parse file-specific options and append global options
      O = parse_options(&argc, &argv, options,
        {"OUT", "MY_INP", "MY_CMN", "MY_OUT"}, "out");
      O.insert(GO.begin(), GO.end());

      ofile = O.get("out", "");
      O.erase("out");

      if (verb) cout << "reading: " << ifile
                     << " options: " << O << "\n";
    }
    // write output file if needed
    if (ofile != ""){
      // parse output options
      argc++; argv--;
      O = parse_options(&argc, &argv, options, {"MY_OUT", "MY_CMN"});

      if (verb) cout << "writing: " << ofile
                     << " options: " << O << "\n";
    }
  }

  catch(Err & e){
    if (e.str()!="") cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond