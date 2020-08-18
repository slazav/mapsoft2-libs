#include "css.h"
#include "err/err.h"

// load css
void load_css(const std::string & fname, Gtk::Widget & w){
  auto css_provider = Gtk::CssProvider::create();
  if (!css_provider) throw Err() << "can't get Gtk::CssProvider";
  auto style_context = Gtk::StyleContext::create();
  if (!style_context) throw Err() << "can't get Gtk::StyleContext";
  struct stat st_buf;
  try{
    if (stat(fname.c_str(), &st_buf) == 0 &&
      css_provider->load_from_path(fname)){
      auto screen = w.get_screen();
      if (!screen) throw Err() << "can't get screen";
      style_context->add_provider_for_screen(
         screen, css_provider,
         GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
  }
  catch (Glib::Error e){
    std::cerr << "Mapview: Reading CSS files: " << e.what() << "\n";
  }
}
