#ifndef DIALOGS_TRK_OPT_H
#define DIALOGS_TRK_OPT_H

#include <gtkmm.h>
#include "opt/opt.h"
#include "w_rainbow.h"

class DlgTrkOpt : public Gtk::Dialog{

    Gtk::CheckButton  *dots;
    Gtk::RadioButton  *m_normal, *m_speed, *m_height;
    RainbowWidget *rv, *rh;
    sigc::signal<void> signal_changed_;

    void on_ch(int mode, Gtk::RadioButton *b);

  public:
    DlgTrkOpt();
    Opt get_opt() const;
    void set_opt(const Opt & o = Opt());
    sigc::signal<void> & signal_changed();

};

#endif
