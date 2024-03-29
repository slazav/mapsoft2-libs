/*
 Based on https://github.com/lyokato/libgeohash
 Modified for mapsoft2: slazav@altlinux.org, 2019-05-25
 */

#ifndef GEOHASH_H
#define GEOHASH_H

#include <string>
#include <set>
#include "geom/point.h"
#include "geom/rect.h"

// check if Geohash contains only valid characters
bool GEOHASH_verify(const std::string & hash);

// encode a point, return hash.
std::string GEOHASH_encode(const dPoint & p, size_t hash_length);

// Encode a rectangle, return longest hash (but not longer then maxlen)
// which covers the rectangle. Can return an ampty string.
std::string GEOHASH_encode(const dRect & r, size_t maxlen);

// Encode a rectangle, return up to 4 longest adjacent hashes (not
// longer then maxlen and non-empty) which covers the rectangle. Should be
// efficient (give longer hashas) for rectangles which cross hash bundaries.
std::set<std::string> GEOHASH_encode4(const dRect & r, size_t maxlen);

// Decode a hash, return coordinate range. Range is empty if hash is not valid.
dRect GEOHASH_decode(const std::string & hash);

// Find adjacent hash.
// Direction starts from north and goes clockwise (0:N, 1:NE, 2:E, 3:SE, 4:S, etc.)
// return empty string on errors.
std::string GEOHASH_adjacent(const std::string & hash, int dir);

// Sometimes it is needed to encode arbitrary type of coordinates,
// not lat-lon. This function does a linear transformation of
// coordinates in such a way that bbox -> dRect(-180,-90,360,90).
// If bbox is empty then original box is returned.
// If box is empty then empty box is returned.
dRect GEOHASH_encode_box(const dRect & box, const dRect & bbox);
dRect GEOHASH_decode_box(const dRect & box, const dRect & bbox);


#endif
