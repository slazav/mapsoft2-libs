#ifndef DIALOGS_PANO_H
#define DIALOGS_PANO_H

#include <gtkmm.h>
#include "viewer/dthread_viewer.h"
#include "viewer/rubber.h"
#include "srtm/srtm.h"
#include "geo_render/gobj_pano.h"
#include "w_rainbow.h"

// dialog for show point action
class DlgPano : public Gtk::Dialog{
    GObjPano gobj_pano;
    DThreadViewer viewer;
    Rubber rubber;
    Gtk::SpinButton *az;
    Gtk::SpinButton *dh;
    Gtk::SpinButton *mr;

    dPoint target; // target point (place to keep it during origin change)

    bool on_key_press(GdkEventKey * event);
    void click (iPoint p, int button, const Gdk::ModifierType & state);

    sigc::signal<void, dPoint> signal_go_;
    sigc::signal<void, dPoint> signal_point_;

  public:
    DlgPano(SRTM * s);

    void set_origin(const dPoint & pt);
    void set_target(const dPoint & pt, bool scroll=true);

    void on_set_az();
    void get_az(const iPoint & p); // update az value from viewer signal
    void on_set_mr();
    void on_set_dh();

    void redraw() {viewer.redraw();}

    sigc::signal<void, dPoint> signal_go();
    sigc::signal<void, dPoint> signal_point();
};


#endif
