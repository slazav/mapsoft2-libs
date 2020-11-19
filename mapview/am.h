#ifndef AM_H
#define AM_H

#include "geom/point.h"
#include "geom/rect.h"
#include <gtkmm.h>
#include <map>

class Mapview;

// ActionMode interface.

class ActionMode {
public:

    Mapview * mapview;
    ActionMode(Mapview * mapview_) : mapview(mapview_) { }

    // Returns name, stock id, accel key, tooltip for the mode.
    virtual std::string get_name() = 0;
    virtual std::string get_icon() { return ""; }
    virtual std::string get_desc() { return ""; }
    virtual Gtk::AccelKey get_acckey()  { return Gtk::AccelKey(""); }

    // for non-radio modes only activate method is called.
    virtual bool is_radio() { return true; }

    // Activates this mode (menu parameter shows from which menu it was called).
    virtual void activate(const std::string & menu) { }

    // Abandons any action in progress and deactivates mode.
    // no need to reset rubber
    virtual void abort() { }

    // Sends user click. Coordinates are in workplane's discrete system.
    virtual void handle_click(const iPoint p, const int button,
                              const Gdk::ModifierType & state) { }
};

class ActionModeNone : public ActionMode {
public:
    ActionModeNone (Mapview * mapview) : ActionMode(mapview) { }
    virtual std::string get_name() override { return ""; }
};

#endif
