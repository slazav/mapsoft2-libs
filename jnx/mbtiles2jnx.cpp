#include <fstream>
#include <sstream>
#include <iostream>

#include "getopt/getopt.h"
#include "geom/poly_tools.h"
#include "geo_tiles/geo_tiles.h"
#include "getopt/help_printer.h"
#include "image/io_jpeg.h"

#include "image_tiles/image_t_mbtiles.h"
#include "jnx.h"

#define MINZ_LIMIT 8
#define MAXZ_LIMIT 13

GetOptSet options;

void usage(bool pod=false){
  HelpPrinter pr(pod, options, "mbtiles2jnx");
  pr.name("mapsoft2 mbtiles to jnx converter");
  pr.usage("<options> <input file> <output file>");
  pr.head(1, "Options");
  pr.opts({"HELP","POD", "JPEG"});

  throw Err();
}

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

    if (files.size()!=2) usage();
    std::string & ifile = files[0];
    std::string & ofile = files[1];

    ImageMBTiles mbtiles(ifile, 1);
    int minz = mbtiles.min_zoom();
    int maxz = mbtiles.max_zoom();
    if (minz<0 || maxz<0) throw Err() << "empty mbtiles file";

    minz = std::max(minz,MINZ_LIMIT);
    maxz = std::min(maxz,MAXZ_LIMIT);
    if (minz>maxz) throw Err() << "no levels available";

    dMultiLine brd = figure_line<double>("[66,36,12,5]");

    std::vector<int32_t> zlevels;
    dRect r;
    for (int z = minz; z<=maxz; ++z){
      r.expand(mbtiles.wgs_bounds(z));
std::cerr << z << " " << mbtiles.wgs_bounds(z) << "\n";
      if (mbtiles.tile_number(z)>0) zlevels.push_back(z);
    }
    int32_t nl = zlevels.size();

std::cerr << "r: " << r.tlc() << " - " << r.brc()  << "\n";

    /********************************************/
    // JNX structure:
    // [header]
    // [level info] ...  -- level_info_size * nlevels
    // [map description, level descriptions] -- variable length
    // [tile info_z1] ...  -- tile_info_size * ntiles(z=z1)
    // [tile info_z2] ...  -- tile_info_size * ntiles(z=z2)
    // [tile data] ...

    // header
    std::ofstream s(ofile);
    jnx_header_t header;
    header.dev_id = 0xe29b66c4;
    header.W = deg2jnx(r.x);
    header.S = deg2jnx(r.y);
    header.N = deg2jnx(r.y+r.h);
    header.E = deg2jnx(r.x+r.w);
    header.nl = nl;
    s.write((char*)&header, sizeof(header));

    // map description
    s.seekp(JNX_HEADER_SIZE + JNX_LEVEL_INFO_SIZE * nl, s.beg);
    int32_t mdver=3;  // section version
    s.write((char *)&mdver, 4);  // section version
    s << "12345678-1234-1234-1234-123456789ABC" << '\0'; // GUID
    s << mbtiles.get_metadata("name") << '\0'; // Product name
    s << mbtiles.get_metadata("description") << '\0'; //
    s << '\0';
    s << '\0';
    s << '\0';
    s.write((char *)&nl, 4);
    for (const auto z:zlevels){
      s << mbtiles.get_metadata("name") << '\0';
      s << mbtiles.get_metadata("name") << '\0';
      s << mbtiles.get_metadata("name") << '\0';
      s.write((char *)&z, 4); // fixme
    }
    int32_t nn=0; // number of some strange records
    s.write((char *)&nn, 4);

    uint32_t tile_info_offs = s.tellp();
    uint32_t offs = tile_info_offs;

    s.seekp(JNX_HEADER_SIZE, s.beg);
    for (const auto z:zlevels){
      jnx_level_info_t info;
      info.ntiles = mbtiles.tile_number(z);
      info.offset = offs;
//      info.scale = rint(34115555.0/pow(2,z) * cos(r.cnt().y/180.0*M_PI)/1.1);
      info.scale = rint(156542976 / pow(2,z)); // same as Demo.JNX
      s.write((char *)&info, sizeof(info));
      offs += JNX_TILE_INFO_SIZE * info.ntiles;
    }
    uint32_t tile_data_offs = offs;

    // writing tile descriptors at tile_info_offs
    // and tile data at tile_data_offs

    // tile descriptors
    s.seekp(0, s.end);
    size_t tile_offs = tile_info_offs;
    GeoTiles tcalc;
    for (const auto z:zlevels){
      for (const auto tkey: mbtiles.tile_list(z)){
        dRect r = tcalc.tile_to_range(tkey, z);
        std::ostringstream s1;
        image_save_jpeg(mbtiles.tile_read(tkey), s1, O);
        auto data = s1.str().substr(2, s1.str().size()-4);

        jnx_tile_info_t info;
        info.N = deg2jnx(r.y+r.h);
        info.W = deg2jnx(r.x);
        info.S = deg2jnx(r.y);
        info.E = deg2jnx(r.x+r.w);
        info.image_w = info.image_h = 256;
        info.data_size = data.size();
        info.data_offs = tile_data_offs;

        s.seekp(tile_info_offs, s.beg);
        s.write((char *)&info, sizeof(info));
        tile_info_offs = s.tellp();

        s.seekp(tile_data_offs, s.beg);
        s.write((char *)data.data(), data.size());
        tile_data_offs=s.tellp();
      }
    }

  }
  catch(Err & e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
