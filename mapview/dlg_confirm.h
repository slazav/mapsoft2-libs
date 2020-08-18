#ifndef MAPVIEW_DLG_CONFIRM_H
#define MAPVIEW_DLG_CONFIRM_H

#include <gtkmm.h>
#include <string>

// Confirmation dialog
// It has two buttons, OK and Cancel. When pressing OK button
// a function attached to the slot is called.

class DlgConfirm : public Gtk::MessageDialog{
  public:
    DlgConfirm(const std::string & msg = "Data was changed. Continue?");
    void call(const sigc::slot<void> & slot);

  private:
    sigc::signal<void> signal_ok_;
    sigc::slot<void> current_slot;
    void on_result(int r);
};

#endif
