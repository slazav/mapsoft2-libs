///\cond HIDDEN (do not show this in Doxyden)

#include <gtkmm.h>
#include "err/err.h"

#include "mapview/w_rainbow.h"
#include "mapview/w_comboboxes.h"

class MyWindow : public Gtk::ApplicationWindow {
  RainbowWidget * rainbow;
  CBCorner * cb_corner;

  public:

  void on_rainbow_ch(){
    std::cerr << "RainbowWidget: "
              << rainbow->get_v1() << " " << rainbow->get_v2() << "\n";
  }

  void on_cb_corner_ch(){
    std::cerr << "CBCorner: " << cb_corner->get_active_id() << "\n";
  }

  // load css
  void load_css(){
    std::string fname = "./widgets.css";
    auto css_provider = Gtk::CssProvider::create();
    if (!css_provider) throw Err() << "can't get Gtk::CssProvider";
    auto style_context = Gtk::StyleContext::create();
    if (!style_context) throw Err() << "can't get Gtk::StyleContext";
    struct stat st_buf;
    try{
      if (stat(fname.c_str(), &st_buf) == 0 &&
        css_provider->load_from_path(fname)){
        auto screen = get_screen();
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


  /***********************/
  MyWindow(){

    /***********************************/
    // Rainbow widget

    rainbow = manage(
       new RainbowWidget(300,8, -1000,10000,1,1, RAINBOW_NORMAL));
    rainbow->set(0,100); // set values
    rainbow->signal_changed().connect(
      sigc::mem_fun(this, &MyWindow::on_rainbow_ch));

    /***********************************/
    // Comboboxes

    Gtk::HBox * cb_box = manage(new Gtk::HBox);

    cb_corner = manage( new CBCorner());
    cb_corner->signal_changed().connect(
      sigc::mem_fun(this, &MyWindow::on_cb_corner_ch));
    cb_corner->set_first_id();

    Gtk::Label * label = manage( new Gtk::Label("Select corner:"));
    cb_box->pack_start(*label, false, false, 2);
    cb_box->pack_start(*cb_corner, false, false, 2);

    /***********************************/
    // Main vbox
    Gtk::VBox * vbox = manage(new Gtk::VBox);
    vbox->pack_start(*rainbow, false, true, 0);
    vbox->pack_start(*cb_box, false, true, 0);

    add (*vbox);
    load_css();
    show_all();
  }




};


int
main(int argc, char **argv){
  try {

    auto app = Gtk::Application::create();

    MyWindow w;
    app->run(w, argc, argv);

    return 0;

  }
  catch (Err & e) {
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond

