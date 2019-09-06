///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "cairo/cairo_wrapper.h"
#include "mapsoft_data/mapsoft_data.h"
#include "getopt/getopt.h"
#include "filename/filename.h"
#include "geo_mkref/geo_mkref.h"
#include "geo_data/conv_geo.h"
#include "geo_data/geo_io.h"

#include "draw_trk.h"
#include "draw_pulk_grid.h"

using namespace std;

ext_option_list options = {
  {"grid", 0,0, MS2OPT_DRAWGRD, "draw grid"},
  {"map",  1,'m', MS2OPT_GEO_O, "write map file in OziExprorer format"},
};

void usage(bool pod=false, ostream & S = cout){
  string head = pod? "\n=head1 ":"\n";
  const char * prog = "draw_trk";
  S << prog << " -- draw_trk test program\n"
    << head << "Usage:\n"
    << prog << "  [<options>] <input files>\n"
    << "\n";
  S << head << "General options:\n";
  print_options(options, MS2OPT_STD, S, pod);
  S << head << "Options for making map reference:\n";
  print_options(options, MS2OPT_MKREF, S, pod);
  S << head << "Options for drawing tracks:\n";
  print_options(options, MS2OPT_DRAWTRK, S, pod);
  S << head << "Options for drawing map grid:\n";
  print_options(options, MS2OPT_DRAWGRD, S, pod);
  S << head << "Options for reading geodata:\n";
  print_options(options, MS2OPT_GEO_I | MS2OPT_GEO_IO, S, pod);
  S << head << "Options for writing map in OziExplorer format:\n";
  print_options(options, MS2OPT_GEO_O, S, pod);
  throw Err();
}

int
main(int argc, char **argv){
  try {
    ms2opt_add_std(options);
    ms2opt_add_out(options);
    ms2opt_add_geo_i(options);
    ms2opt_add_geo_io(options);
    ms2opt_add_ozimap_o(options);
    ms2opt_add_mkref(options);
    ms2opt_add_drawtrk(options);
    ms2opt_add_drawgrd(options);

    if (argc<2) usage();

    std::vector<std::string> files;
    Opt opts = parse_options_all(&argc, &argv, options, ~0, files);

    if (opts.exists("help")) usage();
    if (opts.exists("pod")) usage(true);

    // read geodata
    MapsoftData data;
    for (auto const &f:files) mapsoft_read(f, data, opts);

    // set some defaults:
    if (!opts.exists("mkref")) opts.put("mkref", "proj");
    if (!opts.exists("coords") && !opts.exists("coords_wgs")){
       dRect bbox;
       for (auto const & t:data.trks) bbox.expand(t.bbox());
       opts.put("coords_wgs", bbox);
      //opts.put("coords_wgs", data.bbox());
    }
    // get output file name
    std::string fname = opts.get("out", "");
    if (fname == "") throw Err() << "No output file specified (use -o option)";

    // create map reference
    GeoMap map = geo_mkref(opts);
    ConvMap cnv(map);


    // setup cairo context
    CairoWrapper cr;
    if      (file_ext_check(fname, ".pdf")) cr.set_surface_pdf(fname.c_str(), map.image_size.x,map.image_size.y);
    else if (file_ext_check(fname, ".ps"))  cr.set_surface_ps(fname.c_str(), map.image_size.x,map.image_size.y);
    else if (file_ext_check(fname, ".svg")) cr.set_surface_svg(fname.c_str(), map.image_size.x,map.image_size.y);
    else if (file_ext_check(fname, ".png")) cr.set_surface_img(Image(map.image_size.x,map.image_size.y,IMAGE_32ARGB));
    else throw Err() << "Unknown output format, use .pdf, .ps, .svg, .png extension";

    // draw tracks
    dPoint origin;
    for (auto const & t:data.trks)
      draw_trk(cr, origin, cnv, t, opts);

    // draw grid
    if (opts.get("grid", 0))
      draw_pulk_grid(cr, origin, cnv, opts);

    // if format is png write file
    if (file_ext_check(fname, ".png"))
      cr->save_png(fname.c_str());

    if (opts.exists("map")){
      map.image = fname;
      write_ozi_map(opts.get("map","").c_str(), map, opts);
    }


  }
  catch (Err e) {
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond

