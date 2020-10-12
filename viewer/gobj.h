#ifndef GRED_GOBJ_H
#define GRED_GOBJ_H

#include "geom/rect.h"
#include "opt/opt.h"
#include "cairo/cairo_wrapper.h"
#include "conv/conv_base.h"
#include <sigc++/sigc++.h>
#include <mutex> // Mutex, Lock
#include <memory> // shared_ptr

///\addtogroup gred
///@{
///\defgroup gobj
///@{

/**
An object which know how to draw itself using Cairo::Context.

- Object has its own coordinate system. Conversion from viewer to object
  coordinates is set by set_cnv() method.
- For the `draw` method the caller should provide
  `draw_range` in viewer coordinates and translate Cairo::Context to
  viewer coordinate origin.
- There is no need to save/restore Cairo::Context in GObj,
  it should be done by caller if needed.
- There is no need to set clip to `draw_range`, it should be done
  by coller if needed.
- `draw` method can be run in a separate thread. To prevent collisions
  use get_lock() method. Method `draw` should be locked by multy-thread
  caller, get_cnv, get_opt and other functions which modify data
  should be locked in GObj implementations.
- Usually, `draw` method does not modify data, and read-only access
  from other functions can be done without locking.
  If `draw` method modifies data, read-only functions should be locked as well.
- `stop_drawing` flag is set when current drawing should be aborted.
  Method `draw` can check this flag and stop drawing as soon as possible.
  If one wants to modify data and update everything, following steps are
  needed:
  - run stop_drawing(true)
  - get lock object with `get_lock`
  - modify data
  - run stop_drawing(false)
  - emit signal_redraw_me()
  for set_cnv/set_opt methods this should be done by caller.
*/
class GObj{
protected:

public:

  enum ret_t{
    FILL_NONE = 0, // object draws nothing
    FILL_PART = 1, // object draws some points
    FILL_ALL  = 2  // object fills in the whole image with opaque colors
  };

  GObj(): stop_drawing_flag(false) { }

  // Called by viewer before drawing the screen.
  // draw_range is the whole area, not tiles.
  virtual void prepare_range(const dRect & draw_range) {}

  /** Draw with CairoWrapper.
   \return one of:
   - GObj::FILL_NONE  -- nothing has been drawn
   - GObj::FILL_PART  -- something has been drawn
   - GObj::FILL_ALL   -- all image has been covered with a non-dransparent drawing
   NOTE:
  */
  virtual ret_t draw(const CairoWrapper & cr, const dRect & draw_range) = 0;

  // Return bounding box in object coordinates (empty if not specified)
  virtual dRect bbox() const {return dRect();}

  // signal_redraw_me should be emitted when data was changed and the
  // object has to be redrawn. Normally it is attached to Viewer::redraw
  // method.
  sigc::signal<void, iRect> & signal_redraw_me() {
    return signal_redraw_me_;
  }

  /// emit signal_redraw_me
  virtual void redraw_me() {signal_redraw_me_.emit(iRect());}

private:
  sigc::signal<void, iRect> signal_redraw_me_;

  // Mutex for locking multi-thread operations.
  std::mutex draw_mutex;

public:

  /********************************************************/

  // Functions for setting coordinate conversion and options.
  // Should be redefined in the GObj implementation.

  // change coordinate transformation
  virtual void set_cnv(const std::shared_ptr<ConvBase> c) {};

  // change options
  virtual void set_opt(const Opt & o) {};

  // Get default options. In the case when the object is controlled
  // by GUI it could be useful to get default state of the interface.
  static Opt get_def_opt() {return Opt();}

  /********************************************************/

  // If GObj is used from a DThreadViewer then the draw() method
  // is called from a sepereate thread. In this case all modifications
  // of data used in draw() should be locked.
  // - Caller (DThreadViewer, GobjMulti) locks draw(), set_cnv(), set_opt()
  // - Gobj locks rescale() and set_cnv() operations.
  // - everything else sould be locked inside the object implementation.
  //
  // Method get_lock() returns the lock object.
  std::unique_lock<std::mutex> get_lock() {
    return std::unique_lock<std::mutex>(draw_mutex);
  }

  // stop_drawing flag shows that drawing should be stopped as soon as
  // possible. We set it before doing get_lock() when we want to do
  // any change in sub-objects.
  bool stop_drawing_flag;

  void stop_drawing(bool state=true){ stop_drawing_flag = state; }
  bool is_stopped() const {return stop_drawing_flag;}
};

#endif
