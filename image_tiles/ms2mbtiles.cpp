///\cond HIDDEN (do not show this in Doxyden)

#include "getopt/getopt.h"
#include "getopt/help_printer.h"
#include "getopt/action_prog.h"
#include "image/io.h" // ms2opt_add_image

#include "image_t_mbtiles.h"

/*****************************************************************/
class ActionRescale : public Action{
public:
  // Rescale tiled image on levels zmax .. zmin using data from zmax+1
  ActionRescale():
    Action("rescale", "Fill leyers zmax .. zmin using data from zmax+1") {
    ms2opt_add_std(options, {"VERB"});
    ms2opt_add_image(options);
  }

  void help_impl(HelpPrinter & pr) override {
    pr.usage("[<options>] <file> <zmin> <zmax>");
    pr.head(2, "Options:");
    pr.opts({"VERB", "IMG"});
  }

  virtual void run_impl(const std::vector<std::string> & args,
                   const Opt & opts) override {

    if (args.size()!=3) throw Err() << "three arguments expected: <file> <zmin> <zmax>";
    std::string file = args[0];
    auto zmin = str_to_type<int>(args[1]);
    auto zmax = str_to_type<int>(args[2]);
    bool v = opts.get("verbose", false);

    ImageMBTiles timg(file, 0);
    timg.set_opt(opts);

    // Get tile bounds for zsrc
    for (int z = zmax; z>=zmin; z--){
      if (v) std::cout << "delete layer " << z << "\n";
      timg.layer_del(z);
      iRect trng = ceil((dRect)timg.tile_bounds(z+1)/2.0);
      if (v) std::cout << "rescale layer" << z << ": " << trng << "\n";
      for (int x = trng.x; x<=trng.x+trng.w; x++){
        for (int y = trng.y; y<=trng.y+trng.h; y++){
          //if (v) std::cout << "  rescale" << iPoint(x,y,z) << "\n";
          timg.tile_rescale(iPoint(x,y,z));
        }
      }
    }
  }
};

int
main(int argc, char **argv){
  return ActionProg(argc, argv,
    "ms2mbtiles", "tools for mbtiles", {
    std::shared_ptr<Action>(new ActionRescale),
  });
}


///\endcond