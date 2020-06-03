///\cond HIDDEN (do not show this in Doxyden)

#include "downloader.h"
#include "err/assert_err.h"

int
main(){
  try{

    Downloader D;
    D.add("http://slazav.xyz");
    D.add("http://slazav.xyz/1");
    D.add("http://slazav.xyz/2");
    D.add("http://slazav.xyz/3");
    D.add("http://slazav.mccme.ru/maps/podm/N53cE036.img");
    D.add("http://slazav.mccme.ru/maps/podm/N53cE037.img");
    D.add("http://slazav.mccme.ru/maps/podm/N54aE036.img");
    D.add("http://slazav.mccme.ru/maps/podm/N54aE037.img");
    D.add("http://slazav.mccme.ru/maps/podm/N54bE035.img");
    D.add("http://slazav.mccme.ru/maps/podm/N54bE036.img");
    D.add("http://slazav.mccme.ru/maps/podm/N54bE037.img");
    D.add("http://slazav.mccme.ru/maps/podm/N54bE038.img");

    D.get("http://slazav.mccme.ru/maps/podm/N54bE036.img");

  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
