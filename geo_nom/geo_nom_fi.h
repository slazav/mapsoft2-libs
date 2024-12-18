#ifndef GEO_NOM_FI_H
#define GEO_NOM_FI_H

#include "geom/rect.h"
#include <string>
#include <set>
#include <iostream>

/*
Returns region for a given Finnish 1:100000 nomenclature map "V51 etc.".
Coordinates are ETRS-TM35FIN meters.
*/
dRect nom_to_range_fi(const std::string & name);


/*
Find Finnish 1:100000 nomenclature map for a given point.
Coordinates are ETRS-TM35FIN meters.
*/
std::string pt_to_nom_fi(dPoint p);

#endif
