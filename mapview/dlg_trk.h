#ifndef DIALOGS_TRK_H
#define DIALOGS_TRK_H

#include <gtkmm.h>
#include <geo_data/geo_data.h>

// dialog for Add/Edit Track actions
class DlgTrk : public Gtk::Dialog{
    Gtk::ColorButton *fg;
    Gtk::Entry *name;
    Gtk::SpinButton *width;
    Gtk::Label *info;
    Gtk::Label *hint;

  public:
    DlgTrk();

    void dlg2trk(GeoTrk * trk) const;
    void trk2dlg(const GeoTrk * trk);
    void set_info(const GeoTrk * trk);
    void set_hint(const char * str);
};

#endif
