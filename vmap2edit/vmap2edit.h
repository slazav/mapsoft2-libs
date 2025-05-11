#ifndef VMAP2EDIT_H
#define VMAP2EDIT_H

#include <string>
#include <vector>
#include "vmap2/vmap2.h"
#include "actions.h"
#include "condition.h"

// Run a single action
void vmap2edit_action(VMap2 & vmap, const std::vector<std::string> & cmd, const VMap2cond & conds);

// Read filter file, apply to vmap
void vmap2edit(VMap2 & vmap, const std::string & fname);

#endif
