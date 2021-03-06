#include "w_page_box.h"

double u2mm(int u, double d){
  switch (u){
    case 0: return 25.4/d; // px
    case 1: return 10.0;   // cm
    case 2: return 25.4;   // in
  }
  return 1;
}

double u2px(int u, double d){
  switch (u){
    case 1: return d/2.54; // cm
    case 2: return d;      // in
  }
  return 1;
}

// set spinbutton range and digets for different units
void set_spinpars(Gtk::SpinButton * x, int u){
  switch (u){
    case 0: // px
      x->set_digits(0);
      x->set_range(1, 99999);
      break;
    case 1: // cm
      x->set_digits(2);
      x->set_range(0.01, 99999);
      break;
    case 2: // in
      x->set_digits(2);
      x->set_range(0.01, 99999);
      break;
  }
}

PageBox::PageBox() {

  /*** 1st hbox ***/
  Gtk::HBox * hb1 = manage(new Gtk::HBox);

  Gtk::Label *l1 = manage(
    new Gtk::Label("Size:",  Gtk::ALIGN_END));
  l1->set_padding(3,0);
  Gtk::Label *l2 = manage(
    new Gtk::Label("x",  Gtk::ALIGN_CENTER));
  l2->set_padding(3,0);
  Gtk::Label *l3 = manage(
    new Gtk::Label("DPI:",  Gtk::ALIGN_END));
  l3->set_padding(3,0);

  auto x_adj    = Gtk::Adjustment::create(1,1,99999);
  auto y_adj    = Gtk::Adjustment::create(1,1,99999);
  auto marg_adj = Gtk::Adjustment::create(0, 0, 99999);
  auto dpi_adj  = Gtk::Adjustment::create(300, 1, 9999, 50);

  x   = manage(new Gtk::SpinButton(x_adj));
  y   = manage(new Gtk::SpinButton(y_adj));
  dpi = manage(new Gtk::SpinButton(dpi_adj,0,1));
  units = manage(new CBUnit());

  //                  expand, fill, padding
  hb1->pack_start(*l1, false, false, 3);
  hb1->pack_start(*x, false, false, 0);
  hb1->pack_start(*l2, false, false, 0);
  hb1->pack_start(*y, false, false, 0);
  hb1->pack_start(*units, false, false, 5);
  hb1->pack_start(*l3, false, false, 5);
  hb1->pack_start(*dpi, false, false, 3);

  /*** 2nd hbox ***/
  Gtk::HBox * hb2 = manage(new Gtk::HBox);

  Gtk::Label *l4 = manage(
    new Gtk::Label("Page:", Gtk::ALIGN_END));
  l4->set_padding(3,0);
  Gtk::Label *l5 = manage(
    new Gtk::Label("Margins:", Gtk::ALIGN_END));
  l5->set_padding(3,0);

  landsc = manage(new Gtk::CheckButton("Landscape"));
  marg = manage(new Gtk::SpinButton(marg_adj));
  page = manage(new CBPage());
  m_units = manage(new CBUnit());

  hb2->pack_start(*l4, false, false, 3);
  hb2->pack_start(*page, false, false, 3);
  hb2->pack_start(*l5, false, false, 3);
  hb2->pack_start(*marg, false, false, 3);
  hb2->pack_start(*m_units, false, false, 3);
  hb2->pack_start(*landsc, false, false, 3);

  /*** main vbox ***/
  Gtk::VBox * vb = manage(new Gtk::VBox);
  vb->pack_start(*hb1, false, false, 3);
  vb->pack_start(*hb2, false, false, 3);
  add(*vb);

  /*** default units ***/
  units->set_active_id(0);
  m_units->set_active_id(1);
  old_u = units->get_active_id();
  old_mu = m_units->get_active_id();
  set_spinpars(x, old_u);
  set_spinpars(y, old_u);
  set_spinpars(marg, old_mu);
  marg->set_value(0.65);
  dpi->set_value(300.0);

  /*** signals ***/
  no_ch=false;
  x->signal_changed().connect(
    sigc::mem_fun(this, &PageBox::ch_value));
  y->signal_changed().connect(
    sigc::mem_fun(this, &PageBox::ch_value));

  units->signal_changed().connect(
    sigc::mem_fun(this, &PageBox::ch_units));
  m_units->signal_changed().connect(
    sigc::mem_fun(this, &PageBox::ch_units));

  page->signal_changed().connect(
    sigc::mem_fun(this, &PageBox::ch_page));
  marg->signal_changed().connect(
    sigc::mem_fun(this, &PageBox::ch_page));
  dpi->signal_changed().connect(
    sigc::mem_fun(this, &PageBox::ch_page));
  landsc->signal_toggled().connect(
    sigc::mem_fun(this, &PageBox::ch_page));

  page->set_active(2); // set A4

}



void
PageBox::ch_units(){
  int mu = m_units->get_active_id();
  int u = units->get_active_id();
  double d = dpi->get_value();
  //iPoint pi = page->get_active_id();
  no_ch=true;
  if (u!=old_u){
    double f = u2mm(old_u,d) / u2mm(u,d);
    double xv = x->get_value();
    double yv = y->get_value();
    set_spinpars(x, u);
    set_spinpars(y, u);
    x->set_value( xv * f);
    y->set_value( yv * f);
  }
  if (mu!=old_mu){
    double f = u2mm(old_mu,d) / u2mm(mu,d);
    double v = marg->get_value();
    set_spinpars(marg, mu);
    marg->set_value( v * f);
  }
  old_mu = mu;
  old_u = u;
  no_ch=false;
}

void
PageBox::ch_page(){
  if (no_ch) return;
  dPoint p = page->get_active_id();
  if (p.x==0) { signal_changed_.emit(); return; }

  double d = dpi->get_value();

  // convert margins to mm
  double m = marg->get_value() *
       u2mm(m_units->get_active_id(),d);
  p-=dPoint(m,m)*2;
  if (p.x<0) p.x=0;
  if (p.y<0) p.y=0;

  // convert p to interface units:
  p/=u2mm(units->get_active_id(),d);

  bool sw = landsc->get_active();
  no_ch=true;
  x->set_value(sw? p.y:p.x);
  y->set_value(sw? p.x:p.y);
  no_ch=false;
  signal_changed_.emit();
}

void
PageBox::ch_value(){
  if (no_ch) return;
  page->set_active_id(iPoint(0,0));
  signal_changed_.emit();
}

int
PageBox::get_dpi(){
  return dpi->get_value();
}

dPoint
PageBox::get_px(){
  double f = u2px(units->get_active_id(), dpi->get_value());
  return dPoint(x->get_value(), y->get_value()) * f;
}

void
PageBox::set_px(const dPoint & p){
  double f = u2px(units->get_active_id(), dpi->get_value());
  no_ch=true;
  x->set_value(p.x / f);
  y->set_value(p.y / f);
  no_ch=false;
  ch_value();
}

sigc::signal<void> &
PageBox::signal_changed(){
  return signal_changed_;
}
