#ifndef VMAP2EDIT_ACT_H
#define VMAP2EDIT_ACT_H

#include <memory>
#include "vmap2/vmap2.h"
#include "vmap2/vmap2obj.h"

// Actions for vmap2edit

// Base class
class VMap2action {
  public:
  VMap2 & vmap; // reference to VMap2 object;
  bool do_none; // do not process objects (do some operations in the constructor)
  bool do_once; // process only first object which matches conditions

  VMap2action(VMap2 & vmap): vmap(vmap), do_none(false), do_once(false){}

  // check number of arguments
  static void check_args(const std::vector<std::string> & cmd,
                         const std::vector<std::string> & expected);

  // create an action
  static std::shared_ptr<VMap2action> get_action(VMap2 & vmap, const std::vector<std::string> & cmd);

  // process a single object (action-specific)
  virtual bool process_object(uint32_t id, VMap2obj & o) = 0;

};

#endif
