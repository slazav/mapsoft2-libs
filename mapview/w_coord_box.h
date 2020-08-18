#ifndef W_COORD_BOX_H
#define W_COORD_BOX_H

#include "w_comboboxes.h"
#include "geo_data/conv_geo.h"

/*
  Geo-coordinates selection widget.
  For "show point information" or "add waypoint" dialogs.
*/

class CoordBox : public Gtk::Frame {
public:
  CoordBox();
//  CoordBox(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void set_ll(const dPoint &p);
  dPoint get_ll();
  sigc::signal<void> & signal_changed();
  sigc::signal<void, dPoint> & signal_jump();

private:
  Gtk::Entry *coords;
  CBProj  * proj_cb;
  sigc::signal<void> signal_changed_;
  sigc::signal<void, dPoint> signal_jump_;
  dPoint old_pt; // to fix incorrect values
  ConvGeo cnv;

  dPoint get_xy();
  void init();
  void on_conv();
  void on_change();
  void on_jump();
};

#endif
