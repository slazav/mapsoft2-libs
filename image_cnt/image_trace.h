#ifndef IMAGE_TRACE_H
#define IMAGE_TRACE_H

#include <vector>
#include "geom/line.h"
#include "geom/multiline.h"
#include "image/image_r.h"


/* Functions for river/mountain tracing */

/********************************************************************/
// Trace one river.
// Parameters:
//   p0 -- starting point
//   nmax -- no-sink area when we stop calculation
//   hmin -- threshold height where we want to stop calculation
//   down -- is the flow goes down (river) of up (mountain)
iLine
trace_river(const ImageR & img, const iPoint & p0, int nmax, int hmin, bool down);

// trace rectangular map, return sink directions
ImageR trace_map_dirs(const ImageR & img, int nmax, bool down);

// Use sink directions to calculate sink areas.
//   dirs -- sink directions obtained by trace_map_dirs() or empty image
ImageR trace_map_areas(const ImageR & dirs);

/********************************************************************/
// Trace rivers or mountain ridges on the image.
//   dem -- altitude map
//   nmax -- No-sink area when we stop calculation (points), 0 for all map points
//   down -- Is the flow goes down (river) of up (mountain)
//   mina -- Threshold of the sink area.
//   start_detect -- How to detect start of a river/ridge:
//      TRACE_START_NONE -- show everythins with area > mina. Not reasonable on flat places.
//      TRACE_START_SIDEH2 -- start if hight on both sides is smaller/higher by start_par [m]
//                            ("sides" are 2 data points away)
//      TRACE_START_MINDH -- start if average sink area is smaller/higher by start_par [m]
//   start_par -- parameter for detecting start of a river/ridge
//   smooth_passes -- smooth result (0 - no smoothing, 1,2,3... - stronger smoothing)

enum start_detect_t {
  TRACE_START_MINDH,
  TRACE_START_SIDEH2,
  TRACE_START_PERP3,
  TRACE_START_PERP4,
  TRACE_START_PL3,
  TRACE_START_PL4,
  TRACE_START_NONE
};

dMultiLine
trace_map(const ImageR & dem, const int nmax, const bool down, const double mina,
          const start_detect_t start_detect = TRACE_START_NONE, const double start_par = 1.0,
          const size_t smooth_passes = 2);

dMultiLine
trace_map2(const ImageR & dem, const int nmax, const bool down, const double mina);
#endif
