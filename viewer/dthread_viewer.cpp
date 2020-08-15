#include "dthread_viewer.h"
#include "cairo/cairo_wrapper.h"

#define TILE_SIZE (256)

DThreadViewer::DThreadViewer(GObj * pl) :
    SimpleViewer(pl),
    updater_needed(true) {

  if (!Glib::thread_supported()) Glib::thread_init();
  done_signal.connect(sigc::mem_fun(*this, &DThreadViewer::on_done_signal));

  updater_mutex = new(Glib::Mutex);
  updater_cond = new(Glib::Cond);
  updater_thread =
    Glib::Thread::create(sigc::mem_fun(*this, &DThreadViewer::updater), true);

}

DThreadViewer::~DThreadViewer(){
  updater_mutex->lock();
  updater_needed = false;
  updater_cond->signal();
  updater_mutex->unlock();
  updater_thread->join(); // waiting for our thread to exit
  delete(updater_mutex);
  delete(updater_cond);
}

void
DThreadViewer::redraw(const iRect & range){
  obj->stop_drawing(true);
  updater_mutex->lock();
  tiles_cache.clear();
  updater_mutex->unlock();
  obj->stop_drawing(false);
  SimpleViewer::redraw(range);
}

void
DThreadViewer::set_cnv(std::shared_ptr<ConvBase> c, bool fix_range){
  obj->stop_drawing(true);
  dRect r = get_range(true);
  {
    auto lk = obj->get_lock();
    SimpleViewer::set_cnv(c, false);
  }
  // note: set_range -> rescale -> set_cnv with its own locking
  if (fix_range) set_range(r, true);
  obj->stop_drawing(false);
}

void
DThreadViewer::rescale(const double k, const iPoint & cnt){
  obj->stop_drawing(true);
  auto lk = obj->get_lock();
  SimpleViewer::rescale(k,cnt);
  obj->stop_drawing(false);
}

void
DThreadViewer::set_opt(const Opt & o){
  obj->stop_drawing(true);
  auto lk = obj->get_lock();
  SimpleViewer::set_opt(o);
  obj->stop_drawing(false);
}


iRect
DThreadViewer::tile_to_rect(const iPoint & key) const{
  return iRect(key, key + iPoint(1,1))*TILE_SIZE;
}

void
DThreadViewer::updater(){
  do {

    // generate tiles
    updater_mutex->lock();
    if (!tiles_todo.empty()){

      iPoint key = *tiles_todo.begin();

      obj->stop_drawing(false);
      updater_mutex->unlock();

      CairoWrapper crw;
      crw.set_surface_img(TILE_SIZE, TILE_SIZE);
      crw->set_color(get_bgcolor());
      crw->paint();

      if (obj){
        dRect r = tile_to_rect(key);
        auto lk = obj->get_lock();
        crw->save();
        crw->translate(-r.tlc());
        try { obj->draw(crw, r); }
        catch (Err & e){ std::cerr << "Viewer warning: " << e.str() << "\n"; }
        crw->restore();
      }
      crw.get_surface()->flush();

      updater_mutex->lock();
      if (!obj->is_stopped()){
        if (tiles_cache.count(key)>0) tiles_cache.erase(key);
        tiles_cache.insert(std::make_pair(key, crw));
        tiles_done.push(key);
        tiles_todo.erase(key);
        done_signal.emit();
      }
    }
    updater_mutex->unlock();

    // cleanup queue
    iRect scr = iRect(get_origin().x, get_origin().y,  get_width(), get_height());
    iRect tiles_to_keep = ceil(dRect(scr)/(double)TILE_SIZE);

    updater_mutex->lock();
    std::set<iPoint>::iterator qit=tiles_todo.begin(), qit1;
    while (qit!=tiles_todo.end()) {
      if (tiles_to_keep.contains(*qit)) qit++;
      else {
        qit1=qit; qit1++;
        tiles_todo.erase(qit);
        qit=qit1;
      }
    }
    updater_mutex->unlock();

    // cleanup caches
    tiles_to_keep = expand(tiles_to_keep, TILE_MARG);
    updater_mutex->lock();
    auto it=tiles_cache.begin();
    while (it!=tiles_cache.end()) {
      if (tiles_to_keep.contains(it->first)) it++;
      else it = tiles_cache.erase(it);
    }
    updater_mutex->unlock();

    updater_mutex->lock();
    if (tiles_todo.empty()) updater_cond->wait(*updater_mutex);
    updater_mutex->unlock();
  }
  while (updater_needed);
}


void DThreadViewer::on_done_signal(){

  while (!tiles_done.empty()){
    iPoint key=tiles_done.front();

    if (tiles_cache.count(key)){
      iPoint pt = key*TILE_SIZE-get_origin();
      queue_draw_area(pt.x,pt.y, TILE_SIZE, TILE_SIZE);
    }

    updater_mutex->lock();
    tiles_done.pop();
    updater_mutex->unlock();
  }
  if (tiles_todo.empty()) signal_idle().emit();
}


void DThreadViewer::draw(const CairoWrapper & crw, const iRect & r){

  if (!r) {redraw(); return;}
  iRect tiles = ceil(dRect(r + get_origin())/(double)TILE_SIZE);
  iPoint key;


  updater_mutex->lock();
  if (tiles_todo.empty()) signal_busy().emit();
  updater_mutex->unlock();

  if (obj) {
    auto lk = obj->get_lock();
    obj->prepare_range(r+get_origin());
  }

  // note: updater extracts tiles from todo set sorted by x,y
  for (key.x = tiles.x; key.x<tiles.x+tiles.w; key.x++){
    for (key.y = tiles.y; key.y<tiles.y+tiles.h; key.y++){

      // region to paint in widget coordinates
      iRect rect = tile_to_rect(key) - get_origin();

      // draw the tile from cache
      if (tiles_cache.count(key)>0){
        crw->set_source(
          tiles_cache.find(key)->second.get_surface(),
          rect.x, rect.y);
        crw->paint();
      }

      else { // no tile in cache
        // Background painting.
        // Context should be saved/restored to keep original
        // clipping range and clipped additionaly to the tile extents.
        crw->save();
        crw->rectangle(rect);
        crw->clip(); // set clip region
        crw->set_color(get_bgcolor());
        crw->paint();
        crw->restore();

        // Put this tile in todo queue.
        if (tiles_todo.count(key)==0){
          updater_mutex->lock();
          tiles_todo.insert(key);
          updater_cond->signal();
          updater_mutex->unlock();
        }
      }
    }
  }

  updater_mutex->lock();
  if (tiles_todo.empty()) signal_idle().emit();
  updater_mutex->unlock();
}

const int DThreadViewer::TILE_MARG;

