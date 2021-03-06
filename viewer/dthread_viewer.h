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

    // override redraw with locking
    void redraw (const iRect & range = iRect()) override;

    // override set_cnv with locking
    virtual void set_cnv(std::shared_ptr<ConvBase> c, bool fix_range) override;

    using SimpleViewer::rescale;
    virtual void rescale(const double k, const iPoint & cnt) override;

    // override set_cnv with locking
    virtual void set_opt(const Opt & o) override;

  private:
    // Rectangle of cached tiles if larger then that of visible tiles by
    // this value:
    const static int TILE_MARG=2;

    // CairoContext keeps image surface for fast access
    // for redrawing and Image for keeping actual data
    std::map<iPoint, CairoWrapper> tiles_cache;

    std::set<iPoint>       tiles_todo;
    std::queue<iPoint>     tiles_done;

    Glib::Thread           *updater_thread;
    Glib::Mutex            *updater_mutex;
    Glib::Cond             *updater_cond;
    Glib::Dispatcher        done_signal;

    bool updater_needed;    // to stop updater on exit
};

#endif
