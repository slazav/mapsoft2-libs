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

iMultiLine
trace_map(const ImageR & dem, const int nmax, const bool down, const double mina);

// Filter result of trace_map:
//   smooth_passes -- number of smoothing passes
//   minpt -- min number of points in ridge segment
//   mindh -- min ridge hight [m]
//   dist  -- ridge width [pts]
// TODO: describe better
// TODO: merge lines
dMultiLine
trace_map_flt(const iMultiLine & data, const ImageR & dem, const bool down,
              const size_t smooth_passes = 2, const int minpt=3, const double mindh = 20, const double dist = 2);

#endif
