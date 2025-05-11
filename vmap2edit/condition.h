#ifndef VMAP2EDIT_COND_H
#define VMAP2EDIT_COND_H

/*
Conditions for Vmap2 editor.
*/

#include <list>
#include <string>
#include <vector>
#include "vmap2/vmap2obj.h"

struct VMap2cond: public std::list<std::vector<std::string> > {

  // Evaluate condition (a few sequences)
  bool eval(const VMap2obj & o) const;

  // Evaluate single condition for an object
  static bool eval_single(const std::string & c, const VMap2obj & o);

  // Evaluate sequence of conditions separated with "or" or "and"
  static bool eval_sequence(const std::vector<std::string> & s, const VMap2obj & o);

};

#endif
