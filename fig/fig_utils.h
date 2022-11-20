#ifndef FIG_UTILS_H
#define FIG_UTILS_H

#include <list>
#include "fig.h"

// Size of fig objects
iRect fig_bbox(const std::list<FigObj> & objects);

// Make compound with fig objects
void fig_make_comp(std::list<FigObj> & objects);

// Rotate objects
void fig_rotate(std::list<FigObj> & objects,
    const double a, const iPoint & p0 = iPoint(0,0));

// Shift objects
void fig_shift(std::list<FigObj> & objects, const iPoint & sh);

// Remove empty compounds
void fig_remove_empty_comp(std::list<FigObj> & objects);

#endif
