#ifndef COLORS_H
#define COLORS_H

// color handling functions

#include <stdint.h>
#include "err/err.h"

/*********************************************************************/
// Luminance (from ITU-R BT.601.5)
#define COLOR_LUMINR (0.2989)
#define COLOR_LUMING (0.5866)
#define COLOR_LUMINB (0.1145)

/*********************************************************************/

// distance between two colors
double color_dist(const uint32_t c1, const uint32_t c2, const bool prescaled=true);

// Assemble 32-bit color from a,r,g,b components.
// Prescaled semi-transparent colors are used
uint32_t color_argb(const uint8_t a, const uint8_t r,
                    const uint8_t g, const uint8_t b);

// Remove transparency (with color scaling).
// if gifmode = true, then keep fully transparent colors.
uint32_t color_rem_transp(const uint32_t c, const bool gifmode);

// Convert RGB color to 8-bit greyscale
uint8_t color_rgb_to_grey8(const uint32_t c);

// Convert RGB color to 16-bit greyscale
uint16_t color_rgb_to_grey16(const uint32_t c);


#endif
