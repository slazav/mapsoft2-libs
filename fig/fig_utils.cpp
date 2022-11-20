#include "fig_utils.h"
#include <stack>

using namespace std;

iRect
fig_bbox(const list<FigObj> & objects){
  iRect ret;
  for (const auto & o:objects) ret.expand(o.bbox());
  return ret;
}

void
fig_make_comp(list<FigObj> & objects){
  iRect r = fig_bbox(objects);
  if (r.is_empty()) return;

  FigObj o;
  o.type = -6;
  objects.insert(objects.end(), o);
  o.type=6;
  o.push_back(r.tlc());
  o.push_back(r.brc());
  objects.insert(objects.begin(), o);
}

void
fig_rotate(list<FigObj> & objects, const double a, const iPoint & p0){
  for (auto & o:objects){
    o.rotate2d(p0,a);
    if ((o.type == 4)||(o.type==1)) {
      o.angle += a;
      while (o.angle>M_PI) o.angle-=2*M_PI;
    }
    // should we rotate center_x,y, begin_x,y, end_x,y too?
  }
}

void
fig_shift(list<FigObj> & objects, const iPoint & sh){
  for (auto & o:objects){
    o += sh;
  }
}


void fig_remove_empty_comp(std::list<FigObj> & objects){

  // links to compound start objects:
  std::stack<std::list<FigObj>::iterator> st;
  // number of non-compound objects in compounds
  std::stack<size_t> nn;

  auto i = objects.begin();
  while (i!=objects.end()){
    if (i->is_compound()) {
      st.push(i);
      nn.push(0);
      ++i; continue;
    }

    if (i->is_compound_end()){
      if (st.empty() || nn.empty()){ ++i; continue; }
      auto i0 = st.top();
      auto n0 = nn.top();
      st.pop(); nn.pop();
      ++i;
      if (n0==0) i=objects.erase(i0,i);
      if (!nn.empty()) nn.top()+=n0;
      continue;
    }
    if (!nn.empty()) nn.top()+=1;
    ++i; continue;
  }
}

