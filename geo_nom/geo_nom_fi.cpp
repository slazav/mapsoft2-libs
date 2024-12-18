#include "geo_nom_fi.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

// build range for a single sheet, like FI_V51
dRect nom_to_range_fi(const string & name){
  int W = 192; // km
  int H = 96;  // km

  istringstream f(name);

  // Read letter K..X without O
  char c = tolower(f.get());
  if (c<'k' || c>'x' || c=='o') throw Err()
    << "nom_to_range_fi: can't parse name: \"" << name
    << "\": letter k..n or p..x expected";

  c= (c>'o')? c-'k'-1 : c-'k';

  int Y = (int)c*H + 6570; // y coordinate, m

  // Read digit 2..6
  c = f.get();
  if (c<'2' || c>'6') throw Err()
    << "nom_to_range_fi: can't parse name: \"" << name
    << "\": first digit 2..6 expected";

  int X = (int)(c-'5')*W + 500; // x coordinate, km

  // Read digit 1..4
  c = f.get();
  if (c<'1' || c>'4') throw Err()
    << "nom_to_range_fi: can't parse name: \"" << name
    << "\": second digit 1..4 expected";

  if (c=='3' || c=='4') X+=W/2;
  if (c=='2' || c=='4') Y+=H/2;

  if (!f.eof() && f.peek() != -1) throw Err()
    << "nom_to_range_fi: can't parse name: \"" << name
    << "\": extra symbols after the name";

  return 1e3*dRect(X,Y,W/2,H/2);
}


string
pt_to_nom_fi(dPoint p){
  int W = 192; // km
  int H = 96;  // km
  const char *ch = "klmnpqrstuvwx";

  int N1 = floor((p.x/1000 - 500)/W) + 5;
  int A1 = floor((p.y/1000 - 6570)/H);

  if (N1<2 || N1>6 || A1<0 || A1>13)
    throw Err() << "coordinate out of range";

  double dx = (p.x/1000 - 500)/W - floor((p.x/1000 - 500)/W);
  double dy = (p.y/1000 - 6570)/H - floor((p.y/1000 - 6570)/H);

  int N2 = 1;
  if (dx > 0.5) N2+=2;
  if (dy > 0.5) N2+=1;
  std::ostringstream str;
  str << ch[A1] << N1 << N2;
  return str.str();
}
