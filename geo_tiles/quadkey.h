#ifndef QUADKEY_H
#define QUADKEY_H

#include "err/err.h"
#include <string>

///\addtogroup libmapsoft
///@{
///\defgroup Tiles
///Quadkey calculator
///@{

// Quadkey handling.
// See https://docs.microsoft.com/en-us/bingmaps/articles/bing-maps-tile-system

// Quadkey -> XYZ tiles
void quadkey_to_tile(const std::string & q, int &x, int &y, int &z);

// XYZ tile -> Quadkey
std::string tile_to_quadkey(const int x, const int y, const int z);

#endif
