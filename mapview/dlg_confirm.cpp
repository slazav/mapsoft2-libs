#include "dlg_confirm.h"

DlgConfirm::DlgConfirm(const std::string & msg):
     Gtk::MessageDialog(msg, false,
                        Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL){
  signal_response().connect(
      sigc::mem_fun(this, &DlgConfirm::on_result));
  set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void
DlgConfirm::call(const sigc::slot<void> & slot){
  sigc::connection conn(current_slot);
  conn.disconnect();
  current_slot=slot;
  signal_ok_.connect(slot);
  show_all();
}

void
DlgConfirm::on_result(int r){
  if (r == Gtk::RESPONSE_OK) signal_ok_.emit();
  hide();
}
