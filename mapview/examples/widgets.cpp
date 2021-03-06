///\cond HIDDEN (do not show this in Doxyden)

#include <gtkmm.h>
#include "err/err.h"

#include "mapview/css.h"
#include "mapview/w_rainbow.h"
#include "mapview/w_comboboxes.h"
#include "mapview/w_coord_box.h"
#include "mapview/w_nom_box.h"
#include "mapview/w_page_box.h"

class MyWindow : public Gtk::ApplicationWindow {
  RainbowWidget * rainbow;
  CBCorner * cb_corner;
  CoordBox * crds;
  NomBox   * nom;
  PageBox  * page;

  public:

  void on_rainbow_ch(){
    std::cerr << "RainbowWidget: "
              << rainbow->get_v1() << " " << rainbow->get_v2() << "\n";
  }

  void on_cb_corner_ch(){
    std::cerr << "CBCorner: " << cb_corner->get_active_id() << "\n";
  }

  void on_crds_jump(const dPoint & p){
    std::cerr << "Jump to: " << p << "\n";
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
    // Combobox

    cb_corner = manage( new CBCorner());
    cb_corner->signal_changed().connect(
      sigc::mem_fun(this, &MyWindow::on_cb_corner_ch));
    cb_corner->set_first_id();

    // pack in hbox together with a label
    Gtk::HBox * cb_box = manage(new Gtk::HBox);
    Gtk::Label * label = manage( new Gtk::Label("Select corner:"));
    cb_box->pack_start(*label, false, false, 2);
    cb_box->pack_start(*cb_corner, false, false, 2);

    /***********************************/
    // Coordinate box
    crds = manage(new CoordBox());
    crds->set_ll(dPoint(70.56072,39.43434));
    crds->signal_jump().connect(
      sigc::mem_fun(this, &MyWindow::on_crds_jump));

    /***********************************/
    // Nom box
    nom = manage(new NomBox());
    nom->set_ll(dPoint(70.56072,39.43434));
    nom->signal_jump().connect(
      sigc::mem_fun(this, &MyWindow::on_crds_jump));

    /***********************************/
    // Page box
    page = manage(new PageBox());


    /***********************************/
    /***********************************/
    // Main vbox
    Gtk::VBox * vbox = manage(new Gtk::VBox);
    vbox->pack_start(*rainbow, false, true, 5);
    vbox->pack_start(*cb_box,  false, true, 5);
    vbox->pack_start(*crds,    false, true, 5);
    vbox->pack_start(*nom,     false, true, 5);
    vbox->pack_start(*page,    false, true, 5);


    add (*vbox);
    load_css("./widgets.css", *this);
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

