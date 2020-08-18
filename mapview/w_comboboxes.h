#ifndef W_COMBOBOXES_H
#define W_COMBOBOXES_H

#include "w_simple_combo.h"
#include "geom/point.h"
#include <string>

struct CBProj   : public SimpleCombo<std::string>  { CBProj(); };
struct CBScale  : public SimpleCombo<int>    { CBScale(); };
struct CBUnit   : public SimpleCombo<int>    { CBUnit(); };
struct CBPage   : public SimpleCombo<iPoint> { CBPage(); };
struct CBCorner : public SimpleCombo<int>    { CBCorner(); };

#endif
