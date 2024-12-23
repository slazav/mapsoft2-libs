#include "geo_nom_fi.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

// see https://www.maanmittauslaitos.fi/kartat-ja-paikkatieto/kartat/osta-kartta/nain-loydat-oikean-karttalehden

// build range for a single sheet, like FI_V51
dRect nom_to_range_fi(const string & name){
  int W = 192; // km
  int H = 96;  // km
  dRect ret(0,0,192,96);

  istringstream f(name);

  try {

    // Read letter K..X without O
    char c = tolower(f.get());
    if (c<'k' || c>'x' || c=='o') throw Err() << "letter k..n or p..x expected";
    c= (c>'o')? c-'k'-1 : c-'k';
    ret.y = (int)c*ret.h + 6570; // y coordinate, m

    // Read digit 2..6
    c = f.get();
    if (c<'2' || c>'6') throw Err() << "first digit 2..6 expected";
    ret.x = (int)(c-'5')*ret.w + 500; // x coordinate, km

    // return 1:200'000, 96x192 km map
    if (f.peek() == -1) return ret*1000.0;


    // Read digit 1..4
    c = f.get();
    if (c<'1' || c>'4') throw Err() << "second digit 1..4 expected";
    ret.w/=2;  ret.h/=2;
    if (c=='3' || c=='4') ret.x+=ret.w;
    if (c=='2' || c=='4') ret.y+=ret.h;

    // return 1:100'000, 48x96 km map
    if (f.peek() == -1) return ret*1000.0;


    // Read digit 1..4
    c = f.get();
    if (c<'1' || c>'4') throw Err() << "third digit 1..4 expected";
    ret.w/=2;  ret.h/=2;
    if (c=='3' || c=='4') ret.x+=ret.w;
    if (c=='2' || c=='4') ret.y+=ret.h;

    // return 1:50'000, 24x48 km map
    if (f.peek() == -1) return ret*1000.0;


    // Read digit 1..4
    c = f.get();
    if (c<'1' || c>'4') throw Err() << "forth digit 1..4 expected";
    ret.w/=2;  ret.h/=2;
    if (c=='3' || c=='4') ret.x+=ret.w;
    if (c=='2' || c=='4') ret.y+=ret.h;

    // return 1:25'000, 12x24 km map
    if (f.peek() == -1) return ret*1000.0;


    // Read a letter. Two options:
    // - A..H for a classical 1:10'000, 6x6 km map
    // - L or R for 12x12km map (as in MML datasets)
    c = tolower(f.get());
    if (c=='r' || c=='l'){
      ret.w/=2;
      if (c=='r') ret.x+=ret.w;
      if (f.peek() == -1) return ret*1000.0;
      throw Err() << "extra symbols after the name";
    }
    if (c<'a' || c>'h') throw Err()
      << "letter A..H (or R, or L) expected";
    ret.w/=4;  ret.h/=2;
    c-='a';
    ret.x += ret.w * (c/2);
    if (c%2==1) ret.y+=ret.h;

    // return 1:10'000, 6x6 km map
    if (f.peek() == -1) return ret*1000.0;


    // Read digit 1..4
    c = f.get();
    if (c<'1' || c>'4') throw Err() << "digit 1..4 expected";
    ret.w/=2;  ret.h/=2;
    if (c=='3' || c=='4') ret.x+=ret.w;
    if (c=='2' || c=='4') ret.y+=ret.h;

    // return 1:10'000, 3x3 km map
    if (f.peek() == -1) return ret*1000.0;

    throw Err() << "extra symbols after the name";

    }
  catch (const Err & e){
    throw Err() << "nom_to_range_fi: can't parse name: \""
      << name << "\": " << e.str();
  }
}


string
pt_to_nom_fi(dPoint p, nom_scale_fi_t sc){

  // 1:200'000 range
  p = (p/1000 - dPoint(500,6570))/dPoint(192,96) + dPoint(5,0);

  std::ostringstream str;
  const char *ch = "KLMNPQRSTUVWX";
  iPoint p1 = floor(p);
  if (p1.x<2 || p1.x>6 || p1.y<0 || p1.y>13)
    throw Err() << "coordinate out of range";
  str << ch[p1.y] << p1.x;
  if (sc == SC_FI_200k) return str.str();

  p = (p - floor(p))*2;
  int D = 1;
  if (p.x > 1) D+=2;
  if (p.y > 1) D+=1;
  str << D;
  if (sc == SC_FI_100k) return str.str();

  p = (p - floor(p))*2;
  D = 1;
  if (p.x > 1) D+=2;
  if (p.y > 1) D+=1;
  str << D;
  if (sc == SC_FI_50k) return str.str();

  p = (p - floor(p))*2;
  D = 1;
  if (p.x > 1) D+=2;
  if (p.y > 1) D+=1;
  str << D;
  if (sc == SC_FI_25k) return str.str();

  const char *ch2 = "ABCDEFGH";
  p = (p - floor(p))*dPoint(4,2);
  D = 2*(int)floor(p.x) + (int)floor(p.y);
  str << ch2[D];
  if (sc == SC_FI_10k) return str.str();

  p = (p - floor(p))*2;
  D = 1;
  if (p.x > 1) D+=2;
  if (p.y > 1) D+=1;
  str << D;
  if (sc == SC_FI_5k) return str.str();

  throw Err() << "unknown scale: " << sc;
}
