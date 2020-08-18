#ifndef CSS_H
#define CSS_H

#include <string>
#include <gtkmm.h>

// load css for a gtk application
void load_css(const std::string & fname, Gtk::Widget & w);

#endif