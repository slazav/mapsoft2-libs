#include "line_rectcrop.h"

bool
rect_crop(const dRect & cutter, dLine & line, bool closed){

  double xl=cutter.x;
  double xh=cutter.x+cutter.w;
  double yl=cutter.y;
  double yh=cutter.y+cutter.h;

  if (line.size()<3) closed=false;

  bool res=false;

  // Consider stright cutter line along each side of the rectangle.
  for (int i=0; i<4; i++){

    // Move along the path.
    // If a point is outside the cutter line then replace it with
    // intersection points of adjecent segments with the cutter
    // line (if any) and exclude these points from next step (set skip=true).

    bool skip=false;
    auto p=line.begin();
    while (p!=line.end()){

      // Find previous and next points (or line.end())
      auto ppi = p, npi = p;
      if (skip) ppi=line.end();
      else {
        if (p==line.begin()){
          ppi = line.end();
          if (closed) ppi--;
        }
        else ppi--;
      }
      npi++;
      if ((npi==line.end()) && closed)
        npi=line.begin();

      bool pp_e = (ppi==line.end());
      bool np_e = (npi==line.end());
      dPoint pp,np;
      if (!pp_e) pp=*ppi;
      if (!np_e) np=*npi;

      // Each of four sides
      if ((i==0) && (p->x>xh)){
        if (!pp_e && (pp.x < xh)){
           p = line.insert(p, dPoint(xh, p->y -
            ((p->y - pp.y)*(p->x - xh))/(p->x - pp.x) ));
           p++;
        }
        if (!np_e && (np.x < xh)){
           p = line.insert(p, dPoint(xh, p->y -
            ((p->y - np.y)*(p->x - xh))/(p->x - np.x) ));
           p++;
        }
        p=line.erase(p);
        skip=true;
        res=true;
        continue;
      }

      if ((i==1) && (p->x<xl)){
        if (!pp_e && (pp.x > xl)){
           p = line.insert(p, dPoint(xl, p->y -
            ((p->y - pp.y)*(p->x - xl))/(p->x - pp.x) ));
           p++;
        }
        if (!np_e && (np.x > xl)){
           p = line.insert(p, dPoint(xl, p->y -
            ((p->y - np.y)*(p->x - xl))/(p->x - np.x) ));
           p++;
        }
        p=line.erase(p);
        skip=true;
        res=true;
        continue;
      }

      if ((i==2) && (p->y>yh)){
        if (!pp_e && (pp.y < yh)){
           p = line.insert(p, dPoint(p->x -
            ((p->x - pp.x)*(p->y - yh))/(p->y - pp.y), yh));
           p++;
        }
        if (!np_e && (np.y < yh)){
           p = line.insert(p, dPoint(p->x -
            ((p->x - np.x)*(p->y - yh))/(p->y - np.y), yh));
           p++;
        }
        p=line.erase(p);
        skip=true;
        res=true;
        continue;
      }

      if ((i==3) && (p->y<yl)){
        if (!pp_e && (pp.y > yl)){
           p = line.insert(p, dPoint(p->x -
            ((p->x - pp.x)*(p->y - yl))/(p->y - pp.y), yl));
           p++;
        }
        if (!np_e && (np.y > yl)){
           p = line.insert(p, dPoint(p->x -
            ((p->x - np.x)*(p->y - yl))/(p->y - np.y), yl));
           p++;
        }
        p=line.erase(p);
        skip=true;
        res=true;
        continue;
      }
      skip=false;
      p++;
    }
  }
  return res;
}


