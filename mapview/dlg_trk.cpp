#include "dlg_trk.h"
#include <iomanip>
#include <sstream>

DlgTrk::DlgTrk() {
  add_button (Gtk::Stock::OK,     Gtk::RESPONSE_OK);
  add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  signal_response().connect(
      sigc::hide(sigc::mem_fun(this, &DlgTrk::hide)));

  // Labels
  Gtk::Label *l_name  = manage(new Gtk::Label("Name:",       Gtk::ALIGN_END));
  Gtk::Label *l_width = manage(new Gtk::Label("Line Width:", Gtk::ALIGN_END));
  Gtk::Label *l_fg    = manage(new Gtk::Label("Color:",      Gtk::ALIGN_END));
  l_name->set_padding(3,0);
  l_width->set_padding(3,0);
  l_fg->set_padding(3,0);

  auto w_adj = Gtk::Adjustment::create(0,0,100);

  // Entries
  fg    = manage(new Gtk::ColorButton);
  name  = manage(new Gtk::Entry);
  width = manage(new Gtk::SpinButton(w_adj));
  info  = manage(new Gtk::Label);
  hint  = manage(new Gtk::Label);
  hint->set_line_wrap();

  // Table
  Gtk::Table *table = manage(new Gtk::Table(4,4));
            //  widget    l  r  t  b  x       y
  table->attach(*l_name,  0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*name,    1, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*l_width, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*width,   1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*l_fg,    2, 3, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*fg,      3, 4, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*info,    0, 4, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);
  table->attach(*hint,    0, 4, 4, 5, Gtk::FILL, Gtk::SHRINK, 3, 3);

  get_vbox()->add(*table);
}


void
DlgTrk::dlg2trk(GeoTrk * trk) const{
  if (!trk) return;
  trk->name = name->get_text();
  trk->opts.put("thickness", (int)width->get_value());
  Gdk::Color c = fg->get_color();
  uint32_t color=
    (((unsigned)c.get_red()   & 0xFF00) << 8) +
     ((unsigned)c.get_green() & 0xFF00) +
    (((unsigned)c.get_blue()  & 0xFF00) >> 8);
  trk->opts.put("color", color);
}
void
DlgTrk::trk2dlg(const GeoTrk * trk){
  if (!trk) return;
  name->set_text(trk->name);
  width->set_value(trk->opts.get("thickness", 1));
  int col = trk->opts.get("color", 0xFF0000FF);
  Gdk::Color c;
  c.set_rgb((col & 0xFF0000)>>8,
            (col & 0xFF00),
            (col & 0xFF)<<8);
  fg->set_color(c);
  set_info(trk);
}
void
DlgTrk::set_info(const GeoTrk * trk){
  if (!trk) return;
  std::ostringstream st;
  st << "Points: <b>"
     << trk->size() << "</b>, Length: <b>"
     << std::setprecision(2) << std::fixed
     << trk->length()/1000 << "</b> km";
        info->set_markup(st.str());
}

void
DlgTrk::set_hint(const char * str){
  hint->set_markup(str);
}
