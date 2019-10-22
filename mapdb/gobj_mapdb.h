#ifndef GOBJ_MAPDB_H
#define GOBJ_MAPDB_H

#include <list>
#include <vector>
#include <string>
#include <map>

#include "cairo/cairo_wrapper.h"
#include "viewer/gobj_multi.h"

#include "mapdb.h"
#include "image/image.h"
#include "geo_data/geo_data.h"
#include "opt/opt.h"

/*

Drawing of a vector map is a sequence of "drawing steps". Each step
contains drawing of a single object type, of a grid, or setting
some global parameter (like map reference).
The configuration file contains description of each step.

Drawing of a point/line/area object can be described
in following way:

    (point|line|area) <type> <feature> <options> ...
    + <feature> <options> ...
    + <feature> <options> ...
    ...

where possible features are:

    stroke <width> <color> -- Draw a point, a line or an area contour.
    dash <???> -- Setup dashed line. Valid for lines and areas together with stroke feature.
    fill <color> -- Fill an area.

*/

/********************************************************************/
#include "getopt/getopt.h"

// add MS2OPT_DRAWMAPDB options
void ms2opt_add_drawmapdb(GetOptSet & opts);

/********************************************************************/

/* Render MapDB map */
class GObjMapDB : public GObjMulti{
private:

  std::shared_ptr<MapDB> map;

  CairoWrapper cr;

  enum StepFeature {
    FEATURE_STROKE, // draw the contour with some thickness and color
    FEATURE_FILL,    // fill the area with some color
    FEATURE_SMOOTH,  // set line smoothing
    FEATURE_DASH,    // set dashed line
  };

  enum StepAction {
    STEP_UNKNOWN,
    STEP_DRAW_POINT,
    STEP_DRAW_LINE,
    STEP_DRAW_AREA
  };

  struct DrawingStep : public GObj {
    StepAction action; // what to do
    uint32_t etype;     // object extended type (type + (cl<<16) )
    std::map<StepFeature, std::string> features;
    MapDB * map;
    DrawingStep(MapDB * map): map(map), action(STEP_UNKNOWN), etype(0) {}

    // Put feature parameters to the map of features.
    // We want to parse strings only while reading the config file.
    // Then feature parameters are saved as raw data wrapped in std::string
    // and then restored in draw()
    template<typename T1>
    void put_feature(StepFeature ftr, const std::string & v1){
      struct fdata_t {T1 v1;} fdata = {
         str_to_type<T1>(v1),
      };
      features.emplace(ftr,
        std::string((const char*)&fdata, sizeof(fdata)));
    }
    template<typename T1, typename T2>
    void put_feature(StepFeature ftr,
          const std::string & v1, const std::string & v2){
      struct fdata_t {T1 v1; T2 v2;} fdata = {
         str_to_type<T1>(v1),
         str_to_type<T2>(v2),
      };
      features.emplace(ftr,
        std::string((const char*)&fdata, sizeof(fdata)));
    }

    int draw(const CairoWrapper & cr, const dRect & draw_range) override;
  };


  static void add_feature(DrawingStep & st,
                          const std::vector<std::string> & vs, int ln);

public:

  // constructor -- open new map
  GObjMapDB(const std::string & mapdir);

};

#endif