dMultiLine
rect_split_cropped(const dRect & cutter,
                   const dLine & cropped, bool closed){
  dMultiLine ret;
  dLine rl;

  // Skip empty lines
  if (cropped.size()==0)
    return ret;

  // Keep single points.
  if (cropped.size()==1){
    ret.push_back(cropped);
    return ret;
  }

  // Its important to have these variables. Comparing with
  // cutter.x+cutter.w directly can fail because of rounding.
  double xl=cutter.x;
  double xh=cutter.x+cutter.w;
  double yl=cutter.y;
  double yh=cutter.y+cutter.h;

  // Lines: we simply skip all segments on the cutter sides
  // and separate points.
  if (!closed){
    dLine::const_iterator p, n;
    for (p=cropped.begin(); p!=cropped.end(); p++){
      n = p+1;
      rl.push_back(*p);

      // segment on cutter side
      if ((n==cropped.end()) ||
          ((p->x == n->x) && (n->x == xl)) ||
          ((p->x == n->x) && (n->x == xh)) ||
          ((p->y == n->y) && (n->y == yl)) ||
          ((p->y == n->y) && (n->y == yh)) ){
        // single points:
        if (rl.size()>1) ret.push_back(rl);
        rl.clear();
      }
    }
  }

  // Polygons: find pairs of segments such as:
  // - both are on the same cutter side
  // - one segment is inside another
  // - segments have opposite directions
  // Make cut in this pair, repeat
  else {
    ret.push_back(cropped);
    bool repeat;
    do{
      repeat=false;
      dMultiLine::iterator l;
      for (l=ret.begin(); !repeat && (l!=ret.end()); l++){

        dLine::iterator p1a,p1b,p2a,p2b;
        for (p1a = l->begin();!repeat && (p1a!=l->end()) ; p1a++){
          p1b=p1a+1;
          if (p1b==l->end()) break;

          for (p2a = p1b; !repeat && (p2a!=l->end()); p2a++){
            p2b=p2a+1;
            if (p2b==l->end()) p2b=l->begin();

            // two segments on one side, one inside another, opposite directions
            if (( (((p1a->x == xl) && (p1b->x == xl) && (p2a->x == xl) && (p2b->x == xl)) ||
                   ((p1a->x == xh) && (p1b->x == xh) && (p2a->x == xh) && (p2b->x == xh)) ) &&
                  (((p1a->y <= p2b->y) && (p2b->y <= p2a->y) && (p2a->y <= p1b->y)) ||
                   ((p2a->y <= p1b->y) && (p1b->y <= p1a->y) && (p1a->y <= p2b->y)) ||
                   ((p1a->y >= p2b->y) && (p2b->y >= p2a->y) && (p2a->y >= p1b->y)) ||
                   ((p2a->y >= p1b->y) && (p1b->y >= p1a->y) && (p1a->y >= p2b->y)) )) ||
                ( (((p1a->y == yl) && (p1b->y == yl) && (p2a->y == yl) && (p2b->y == yl)) ||
                   ((p1a->y == yh) && (p1b->y == yh) && (p2a->y == yh) && (p2b->y == yh)) ) &&
                  (((p1a->x <= p2b->x) && (p2b->x <= p2a->x) && (p2a->x <= p1b->x)) ||
                   ((p2a->x <= p1b->x) && (p1b->x <= p1a->x) && (p1a->x <= p2b->x)) ||
                   ((p1a->x >= p2b->x) && (p2b->x >= p2a->x) && (p2a->x >= p1b->x)) ||
                   ((p2a->x >= p1b->x) && (p1b->x >= p1a->x) && (p1a->x >= p2b->x)) ))) {
                dLine l1,l2;
                l1.insert(l1.end(), l->begin(), p1a+1);
                if (p2b!=l->begin()) l1.insert(l1.end(), p2b, l->end());
                l2.insert(l2.end(), p1b, p2a+1);
                l->swap(l1);
                ret.push_back(l2);
                repeat=true;
            }
          }
        }
      }
    } while (repeat);
  }
  return ret;
}


