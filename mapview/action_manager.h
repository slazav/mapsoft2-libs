#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include <vector>
#include <string>
#include <gtkmm.h>

#include "am.h"

class ActionManager {
public:
  ActionManager(Mapview * mapview_);

  // clear rubber, abort current mode
  void clear_state();

  // do clear_state, activate a new mode, put mode name to the statusbar
  void set_mode (int new_mode, const std::string & menu = std::string());

  void click (const iPoint p, const int button,
              const Gdk::ModifierType & state) {
    modes[current_mode]->handle_click(p, button, state); }

  // get main menu widget
  Gtk::Widget * get_main_menu() {
    return ui_manager->get_widget("/MenuBar"); }

private:

  // Menus
  Glib::RefPtr<Gtk::ActionGroup> actions;
  Glib::RefPtr<Gtk::UIManager> ui_manager;

  // Add action to a menu. Menu name is Popup* for pop-up menus
  // or Group name in Main menu (File, View, etc...)
  void AddAction(
    ActionMode *action,
    const std::string & id,
    const std::string & menu);

  // Add separator to a menu
  void AddSep(const std::string & menu);

  Mapview * mapview;
  std::vector<std::shared_ptr<ActionMode> > modes;
  int current_mode;

  // add maps from json file to the menu
  void AddMaps(const std::string & menu, const std::string & file);
};

#endif
