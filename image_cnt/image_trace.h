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

// Trace rivers or mountains on the image.
//   dem -- altitude map
//   nmax -- No-sink area when we stop calculation
//   down -- Is the flow goes down (river) of up (mountain)
//   mina -- Threshold of the sink area.
//   mindh -- Threshold of difference between altitude and mean altitude
//            of the sink area (dh parameter).
// Tracing starts when thresholds are reached and continue
// unlil a no-sink point of map edge, even if dh parameter goes below threshold.
dMultiLine
trace_map(const ImageR & dem, const int nmax,
          const bool down, const double mina, const double mindh);

#endif
