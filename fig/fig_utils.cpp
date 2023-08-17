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
fig_scale(list<FigObj> & objects, const double scale, const iPoint & p0){
  for (auto & o:objects){
    o = (o-p0)*scale + p0;
    // todo: circles, text, line widths...
  }
}

void
fig_shift(list<FigObj> & objects, const iPoint & sh){
  for (auto & o:objects){
    o += sh;
  }
}


void
fig_remove_empty_comp(std::list<FigObj> & objects){

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

void
fig_remove_comp(std::list<FigObj> & objects){
  auto i = objects.begin();
  while (i!=objects.end()){
    if (i->is_compound() || i->is_compound_end())
      i=objects.erase(i);
    else ++i;
  }
}

bool
fig_match_template(const FigObj & o, const std::string & tmpl){
  FigObj t = figobj_template(tmpl); // make template object
  int c1 = o.pen_color;
  int c2 = t.pen_color;

  // compounds are always equal:
  if (o.is_compound() && t.is_compound()) return true;
  if (o.is_compound_end() && t.is_compound_end()) return true;

  // text and compound is not equal to any other types
  if (o.is_text() && !t.is_text()) return false;
  if (t.is_text() && !o.is_text()) return false;
  if (o.is_compound() && !t.is_compound()) return false;
  if (t.is_compound() && !o.is_compound()) return false;
  if (o.is_compound_end() && !t.is_compound_end()) return false;
  if (t.is_compound_end() && !o.is_compound_end()) return false;

  // lines
  if ((o.is_polyline() || o.is_spline()) && (o.size()>1)){
    // depth and thickness should match:
    if ((o.depth != t.depth) ||
        (o.thickness != t.thickness)) return false;
    // if thickness > 0 color and line style should match:
    if (o.thickness!=0 &&
        (c1!=c2 || o.line_style!=t.line_style)) return false;

    // fill
    int af1 = o.area_fill;
    int af2 = t.area_fill;
    int fc1 = o.fill_color;
    int fc2 = t.fill_color;
    // there are two types of white fill
    if ((fc1!=0xffffff)&&(af1==40)) {fc1=0xffffff; af1=20;}
    if ((fc2!=0xffffff)&&(af2==40)) {fc2=0xffffff; af2=20;}

    // fill type should match:
    if (af1 != af2) return false;
    // if fill is not transparent, fill color should match:
    if (af1!=-1 && fc1!=fc2) return false;

    // for hatch filling pen_color should match (even if thickness=0)
    if (af1>41 && c1!=c2) return false;

    // After all tests we assume that object has this type!
    return true;
  }

  // points
  if ((o.is_polyline() || o.is_spline()) && o.size()==1){
    //depth, thickness, color, cap_style should match:
    if (o.depth != t.depth || o.thickness != t.thickness ||
        c1 != c2 || (o.cap_style%2)!=(t.cap_style%2)) return false;
    return true;
  }

  // text
  if (o.is_text()){
    // depth, color, font should match:
    if (o.depth!=t.depth || c1!=c2 || o.font!=t.font) return false;
    return true;
  }

  return false;
}

bool
fig_match_templates(const std::string & tmpl1, const std::string & tmpl2){
  return fig_match_template(figobj_template(tmpl1), tmpl2);
}
