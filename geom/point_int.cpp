#include "point_int.h"
#include <cmath>

iPoint
adjacent(const iPoint &p, int dir){
  if (dir<0) dir = abs(7-dir);
  switch(dir%8){
    case 0: return iPoint(p.x-1,p.y-1);
    case 1: return iPoint(p.x,  p.y-1);
    case 2: return iPoint(p.x+1,p.y-1);
    case 3: return iPoint(p.x+1,p.y  );
    case 4: return iPoint(p.x+1,p.y+1);
    case 5: return iPoint(p.x,  p.y+1);
    case 6: return iPoint(p.x-1,p.y+1);
    case 7: return iPoint(p.x-1,p.y  );
    default: return p;
  }
}

int
is_adjacent(const iPoint & p1, const iPoint & p2){
  for (int i = 0; i<8; i++){
    if (adjacent(p1, i) == p2) return i;
  }
  return -1;
}


std::set<iPoint>
border_set(const std::set<iPoint >& pset){
  std::set<iPoint> ret;
  std::set<iPoint>::const_iterator it;
  for (it = pset.begin(); it != pset.end(); it++){
    for (int i=0; i<8; i++){
      iPoint p=adjacent(*it, i);
      if (pset.count(p)==0) ret.insert(p);
    }
  }
  return ret;
}


bool
add_set_and_border(const iPoint& p, std::set<iPoint>& pset, std::set<iPoint>& bord){
  if (pset.count(p)) return false; // point already there
  pset.insert(p);
  bord.erase(p);
  for (int i=0; i<8; i++){
    iPoint p2 = adjacent(p, i);
    if (pset.count(p2)==0) bord.insert(p2);
  }
  return true;
}

iPoint
my_crn (int k){
  k%=4;
  return iPoint(k/2, (k%3>0)?1:0);
}

iMultiLine
border_line(const std::set<iPoint>& pset){
  iMultiLine ret;
  // add loops around each point:
  for (const auto & p:pset){
    for (int k = 0; k<4; k++){
      iLine side; // always 2 points
      side.push_back(p+my_crn(k));
      side.push_back(p+my_crn(k+1));
      // Here we will have many line pairs drawn in opposite directions.
      // merge() will remove them, but it's a long process.
      // Remove them:
      bool ispair=false;
      for (auto i = ret.begin(); i!=ret.end(); ++i){
        if (side[0] == (*i)[1] && side[1] == (*i)[0]){
           ispair = true;
           ret.erase(i);
           break;
        }
      }
      if (!ispair) ret.push_back(side);
    }
  }

  // merge lines
  for (auto i1 = ret.begin(); i1!=ret.end(); i1++){
    for (auto i2 = i1+1; i2!=ret.end(); i2++){
      iLine tmp;
      if (*(i1->begin())  == *(i2->begin()))  {
        tmp.insert(tmp.end(), i1->rbegin(), i1->rend());
        tmp.insert(tmp.end(), i2->begin()+1, i2->end());
      }
      else if (*(i1->begin())  == *(i2->rbegin())) {
        tmp.insert(tmp.end(), i1->rbegin(), i1->rend());
        tmp.insert(tmp.end(), i2->rbegin()+1, i2->rend());
      }
      else if (*(i1->rbegin()) == *(i2->begin())) {
        tmp.insert(tmp.end(), i1->begin(), i1->end());
        tmp.insert(tmp.end(), i2->begin()+1, i2->end());
      }
      else if (*(i1->rbegin()) == *(i2->rbegin())) {
        tmp.insert(tmp.end(), i1->begin(), i1->end());
        tmp.insert(tmp.end(), i2->rbegin()+1, i2->rend());
      }
      else continue;
      i1->swap(tmp);
      ret.erase(i2);
      i2=i1;
    }
  }

  // remove points located on stright lines
  for (auto & l:ret){
    for (auto i1 = l.begin(); i1!=l.end(); i1++){
      auto i2 = i1+1;
      auto i3 = i2+1;
      if (i2 == l.end() || i3 == l.end()) break;
      if ((i2->x == i1->x && i2->x == i3->x) ||
          (i2->y == i1->y && i2->y == i3->y)) {
        l.erase(i2);
      }
    }
  }
  return ret;
}

// see details in https://github.com/slazav/bresenham/blob/master/br.c
std::set<iPoint>
bres_line(iPoint p1, iPoint p2, const int w, const int sh){

  std::set<iPoint> ret;
  int dx=p2.x-p1.x, dy=p2.y-p1.y;
  int e,j;
  int sx=dx>0;  // line goes right
  int sy=dy>0;  // line goes up

  if (!sx) dx=-dx;
  if (!sy) dy=-dy;

  int s=dx>dy; // line closer to horizontal
  bool sd = (sx&&s) || (!sy&&!s); // negative shift

  // start/stop width
  int w1=0-w + (sd?-sh:sh);
  int w2=1+w + (sd?-sh:sh);

  if (s){
    for (j=w1;j<w2;j++) ret.insert(p1 + iPoint(0,j));
    e = (dy<<1)-dx;
    while (p1.x!=p2.x){
      if (e<0){
        sx?p1.x++:p1.x--; e+=dy<<1;
        for (j=w1;j<w2;j++) ret.insert(p1 + iPoint(0,j));
      }
      else {
        sy?p1.y++:p1.y--; e-=dx<<1;
      }
    }
  }
  else {
    for (j=w1;j<w2;j++) ret.insert(p1 + iPoint(j,0));
    e = (dx<<1)-dy;
    while (p1.y!=p2.y){
      if (e<0){
        sy?p1.y++:p1.y--; e+=dx<<1;
        for (j=w1;j<w2;j++) ret.insert(p1 + iPoint(j,0));
      }
      else {
        sx?p1.x++:p1.x--; e-=dy<<1;
      }
    }
  }
  return ret;
}
