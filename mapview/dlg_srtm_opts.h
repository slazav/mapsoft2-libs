#ifndef DIALOGS_SRTM_OPT_H
#define DIALOGS_SRTM_OPT_H

#include <gtkmm.h>
#include "opt/opt.h"
#include "w_rainbow.h"
#include "w_comboboxes.h"

class DlgSrtmOpt : public Gtk::Dialog{

    Gtk::CheckButton  *cnt, *shades, *peaks, *interp, *holes, *surf;
    Gtk::RadioButton  *m_heights, *m_slopes;
    Gtk::SpinButton   *cnt_val;
    Gtk::Button       *dirbtn;
    Gtk::Label        *dir;
    RainbowWidget     *rh, *rs;
    Gtk::FileChooserDialog fdlg;

    sigc::signal<void> signal_changed_;
    void on_ch(int mode, Gtk::RadioButton *b);

    void on_dirbtn();
    void on_fresult(int r);


  public:
    DlgSrtmOpt();
    Opt get_opt() const;
    void set_opt(const Opt & o = Opt());
    sigc::signal<void> & signal_changed() { return signal_changed_; }

};

#endif
