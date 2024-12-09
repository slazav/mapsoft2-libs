#include <fstream>
#include <sstream>
#include <iostream>

#include "getopt/getopt.h"
#include "geo_tiles/geo_tiles.h"
#include "getopt/help_printer.h"
#include "image/io_jpeg.h"

#include "image_mbtiles/image_mbtiles.h"
#include "jnx.h"

GetOptSet options;

void usage(bool pod=false){
  HelpPrinter pr(pod, options, "mbtiles2jnx");
  pr.name("mapsoft2 mbtiles to jnx converter");
  pr.usage("<options> <input file> <output file>");
  pr.head(1, "Options");
  pr.opts({"HELP","POD", "JPEG"});

  throw Err();
}

int32_t deg2crd(const double v) {return (int32_t)rint(v / 180.0 * 0x7fffffff);}

int
main(int argc, char *argv[]){
  try{
    ms2opt_add_std(options, {"HELP","POD"});
    options.add("jpeg_quality", 1,0, "JPEG",
      "Set JPEG quality (default 95).");

    if (argc<2) usage();
    std::vector<std::string> files;
    Opt O = parse_options_all(&argc, &argv, options, {}, files);
    if (O.exists("help")) usage();
    if (O.exists("pod"))  usage(true);

    if (files.size()!=1) usage();
    std::string & ifile = files[0];

    /********************************************/
    // JNX structure:
    // [header]  -- header_size (13*4 = 52)
    // [level info] ...  -- level_info_size * nlevels
    // [map description, level descriptions] -- variable length
    // [tile info_z1] ...  -- tile_info_size * ntiles(z=z1)
    // [tile info_z2] ...  -- tile_info_size * ntiles(z=z2)
    // [tile data] ...

    const size_t header_size = 52;
    const size_t level_info_size = 17;
    const size_t tile_info_size = 28;

    // header
    std::ifstream s(ifile);
    jnx_header_t header;
    s.read((char*)&header, sizeof(header));

    std::cout << "JNX header:\n"
      << "  version: " << header.ver << "\n"
      << "  dev_id:  " << std::hex << (uint32_t)header.dev_id << std::dec <<"\n"
      << "  S:       " << header.S << " -> " << jnx2deg(header.S) << "\n"
      << "  N:       " << header.N << " -> " << jnx2deg(header.N) << "\n"
      << "  W:       " << header.W << " -> " << jnx2deg(header.W) << "\n"
      << "  E:       " << header.E << " -> " << jnx2deg(header.E) << "\n"
      << "  nl:      " << header.nl << "\n"
      << "  exp:     " << header.exp << "\n"
      << "  pr_id:   " << header.pr_id << "\n"
      << "  crc:     " << header.crc << "\n"
      << "  sver:    " << header.sver << "\n"
      << "  soff:    " << header.soff << "\n"
    ;

    jnx_level_info_t level_info[header.nl];
    s.read((char*)&level_info, sizeof(level_info));

    for (int i=0; i<header.nl; ++i){
      std::cout << "Level " << i << "\n"
         << "  ntiles: " << level_info[i].ntiles << "\n"
         << "  offset: " << level_info[i].offset << "\n"
         << "  scale:  " << level_info[i].scale  << "\n";
    }

    {
      int32_t v;
      s.read((char*)&v, 4);
      std::cout << "Map descr version: " << v << "\n";
    }

    std::cout << "Map descr:\n";
    for (int i=0; i<6; i++){
      std::string str;
      getline(s, str, (char)0);
      std::cout << "  " << i << ": " << str << "\n";
    }

    int32_t NL;
    s.read((char*)&NL, 4);
    std::cout << "NlevelDescr: " << NL << "\n";

    for (int i=0; i<NL; ++i){
      std::cout << "Level " << i << " descr:\n";

      std::string str;
      getline(s, str, (char)0);
      std::cout << "  str: " << str << "\n";
      getline(s, str, (char)0);
      std::cout << "  str: " << str << "\n";
      getline(s, str, (char)0);
      std::cout << "  str: " << str << "\n";

      int32_t v;
      s.read((char*)&v, 4);
      std::cout << "  int: " << v << " (z scale)\n";
    }

    {
      int32_t N;
      s.read((char*)&N, 4);
      std::cout << "NTileDescr: " << N << "\n";
      std::cout << "Tile descriptions:\n";
      for (int i=0; i<N; i++){
        std::string str;
        getline(s, str, (char)0);
        std::cout << "  " << i << ": " << str << "\n";
      }
    }

    for (int l = 0; l<header.nl; ++l){
      std::cout << "current pos: " << s.tellg()
                << "  offs1: " << level_info[l].offset << "\n";

      jnx_tile_info_t tinfo[level_info[l].ntiles];
      s.read((char *)&tinfo, sizeof(tinfo));
      size_t pos = s.tellg();

      for (int i=0; i<level_info[l].ntiles; i++){
        std::cout << "TILE:\n"
          << "  S: " << tinfo[i].S << " -> " << jnx2deg(tinfo[i].S) << "\n"
          << "  N: " << tinfo[i].N << " -> " << jnx2deg(tinfo[i].N) << "\n"
          << "  W: " << tinfo[i].W << " -> " << jnx2deg(tinfo[i].W) << "\n"
          << "  E: " << tinfo[i].E << " -> " << jnx2deg(tinfo[i].E) << "\n"
          << "  image: " << tinfo[i].image_w << " " << tinfo[i].image_h << "\n"
          << "  data size:  " << tinfo[i].data_size << "\n"
          << "  data offs:  " << tinfo[i].data_offs << "\n";
        s.seekg(tinfo[i].data_offs, s.beg);
        std::string data(tinfo[i].data_size, '\0');
        s.read((char *)data.data(), data.size());
        std::ostringstream ofname;
        ofname << l << "-" << i << ".jpg";
        std::ofstream out(ofname.str());
        out << (char)0xFF << (char)0xD8;
        out.write((char *)data.data(), data.size());
        out << (char)0xFF << (char)0xD9;
      }
      s.seekg(pos, s.beg);
    }
  }

  catch(Err & e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
