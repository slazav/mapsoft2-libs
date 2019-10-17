#ifndef GOBJ_MAPDB_H
#define GOBJ_MAPDB_H

#include <list>
#include <vector>
#include <string>
#include <map>

#include "cairo/cairo_wrapper.h"
#include "viewer/gobj.h"

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
class GObjMapDB : public GObj{
private:

  CairoWrapper cr;
  MapDB & map;

  enum StepFeature {
    FEATURE_STROKE, // draw the contour with some thickness and color
    FEATURE_DASH,   // set dashed line
    FEATURE_FILL,   // fill the area with some color
  };

  enum StepAction {
    STEP_UNKNOWN,
    STEP_DRAW_POINT,
    STEP_DRAW_LINE,
    STEP_DRAW_AREA
  };

  struct DrawingStep {
    StepAction action; // what to do
    int type;          // object type
    std::map<StepFeature, std::string> features;
    DrawingStep(): action(STEP_UNKNOWN), type(0) {}
  };

  std::list<DrawingStep> steps;

  static void add_feature(DrawingStep & st,
                          const std::vector<std::string> & vs, int ln);

public:

  /***/
  GObjMapDB(MapDB & map);

  int draw(const CairoWrapper & cr, const dRect &box) override;

};

#endif
