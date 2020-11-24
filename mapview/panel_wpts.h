#ifndef MAPVIEW_PANEL_WPTS_H
#define MAPVIEW_PANEL_WPTS_H

#include "geo_render/gobj_wpts.h"
#include "panel.h"

/* Control panel for waypoint gobjects. */

class PanelWpts : public Panel<GObjWpts, GeoWptList> {
public:
    PanelWpts() { set_name("WPTS"); }

    ptr_t add(const std::shared_ptr<GeoWptList> & wpts) override;

    // Find waypoints in a rectangular area
    std::map<ptr_t, std::vector<size_t> > find_points(const iRect & r) const;

    // Find waypoints (point is in the waypoint circle or waypoint label).
    // Return point numbers, sorted by distance.
    std::map<ptr_t, std::vector<size_t> > find_points(const dPoint & pt) const;

    // Delete points in a rectangular area
    void del_points(const iRect & r, const ptr_t & obj);

    bool upd_name(ptr_t sel_gobj = NULL, bool dir=true);

    void on_select(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* col) override;

};

#endif