dMultiLine
rect_crop_multi(const dRect & cutter, const dMultiLine & obj, bool closed){
  dMultiLine ret;

  // open lines
  if (!closed) {
    for (const auto & line:obj){
      dLine l = line;
      rect_crop(cutter, l, closed);
      auto pts = rect_split_cropped(cutter, l, closed);
      ret.insert(ret.end(), pts.begin(), pts.end());
    }
    return ret;
  }

  // First crop without splitting
  for (const auto & line:obj){
    dLine l = line;
    rect_crop(cutter, l, closed);
    ret.push_back(l);
  }

  if (ret.size()==0) return ret;

  // For closed line (polygons) we want to join some contours
  // (assuming EVEN-ODD rule).

  // Its important to have these variables. Comparing with
  // cutter.x+cutter.w directly can fail because of rounding.
  double xl=cutter.x;
  double xh=cutter.x+cutter.w;
  double yl=cutter.y;
  double yh=cutter.y+cutter.h;

  // Step1: find segment on a cutter side from one part, and
  // point inside this segment on another part; connect them

  // l1: each non-empty part
  for (auto l1=ret.begin(); l1!=ret.end(); ++l1){
    if (l1->size() == 0) continue;
    // p1a, p1b: each segment of part l1
    for (auto p1a=l1->begin(); p1a!=l1->end(); ++p1a){
      auto p1b = p1a+1;
      if (p1b == l1->end()) p1b=l1->begin();

      // continue if the segment is not on a cutter side:
      bool bxl = p1a->x == xl && p1b->x == xl;
      bool bxh = p1a->x == xh && p1b->x == xh;
      bool byl = p1a->y == yl && p1b->y == yl;
      bool byh = p1a->y == yh && p1b->y == yh;
      if (!bxl && !bxh && !byl && !byh) continue;

      // l2: another non-empty part:
      for (auto l2=ret.begin(); l2!=ret.end(); ++l2){
        if (l2->size()==0 || l2==l1) continue;

        // p2: each point of part l2
        for (auto p2=l2->begin(); p2!=l2->end(); ++p2){

          // continue if p2 is not inside p1a-p1b:
          double minx = std::min(p1a->x, p1b->x);
          double miny = std::min(p1a->y, p1b->y);
          double maxx = std::max(p1a->x, p1b->x);
          double maxy = std::max(p1a->y, p1b->y);
          if ((!bxl || p2->x != xl || p2->y < miny || p2->y > maxy) &&
              (!bxh || p2->x != xh || p2->y < miny || p2->y > maxy) &&
              (!byl || p2->y != yl || p2->x < minx || p2->x > maxx) &&
              (!byh || p2->y != yh || p2->x < minx || p2->x > maxx)) continue;

          // connect line parts
          dLine nl;
          nl.insert(nl.end(), p2, l2->end());
          nl.insert(nl.end(), l2->begin(), p2);
          nl.insert(nl.end(), *p2);
          if (nl.area()*l1->area()>0) nl.invert();

          // tricky thing, insert can invalidate all l1 iterators!
          size_t idx = p1a - l1->begin(); // index of p1a
          l1->insert(p1b, nl.begin(), nl.end());
          l2->clear();
          p1a = l1->begin() + idx;
          p1b = p1a+1;
          if (p1b == l1->end()) p1b=l1->begin();

          break;
        } // points of l2
      } // l2
    } // points of l1
  } // l1


  // step2: split lines
  for (auto l = ret.begin(); l != ret.end(); ++l){
    if (l->size() == 0) continue;
    auto pts = rect_split_cropped(cutter, *l, true);
    if (pts.size() < 2) continue;
    l->swap(pts[0]);
    size_t idx = pts.size() + l - ret.begin();
    ret.insert(l+1, pts.begin()+1, pts.end());
    l = ret.begin() + idx;
  }

  // step 3: remove extra points
  for (auto l1 = ret.begin(); l1!=ret.end(); ++l1){
    if (l1->size() == 0) continue;

    auto p1a = l1->begin();
    while (p1a!=l1->end()) {
      auto p1b = p1a+1;
      if (p1b == l1->end()) p1b = l1->begin();
      auto p1c = p1b+1;
      if (p1c == l1->end()) p1c = l1->begin();

      if ((p1a->x == xl && p1b->x == xl && p1c->x == xl) ||
          (p1a->x == xh && p1b->x == xh && p1c->x == xh) ||
          (p1a->y == yl && p1b->y == yl && p1c->y == yl) ||
          (p1a->y == yh && p1b->y == yh && p1c->y == yh))
        p1a=l1->erase(p1b)-1;
      else
        ++p1a;
    }
  }

  // step 4: remove empty segments
  auto l = ret.begin();
  while (l != ret.end()){
    if (l->size()==0) l = ret.erase(l);
    else ++l;
  }

  return ret;
}
