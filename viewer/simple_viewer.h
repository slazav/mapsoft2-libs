#ifndef SIMPLE_VIEWER
#define SIMPLE_VIEWER

#include <gtkmm.h>
#include <cairomm/context.h>
#include "cairo/cairo_wrapper.h"
#include "gobj.h"

///\addtogroup gred
///@{
///\defgroup simple_viewer
///@{

/**
Simple viewer for GObj. One can drag and rescale the object.
Drawing is done on demand.
This viewer is good only for objects which can draw fast.
For slow objects see DThreadViewer.
*/

class SimpleViewer : public Gtk::DrawingArea {
  public:

    SimpleViewer(GObj * o = NULL);

    // Change object.
    virtual void   set_obj (GObj * o) {obj=o; redraw();}

    // Get pointer to the current object.
    virtual GObj * get_obj (void) const {return obj;}


    // Scroll to a position (coordinates of top-left corner)
    virtual void   set_origin (iPoint new_origin);

    // Get coordinates of top-left corner.
    virtual iPoint get_origin (void) const { return origin;}

    // Get coordinates of the center (in object/viewer coordinates).
    virtual dPoint get_center (bool obj_crd) const;

    // Scroll to a position (viewer/object coordinates of the center)
    virtual void set_center(dPoint new_center, bool obj_crd);

    // Get view range (in object/viewer coordinates).
    virtual dRect get_range (bool obj_crd) const;

    // Scroll to an area (viewer/object coordinates)
    virtual void set_range(dRect r, bool obj_crd);



    // Set background color.
    virtual void set_bgcolor(int c) {bgcolor=c | 0xFF000000;}

    // Get background color.
    virtual int get_bgcolor(void) const {return bgcolor;}

    // Redraw the whole window or a rectangle in it
    virtual void redraw (const iRect & range = iRect());

    // Set conversion viewer -> object coordinates,
    // if fix_range=true, do scroll/resize to keep roughly same area on the screen.
    virtual void set_cnv(std::shared_ptr<ConvBase> c, bool fix_range);

    // get conversion
    virtual ConvBase & get_cnv() const {return *cnv;}

    // Set drawing options
    virtual void set_opt(const Opt & o);

    // rescale keeping the p0 point on the place
    virtual void rescale(const double k, const iPoint & p0);

    // rescale keeping center on the place
    virtual void rescale(const double k){
      rescale(k,iPoint(get_width(), get_height())/2); }

    // Handler for GTK draw sidnal. It calculates which parts should be redrawn
    // and calls draw() method.
    virtual bool on_draw (Cairo::RefPtr<Cairo::Context> const & cr) override;

    // Redraw part of the screen (will be overriden in DThreadViewer).
    virtual void draw(const CairoWrapper & crw, const iRect & r);

    // Drag the image by pressing button 1 or 2
    virtual bool on_button_press_event (GdkEventButton * event);
    virtual bool on_button_release_event (GdkEventButton * event);
    virtual bool on_motion_notify_event (GdkEventMotion * event);
    virtual bool is_on_drag() const {return on_drag;}

    // +/- for rescaling, "r" for redraw
    virtual bool on_key_press (GdkEventKey * event); // read note in simple_viewer.cpp

    // rescaling
    virtual bool on_scroll_event (GdkEventScroll * event);

    sigc::signal<void> & signal_busy()        {return signal_busy_;}
    sigc::signal<void> & signal_idle()        {return signal_idle_;}
    sigc::signal<void, double> & signal_on_rescale()  {return signal_on_rescale_;}
    sigc::signal<void, iPoint> & signal_ch_origin()   {return signal_ch_origin_;}
    sigc::signal<void, iPoint, int, const Gdk::ModifierType&> & signal_click() {return signal_click_;}

    void set_bbox(const dRect & r) {bbox = r;}
    dRect get_bbox() const {return bbox;}
    void reset_bbox() { bbox = dRect(); }
    void set_xloop(const bool v = true) {xloop = v;}
    bool get_xloop() const {return xloop;}

  private:

    sigc::signal<void> signal_busy_;
    sigc::signal<void> signal_idle_;
    sigc::signal<void, double> signal_on_rescale_;
    sigc::signal<void, iPoint> signal_ch_origin_;
    sigc::signal<void, iPoint, int, const Gdk::ModifierType&> signal_click_;

    iPoint origin;

    std::shared_ptr<ConvBase> cnv;
    Opt opt;

    bool on_drag;
    iPoint drag_pos, drag_start;

    int bgcolor;
    double sc;

    dRect bbox;
    bool xloop, yloop;

  protected:
    GObj * obj;

};

#endif
