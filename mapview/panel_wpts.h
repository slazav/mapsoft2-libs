#ifndef MAPVIEW_PANEL_WPTS_H
#define MAPVIEW_PANEL_WPTS_H

#include "geo_render/gobj_wpts.h"
#include "panel.h"

/* Control panel for waypoint gobjects. */

class PanelWpts : public Panel<GObjWpts, GeoWptList> {
public:
    PanelWpts() { set_name("WPTS"); }

    void add(const std::shared_ptr<GeoWptList> & wpts) override;


    // Find waypoints in a rectangular area
    std::map<GObjWpts*, std::vector<size_t> > find_points(const iRect & r) const;

    // Find waypoints (point is in the waypoint circle or waypoint label).
    // Return point numbers, sorted by distance.
    std::map<GObjWpts*, std::vector<size_t> > find_points(const dPoint & pt) const;


    bool upd_name(GObjWpts * sel_gobj=NULL, bool dir=true);

    void on_select(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* col) override;

};

#endif
