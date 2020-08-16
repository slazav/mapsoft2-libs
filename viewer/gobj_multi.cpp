#include "gobj_multi.h"

#include <map>
#include <vector>
#include <memory>
#include "gobj.h"


std::multimap<int, GObjMulti::GObjData>::iterator
GObjMulti::find(const std::shared_ptr<GObj> & o) {
  for (auto i = data.begin(); i!=data.end(); ++i)
    if (i->second.obj == o) return i;
  return data.end();
}

std::multimap<int, GObjMulti::GObjData>::const_iterator
GObjMulti::find(const std::shared_ptr<GObj> & o) const {
  for (auto i = data.begin(); i!=data.end(); ++i)
    if (i->second.obj == o) return i;
  return data.end();
}

void
GObjMulti::process_error(std::exception & e){
  switch (error_policy){
    case GOBJ_MULTI_ERR_IGN:  return;
    case GOBJ_MULTI_ERR_WARN: std::cerr << e.what() << "\n"; break;
    case GOBJ_MULTI_ERR_EXC:  throw;
  }
}


/************************************************/
// Public methods

void
GObjMulti::add(int depth, std::shared_ptr<GObj> o){
  if (!o) return;
  o->set_opt(opt);
  o->set_cnv(cnv);

  stop_drawing(true);
  auto lock = get_lock();

  GObjData D;
  D.obj = o;
  D.on = true;
  auto & sig = signal_redraw_me();
  D.redraw_conn = o->signal_redraw_me().connect(
    sigc::mem_fun (&sig, &sigc::signal<void, iRect>::emit));
  data.emplace(-depth, D); // we use negative depth for correct sorting

  stop_drawing(false);
  signal_redraw_me().emit(iRect());
}

std::vector<std::shared_ptr<GObj> >
GObjMulti::get_data() const {
  std::vector<std::shared_ptr<GObj> > ret;
  for (auto const & d:data)
    ret.push_back(d.second.obj);
  return ret;
}

int
GObjMulti::get_depth(std::shared_ptr<GObj> o) const{
  auto it = find(o);
  if (it==data.end())
    throw Err() << "GObjMulti::get_depth: no such object";
  return -it->first;
}

bool
GObjMulti::get_visibility(std::shared_ptr<GObj> o) const{
  auto it = find(o);
  if (it==data.end())
    throw Err() << "GObjMulti::get_visibility: no such object";
  return it->second.on;
}

void
GObjMulti::set_depth(std::shared_ptr<GObj> o, int depth){
  if (!o) return;

  stop_drawing(true);
  auto lock = get_lock();

  auto it = find(o);
  if (it==data.end()) return;
  if (it->first == -depth) return;
  data.emplace(-depth, it->second);
  data.erase(it);

  stop_drawing(false);
  signal_redraw_me().emit(iRect());
}

void
GObjMulti::set_visibility(std::shared_ptr<GObj> o, bool on){
  if (!o) return;

  stop_drawing(true);
  auto lock = get_lock();

  auto it = find(o);
  if (it==data.end()) return;
  it->second.on = on;

  stop_drawing(false);
  signal_redraw_me().emit(iRect());
}

void
GObjMulti::del(std::shared_ptr<GObj> o){
  if (!o) return;

  stop_drawing(true);
  auto lock = get_lock();

  auto it = find(o);
  if (it==data.end()) return;
  it->second.redraw_conn.disconnect();
  data.erase(it);

  stop_drawing(false);
  signal_redraw_me().emit(iRect());
}

void
GObjMulti::clear(){
  stop_drawing(true);
  auto lock = get_lock();

  for (auto & o:data) o.second.redraw_conn.disconnect();
  data.clear();

  stop_drawing(false);
  signal_redraw_me().emit(iRect());
}

/************************************************/
// override default GObj methods

int
GObjMulti::draw(const CairoWrapper & cr, const dRect & draw_range){
  int res = GObj::FILL_NONE;
  for (auto const & p:data){
    if (!p.second.on) continue;
    if (is_stopped()) return GObj::FILL_NONE;
    cr->save();
    auto o = p.second.obj;
    auto lk = o->get_lock();
    try {
      int res1 = o->draw(cr, draw_range);
      if (res1 != GObj::FILL_NONE &&
          res!=GObj::FILL_ALL) res=res1;
    }
    catch (std::exception & e) { process_error(e); }
    cr->restore();
  }
  return res;
}

void
GObjMulti::prepare_range(const dRect & range){
  for (auto const & p:data){
    if (!p.second.on) continue;
    auto o = p.second.obj;
    auto lk = o->get_lock();
    try { o->prepare_range(range); }
    catch (std::exception & e) { process_error(e); }
  }
}

void
GObjMulti::set_cnv(std::shared_ptr<ConvBase> c) {
  cnv = c;
  for (auto const & p:data){
    auto o = p.second.obj;
    o->stop_drawing(true);
    auto lk = o->get_lock();
    try { o->set_cnv(cnv); }
    catch (std::exception & e) { process_error(e); }
    o->stop_drawing(false);
  }
  signal_redraw_me().emit(iRect());
}

void
GObjMulti::set_opt(const Opt & o) {
  opt = o;

  // option "error_policy"
  if (o.get("error_policy") == "ignore")
    error_policy = GOBJ_MULTI_ERR_IGN;
  else if (o.get("error_policy") == "warning")
    error_policy = GOBJ_MULTI_ERR_WARN;
  else if (o.get("error_policy") == "exception")
    error_policy = GOBJ_MULTI_ERR_EXC;

  // send options to children
  for (auto const & p:data){
    auto o = p.second.obj;
    o->stop_drawing(true);
    auto lk = o->get_lock();
    try { o->set_opt(opt); }
    catch (std::exception & e) { process_error(e); }
    o->stop_drawing(false);
  }
  signal_redraw_me().emit(iRect());
}

