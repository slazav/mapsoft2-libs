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

// Scale objects
void fig_scale(std::list<FigObj> & objects,
    const double scale, const iPoint & p0 = iPoint(0,0));

// Shift objects
void fig_shift(std::list<FigObj> & objects, const iPoint & sh);

// Remove empty compounds
void fig_remove_empty_comp(std::list<FigObj> & objects);

// Remove all compounds
void fig_remove_comp(std::list<FigObj> & objects);

// Check if a fig object matches template.
// It compares only properties which are visually different:
// for example if line width is 0 then no need to compare color.

// Lines: depth and thickness should match;
//   if thickness > 0 color and line style should match;
//   fill type should match, if fill is not transparent fill color should match;
//   for hatch filling pen_color should match (even if thickness=0);
// Points: depth, thickness, color, cap_style should match
// Text: depth, color, font should match
// TODO: other types.
bool fig_match_template(const FigObj & o, const std::string & tmpl);

// Same but match two templates. Note that here difference between points and
// lines is lost (no point numbers)
bool fig_match_templates(const std::string & tmpl1, const std::string & tmpl2);


#endif
