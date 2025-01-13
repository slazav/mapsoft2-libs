#ifndef GEO_NOM_FI_H
#define GEO_NOM_FI_H

#include "geom/rect.h"
#include <string>
#include <set>
#include <iostream>

typedef enum {
 SC_FI_200k, // V5
 SC_FI_100k, // V51
 SC_FI_50k,  // V511
 SC_FI_25k,  // V5111
 SC_FI_10k,  // V5111A
 SC_FI_5k,   // V5111A1
 SC_FI_H200k, // V5L
 SC_FI_H100k, // V51L
 SC_FI_H50k,  // V511L
 SC_FI_H25k,  // V5111L
 SC_FI_H10k,  // V5111AL
 SC_FI_H5k,   // V5111A1L
} nom_scale_fi_t;

// See https://www.maanmittauslaitos.fi/en/maps-and-spatial-data/datasets-and-interfaces/product-descriptions/map-sheet-division

/*
Returns region for a given Finnish map "V51 etc.".
Coordinates are ETRS-TM35FIN meters.
If sc is not NULL, return scale.
*/
dRect nom_to_range_fi(const std::string & name, nom_scale_fi_t *sc = NULL);

/*
Find Finnish 1:100000 nomenclature map for a given point.
Coordinates are ETRS-TM35FIN meters.
*/
std::string pt_to_nom_fi(dPoint p, nom_scale_fi_t sc = SC_FI_100k);

#endif
