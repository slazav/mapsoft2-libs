#ifndef DIALOGS_WPT_H
#define DIALOGS_WPT_H

#include <gtkmm.h>
#include <geo_data/geo_data.h>
#include "w_coord_box.h"

// dialog for Add/Edit Waypoint actions
class DlgWpt : public Gtk::Dialog{
    CoordBox * coord;
//    Gtk::ColorButton *fg, *bg;
    Gtk::Entry *name, *comm, *alt, *time;
//    Gtk::SpinButton *fs, *ps;

  public:
    DlgWpt();

    void dlg2wpt(GeoWpt & wpt) const;
    void wpt2dlg(const GeoWpt & wpt);

    sigc::signal<void, dPoint> signal_jump() {return coord->signal_jump();}
    void set_ll(const dPoint & p) {coord->set_ll(p);}
};

#endif
