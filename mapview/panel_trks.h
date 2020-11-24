#ifndef MAPVIEW_PANEL_TRKS_H
#define MAPVIEW_PANEL_TRKS_H

#include "geo_render/gobj_trk.h"
#include "panel.h"

/* Control panel for track gobjects */

class PanelTrks : public Panel<GObjTrk, GeoTrk> {
public:

    PanelTrks () { set_name("TRKS"); }

    ptr_t add(const std::shared_ptr<GeoTrk> & trk) override;


    // Find track points in a rectangular area
    std::map<ptr_t, std::vector<size_t> > find_points(const iRect & r) const;

    // Find track points near pt.
    std::map<ptr_t, std::vector<size_t> > find_points(const dPoint & pt) const;

    // Find segments near pt.
    std::map<ptr_t, std::vector<size_t> > find_segments(const dPoint & pt) const;

    // Delete points in a rectangular area
    void del_points(const iRect & r, const ptr_t & obj);

    bool upd_name(ptr_t sel_gobj=NULL, bool dir=true);

    void on_select(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* col) override;

};

#endif
