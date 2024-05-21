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

// Assemble 64-bit color from a,r,g,b components.
// Prescaled semi-transparent colors are used
uint64_t color_argb64(const uint16_t a, const uint16_t r,
                      const uint16_t g, const uint16_t b);

// Convert to prescale color
uint32_t color_prescale(const uint32_t c);

// Remove transparency (with color scaling).
// if gifmode = true, then keep fully transparent colors.
uint32_t color_rem_transp(const uint32_t c, const bool gifmode);
uint64_t color_rem_transp64(const uint64_t c, const bool gifmode);

// Convert RGB color to 8-bit greyscale
uint8_t color_rgb_to_grey8(const uint32_t c);
uint8_t color_rgb64_to_grey8(const uint64_t c);

// Convert RGB color to 16-bit greyscale
uint16_t color_rgb_to_grey16(const uint32_t c);
uint16_t color_rgb64_to_grey16(const uint64_t c);

// Convert RGB color from 64 to 32 bpp
uint32_t color_rgb_64to32(const uint64_t c);

// Convert RGB color from 32 to 64 bpp
uint64_t color_rgb_32to64(const uint32_t c);

// Invert RGB color, keep transparency
uint32_t color_rgb_invert(const uint32_t c);

// Invert RGB 64bit color, keep transparency
uint64_t color_rgb64_invert(const uint64_t c);


#endif
