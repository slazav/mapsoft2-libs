#ifndef W_PAGE_BOX_H
#define W_PAGE_BOX_H

#include "w_comboboxes.h"
#include "geom/point.h"

// widget for page selection
class PageBox : public Gtk::Frame{
  CBPage *page;
  CBUnit *units, *m_units;

  Gtk::CheckButton *landsc;
  Gtk::SpinButton *marg, *x, *y, *dpi;

  void ch_units(); // on units/dpi change
  void ch_page();  // on page/landscape/dpi change
  void ch_value(); // on x,y value change

  int old_mu, old_u;
  bool no_ch;
  sigc::signal<void> signal_changed_;

  public:
    PageBox();
    int get_dpi();
    dPoint get_px();
    void set_px(const dPoint & p);
    sigc::signal<void> & signal_changed();

};

#endif
