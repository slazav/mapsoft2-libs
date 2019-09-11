#ifndef DTHREAD_VIEWER
#define DTHREAD_VIEWER

#include "simple_viewer.h"
#include <map>
#include <set>
#include <queue>

///\addtogroup gred
///@{
///\defgroup dthread_viewer
///Double-threaded viewer with square tiles.
///@{
class DThreadViewer : public SimpleViewer {
  public:

    DThreadViewer(GObj * pl);
    ~DThreadViewer();

    iRect tile_to_rect(const iPoint & key) const;
    void updater();
    void on_done_signal();
    void draw(const CairoWrapper & crw, const iRect & r);

    void redraw (void);
    void rescale(const double k, const iPoint & cnt);
    void rescale(const double k){
      rescale(k,iPoint(get_width(), get_height())/2);
    }

  private:
    // Rectangle of cached tiles if larger then that of visible tiles by
    // this value:
    const static int TILE_MARG=2;

    // We want to have surface cache for fast access from Cairo
    // (we use it to redraw window after each rubber change).
    // But we keep actual data in Image object.
    // Here two parallel caches are used. Not very nice solution.
    std::map<iPoint, Cairo::RefPtr<Cairo::ImageSurface> > surf_cache;
    std::map<iPoint, Image> tiles_cache; // it keeps actual data

    std::set<iPoint>       tiles_todo;
    std::queue<iPoint>     tiles_done;

    Glib::Thread           *updater_thread;
    Glib::Mutex            *updater_mutex;
    Glib::Mutex            *draw_mutex;
    Glib::Cond             *updater_cond;
    Glib::Dispatcher        done_signal;

    bool updater_needed; // to stop updater on exit
    bool stop_drawing; // to avoid caching of old tiles
};

#endif
