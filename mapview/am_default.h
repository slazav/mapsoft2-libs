#ifndef AM_DEFAULT_H
#define AM_DEFAULT_H

/* Action modes for File menu */
#include "am.h"

class ActionModeDefault : public ActionMode {
public:
  ActionModeDefault (Mapview * mapview) :
    ActionMode(mapview) { }
  std::string get_name() override { return std::string(); }

};


#endif
