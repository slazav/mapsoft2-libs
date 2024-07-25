#ifndef IMAGE_TRACE_H
#define IMAGE_TRACE_H

#include <vector>
#include "geom/line.h"
#include "image/image_r.h"


/* Functions for river/mountain tracing */

/********************************************************************/
// Trace one river.
// Parameters:
//    p0 -- starting point
//    nmax -- no-sink area when we stop calculation
//    hmin -- threshold height where we want to stop calculation
//    down -- is the flow goes down (river) of up (mountain)
iLine
trace_river(const ImageR & img, const iPoint & p0, int nmax, int hmin, bool down);

// trace rectangular map, return sink directions
ImageR trace_map_dirs(const ImageR & img, int nmax, bool down);

// use sink directions to calculate areas
ImageR trace_map_areas(const ImageR & dirs);

#endif
