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

