#ifndef GOBJ_TEST_TILE_H
#define GOBJ_TEST_TILE_H

#include "viewer/gobj.h"

class GObjTestTile: public GObj{
  bool slow;
public:
  GObjTestTile(const bool slow_ = false);
  ret_t draw(const CairoWrapper & cr, const dRect &box) override;
};

#endif
