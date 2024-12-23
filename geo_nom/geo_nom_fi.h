#ifndef GEO_NOM_FI_H
#define GEO_NOM_FI_H

#include "geom/rect.h"
#include <string>
#include <set>
#include <iostream>

typedef enum {
 SC_FI_200k = 200000,
 SC_FI_100k = 100000,
 SC_FI_50k  = 50000,
 SC_FI_25k  = 25000,
 SC_FI_10k  = 10000,
 SC_FI_5k   = 5000,
} nom_scale_fi_t;

// Only TM35 1:100'000 map division is implemented
// See https://www.maanmittauslaitos.fi/en/maps-and-spatial-data/datasets-and-interfaces/product-descriptions/map-sheet-division

/*
Returns region for a given Finnish 1:100000 nomenclature map "V51 etc.".
Coordinates are ETRS-TM35FIN meters.
*/
dRect nom_to_range_fi(const std::string & name);


/*
Find Finnish 1:100000 nomenclature map for a given point.
Coordinates are ETRS-TM35FIN meters.
*/
std::string pt_to_nom_fi(dPoint p, nom_scale_fi_t sc = SC_FI_100k);

#endif
