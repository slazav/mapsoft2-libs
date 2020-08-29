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

        auto box = get_bbox();
        int x1 = floor((r.x - box.x) / (double)box.w);
        int x2 =  ceil((r.x + r.w - box.x) / (double)box.w) + 1;
        for (int x = x1; x<x2; x++) {
          // if xloop = false we draw only x=0
          if (!get_xloop() && x!=0) continue;


          auto r1 = r; r1.x -= x*box.w;
          r1.intersect(box);
          if (r1.is_zsize()) continue;

          crw->save();
          crw->translate(-r1.tlc());
          try {
            auto lk = obj->get_lock();
            obj->draw(crw, r1);
          }
          catch (Err & e){ std::cerr << "Viewer warning: " << e.str() << "\n"; }
          crw->restore();
        }
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

  // Background painting.
  // Context should be saved/restored to keep original
  // clipping range and clipped additionaly to the range extents.
  crw->save();
  crw->rectangle(r);
  crw->clip(); // set clip region
  crw->set_color(get_bgcolor());
  crw->paint();
  crw->restore();

  // It could be that viewer.bbox does not cover the whole
  // range. It xloop is set we want to draw a few pictures.
  // Calculate shifts:
  auto box = get_bbox();
  auto org = get_origin();
  int x1 = floor((org.x + r.x - box.x) / (double)box.w);
  int x2 =  ceil((org.x + r.x + r.w - box.x) / (double)box.w) + 1;
  for (int x = x1; x<x2; x++) {
    // if xloop = false we draw only x=0
    if (!get_xloop() && x!=0) continue;

    // clip to viewer.bbox
    iRect draw_range = r + org; draw_range.x -= x*box.w;
    draw_range.intersect(box);
    if (draw_range.is_zsize()) continue;

    if (obj)
      obj->prepare_range(draw_range);
  }

  updater_mutex->lock();
  if (tiles_todo.empty()) signal_busy().emit();
  updater_mutex->unlock();

  iRect tiles = ceil(dRect(r + org)/(double)TILE_SIZE);
  iPoint key;

  // note: updater extracts tiles from todo set sorted by x,y
  for (key.x = tiles.x; key.x<tiles.x+tiles.w; key.x++){
    for (key.y = tiles.y; key.y<tiles.y+tiles.h; key.y++){

      // region to paint in widget coordinates
      iRect rect = tile_to_rect(key) - org;

      // draw the tile from cache
      if (tiles_cache.count(key)>0){
        crw->set_source(
          tiles_cache.find(key)->second.get_surface(),
          rect.x, rect.y);
        crw->paint();
      }

      // no tile in cache
      else if (tiles_todo.count(key)==0){
        updater_mutex->lock();
        tiles_todo.insert(key);
        updater_cond->signal();
        updater_mutex->unlock();
      }
    }
  }

  updater_mutex->lock();
  if (tiles_todo.empty()) signal_idle().emit();
  updater_mutex->unlock();

}

const int DThreadViewer::TILE_MARG;

