///\cond HIDDEN (do not show this in Doxyden)

#include "downloader.h"
#include "err/assert_err.h"
#include <stdio.h>
//#include <unistd.h>
int
main(){
  try{

    {
      // create and destroy
      Downloader D;
    }

//    char cwd[1024];
//    getcwd(cwd, sizeof(cwd));
    std::string pref("file://");
    pref += getenv("PWD");

    Downloader D;

    // add files for parallel downloading
    D.add(pref + "/missing_file");
    D.add(pref + "/downloader.h");
    D.add(pref + "/downloader.test.cpp");
    D.add(pref + "/downloader.cpp");

    // start downloading if needed and get data
    auto s = D.get(pref + "/downloader.h");
    assert_eq(s.substr(0,20), "#ifndef DOWNLOADER_H")

    assert_eq(D.get_status(pref + "/downloader.h"), 2); // finished
    assert_eq(D.get_status(pref + "/downloader.h1"), -1); // not known
    assert_eq(D.get_status(pref + "/downloader.cpp") >= 0, true); // in queue, or finished
    assert_eq(D.wait(pref + "/missing_file"), 3); // wait, check that there is error
    assert_eq(D.wait(pref + "/downloader.cpp"), 2); // wait, check that there is ok


    D.del(pref + "/downloader.h");
    assert_eq(D.get_status(pref + "/downloader.h"), -1); // not known

    // get it again, without adding in advance
    s = D.get(pref + "/downloader.h");
    assert_eq(s.substr(0,20), "#ifndef DOWNLOADER_H")

    assert_err(D.get(pref+"/missing_file"),
      "Couldn't read a file:// file: " + pref + "/missing_file");

    D.clear();
    assert_eq(D.get_status(pref + "/downloader.h"), -1);
    assert_eq(D.wait(pref + "/missing_file"), -1);

//    D.add("http://slazav.mccme.ru/maps/podm/N53cE036.img");
//    D.add("http://slazav.mccme.ru/maps/podm/N53cE037.img");
//    D.add("http://slazav.mccme.ru/maps/podm/N54aE036.img");
//    D.add("http://slazav.mccme.ru/maps/podm/N54aE037.img");
//    D.add("http://slazav.mccme.ru/maps/podm/N54bE035.img");
//    D.add("http://slazav.mccme.ru/maps/podm/N54bE036.img");
//    D.add("http://slazav.mccme.ru/maps/podm/N54bE037.img");
//    D.add("http://slazav.mccme.ru/maps/podm/N54bE038.img");
//    D.get("http://slazav.mccme.ru/maps/podm/N54bE036.img");


  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
