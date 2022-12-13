#ifndef DIALOGS_TPT_H
#define DIALOGS_TPT_H

#include <gtkmm.h>
#include "geo_data/geo_data.h"
#include "w_coord_box.h"

// dialog for Edit trackpoint action
class DlgTpt : public Gtk::Dialog{
    CoordBox * coord;
    Gtk::Entry *time, *alt;
    Gtk::CheckButton *start;

  public:
    DlgTpt();

    void dlg2tpt(GeoTpt & tpt) const;
    void tpt2dlg(const GeoTpt & tpt);

    sigc::signal<void, dPoint> signal_jump();
};

#endif
