///\cond HIDDEN (do not show this in Doxyden)

//#include <string>
//#include <vector>

#include "getopt/getopt.h"
#include "getopt/help_printer.h"
#include "filename/filename.h"
#include "shp.h"
#include "dbf.h"

using namespace std;

GetOptSet options;

void usage(bool pod=false){
  HelpPrinter pr(pod, options, "shp2vmap");
  pr.name("Read shape files from maanmittauslaitos.fi");
  pr.usage("<options> <shape basename>");

  pr.head(2, "Options:");
  pr.opts({"HELP","POD","A"});

  throw Err();
}


int
main(int argc, char *argv[]){
  try{
    ms2opt_add_std(options, {"HELP","POD"});

    options.add("crd", 0,'c', "A", "print coordinates");

    vector<string> files;
    Opt O = parse_options_all(&argc, &argv, options, {}, files);
    if (O.exists("help")) usage();
    if (O.exists("pod"))  usage(true);

    if (files.size() != 1) usage();

    // SHP file

    std::string shp = files[0] + ".shp";
    if (!file_exists(shp)) {
      std::cout << "SHP: no file\n";
      exit(0);
    }

    Shp SH(shp.c_str(), 0);
    std::cout << "SHP: "
      << "type=" << SH.get_type() << " "
      << "num=" <<SH.get_num() << "\n";

    // DBF file

    std::string dbf = files[0] + ".dbf";
    if (!file_exists(dbf)) {
      std::cout << "DBF: no file\n";
      exit(0);
    }

    Dbf DB(dbf.c_str(), 0);

    int nr = DB.nrecords();
    int nf = DB.nfields();

    std::cout << "DBF: "
      << "nfld=" << nf << " "
      << "nrec=" << nr << "\n";

    if (SH.get_num() != nr) throw Err()
      << "different number of objects in .shp and .dbf";

    for (size_t i = 0; i<nr; ++i){
      std::cout << "id: " << i << "\n";
      auto l = SH.get(i);
      if (O.exists("crd"))
        std::cout << "crd: " << l << "\n";
      else
        std::cout << "crd: " << l.npts()
           << " points in " << l.size() << " segments\n";
      for (size_t j = 0; j<nf; ++j){
        std::cout << "  " << DB.field_type(j)
                  << " " << DB.field_width(j) 
                  << ":" << DB.field_decimals(j) 
                  << " " << DB.field_name(j) 
                  << ":  " << DB.get_str(i,j) << "\n";
      }
    }

  }

  catch(Err & e){
    if (e.str()!="") cerr << "Error: " << e.str() << "\n";
    return 1;
  }

  catch(const char * e){
    cerr << "Error: " << e << "\n";
    return 1;
  }
}

///\endcond
