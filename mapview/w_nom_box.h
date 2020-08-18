#ifndef W_NOM_BOX_H
#define W_NOM_BOX_H

#include "w_comboboxes.h"
#include "geo_data/conv_geo.h"

/*
  Soviet nomenclatorial map name selection widget.
  For "show point information" action
*/

class NomBox :  public Gtk::Frame {
public:
  NomBox();
//  NomBox(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  std::string get_nom() const;
  dPoint get_ll() const;
  int get_rscale();

  void set_ll(const dPoint &p);
  sigc::signal<void, dPoint> & signal_jump();

private:
  void init();

  void set_nom(const std::string &nom);
  void on_change_rscale();
  void move(int dx, int dy);

  Gtk::Entry * nom;
  CBScale * rscale;
  dPoint pt;
  ConvGeo cnv;
  sigc::signal<void, dPoint> signal_jump_;
};

#endif
