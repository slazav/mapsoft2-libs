#include "w_rainbow.h"
#include <sstream>
#include <iomanip>

using namespace std;

RainbowWidget::RainbowWidget( int width, int height,
    double l, double h, double s, double d, const char * colors):
      img(width, height, IMAGE_32ARGB), digits(d){

  if (width < 5 || height < 5) throw Err()
     << "RainbowWidget: too small size: " << width << " " << height;
//  set_size_request(width+10, height+10);

  // we use as much ticks as 
  int size = strlen(colors);

  // rainbow image:
  double dx = 0.5/(size-1);
  Rainbow R(0+dx,1-dx, colors);
  img.fill32(0);
  for (int i = 0; i<width; i++){
    uint32_t c = R.get(i/(double)(width-1));
    for (int j = 3; j < height; j++) img.set32(i,j,c);
  }

  // ticks and labels
  Gtk::HBox * hbox = manage(new Gtk::HBox());
  for (int i = 0; i<size; i++){
    int x = (i*(width-1))/size + (width-1)/size/2;
    for (int j = 0; j < height; j++) img.set32(x,j,0xFF000000);
    Gtk::Label * l = manage(new Gtk::Label());
    hbox->pack_start(*l, true, false, 1);
    labels.push_back(l);
  }

  Gtk::Table *t = manage(new Gtk::Table(4,3));
  Gtk::Label * v1l = Gtk::manage(new Gtk::Label("from:", Gtk::ALIGN_CENTER));
  Gtk::Label * v2l = Gtk::manage(new Gtk::Label("to:", Gtk::ALIGN_CENTER));

  auto v1a = Gtk::Adjustment::create(l,l,h,s);
  auto v2a = Gtk::Adjustment::create(h,l,h,s);

  v1e = manage(new Gtk::SpinButton(v1a, 0,d));
  v2e = manage(new Gtk::SpinButton(v2a, 0,d));
  v1e->set_value(l);
  v2e->set_value(h);

  Gtk::Image * i = Gtk::manage(new Gtk::Image(
        Gdk::Pixbuf::create_from_data (img.data(), Gdk::COLORSPACE_RGB,\
        true /*alpha*/, 8 /*bps*/, img.width() /*w*/, img.height() /*h*/,
        img.width()*4 /*rowstride*/)));

  // widget - left - right - top - buttom - xopt - yopt - xpad - ypad
  t->attach(*v1l,      0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*v1e,      1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*v2l,      2, 3, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*v2e,      3, 4, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*hbox,     0, 4, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*i,        0, 4, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);

  add(*t);
  on_ch();
  v1e->signal_value_changed().connect(
      sigc::mem_fun(this, &RainbowWidget::on_ch));
  v2e->signal_value_changed().connect(
      sigc::mem_fun(this, &RainbowWidget::on_ch));

  // set widget style (for use in css)
  get_style_context()->add_class("rainbow_widget");
}

void
RainbowWidget::on_ch(){
  int n = labels.size();
  double v1 = v1e->get_value();
  double v2 = v2e->get_value();
  for (int i = 0; i<n; i++){
    ostringstream s;
    s << fixed << setprecision(digits) << v1 + (v2-v1)*i/(n-1);
    labels[i]->set_text(s.str());
  }
  signal_changed_.emit();
}

void
RainbowWidget::set(double v1, double v2){
  v1e->set_value(v1);
  v2e->set_value(v2);
}

double
RainbowWidget::get_v1() const{
  return v1e->get_value();
}

double
RainbowWidget::get_v2() const{
  return v2e->get_value();
}

sigc::signal<void> &
RainbowWidget::signal_changed(){
  return signal_changed_;
}
