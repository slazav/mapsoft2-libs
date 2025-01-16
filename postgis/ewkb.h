#ifndef EWKB_H
#define EWKB_H

#include <string>
#include "geom/multiline.h"

/*
Decode WKB/EWKB.. ascii hex streams into dMultiLine.
EWKB is Postgis extension of WKB, a representation of
geometric forms as binary data. 

WKB https://www.ibm.com/docs/en/informix-servers/12.10?topic=geometry-description-wkbgeometry-byte-streams
EWKB https://github.com/postgis/postgis/blob/2.1.0/doc/ZMSgeoms.txt
some other forms can be found in postgis/liblwgeom/lwin_wkb.c
*/

dMultiLine ewkb_decode(const std::string & str, const bool docnv, const bool binary, size_t & pos);

dMultiLine ewkb_decode(const std::string & str, const bool docnv, const bool binary);

#endif
