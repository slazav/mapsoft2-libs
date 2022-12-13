#include "dlg_tpt.h"
#include "time_fmt/time_fmt.h"
#include <sstream>
#include <iostream>
#include <iomanip>


DlgTpt::DlgTpt(){
  add_button (Gtk::Stock::OK,     Gtk::RESPONSE_OK);
  add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  // Labels
  Gtk::Label *l_alt  = manage(new Gtk::Label("Altitude:",      Gtk::ALIGN_END));
  Gtk::Label *l_time = manage(new Gtk::Label("Date and Time:", Gtk::ALIGN_END));
  l_alt->set_padding(3,0);
  l_time->set_padding(3,0);

  // Entries
  alt   = manage(new Gtk::Entry);
  time  = manage(new Gtk::Entry);
  coord = manage(new CoordBox);
  start = manage(new Gtk::CheckButton("Start point of a new segment"));

  // Table
  Gtk::Table *table = manage(new Gtk::Table(2,4));

            //  widget    l  r  t  b  x       y
  table->attach(*coord,   0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*l_alt,   0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*alt,     1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*l_time,  0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*time,    1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*start,   1, 2, 3, 4, Gtk::FILL, Gtk::SHRINK, 3, 3);

  get_vbox()->add(*table);
}

void
DlgTpt::dlg2tpt(GeoTpt & tpt) const{
  tpt.dPoint::operator=(coord->get_ll());
  tpt.start = start->get_active();
  tpt.t  = parse_utc_time(time->get_text());

  double d;
  std::istringstream s(alt->get_text());
  s >> d;
  if (s.fail()) tpt.clear_alt();
  else tpt.z=d;
}

void
DlgTpt::tpt2dlg(const GeoTpt & tpt){
  coord->set_ll(tpt);
  start->set_active(tpt.start);
  time->set_text(write_fmt_time("%F %T%f", tpt.t));

  if (tpt.have_alt()){
    std::ostringstream s;
    s.setf(std::ios::fixed);
    s << std::setprecision(1) <<tpt.z;
    alt->set_text(s.str());
  }
  else alt->set_text("");
}

sigc::signal<void, dPoint>
DlgTpt::signal_jump(){
  return coord->signal_jump();
}
