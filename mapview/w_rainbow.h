#ifndef W_RAINBOW_H
#define W_RAINBOW_H

#include <gtkmm.h>
#include "rainbow/rainbow.h"
#include "image/image_r.h"


// Rainbow widget provides GUI to Rainbow class.

class RainbowWidget : public Gtk::Frame, public Rainbow{
  ImageR img;
  std::vector<Gtk::Label *> labels;
  Gtk::SpinButton *v1e, *v2e;
  int digits;

  void on_ch(); /// on values change: update labels, emit signal_changed
  sigc::signal<void> signal_changed_;

  public:

  RainbowWidget(
    int width, int height, // image width and height
    double l, double h, double s, double d, // spinbox settings: low, high, step, digits
    const char * colors = RAINBOW_NORMAL // rainbow color string
  );

  void set(double v1, double v2);
  double get_v1() const;
  double get_v2() const;
  sigc::signal<void> & signal_changed();
};


#endif
