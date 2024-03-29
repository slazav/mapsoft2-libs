#include <list>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstdlib> // for atoi

#include "vmap.h"

#define CUR_VER 3.2

const int vmap_point_scale = 1000000;
using namespace std;

/***************************************/

// read key-value pair separated by tab char
int vmap_get_kv(const string &s, string &key, string &val){
  if (s=="") return 1;
  int tab=s.find('\t', 0);
  if (tab==-1) throw Err() << "can't find key-value pair";
  int st=0;
  while (s[st]==' ') st++;
  key=s.substr(st, tab-st);
  val=s.substr(tab+1, -1);
  return 0;
}

// read a single point (x,y), divide result by vmap_point_scale
dPoint vmap_read_pt(istream & IN){
  dPoint p;
  char sep;
  IN >> p.x >> sep >> p.y;
  if (!IN.eof()) IN >> std::ws;
  if (sep!=',') throw Err() << "can't read point";
  p/=vmap_point_scale;
  return p;
}

// Read label parameters: align, horizontality or angle, font size
// v<3.1:  <align:0|1|2> <hor:0|1> <ang>
// v>=3.1: L|C|R H|<ang>
// v>=3.2: L|C|R H|<ang> S<fsize>

void vmap_read_lab_pars(istream & IN, VMapLab & l, double ver){
  if (ver<3.1){
    IN >> l.dir >> l.hor >> l.ang;
  }
  else { // version 3.1...
    char c;
    IN >> c;
    switch (c){
      case 0: case 'l': case 'L': l.dir=0; break;
      case 1: case 'c': case 'C': l.dir=1; break;
      case 2: case 'r': case 'R': l.dir=2; break;
    }
    string s;
    IN >> s;
    if (s.size() && (s[0]=='H' || s[0]=='h')){
      l.hor=1;
      l.ang=0.0;
    }
    else{
      l.hor=0;
      l.ang=atof(s.c_str());
    }
    IN >> s; // version 3.2
    if (s.size() && s[0]=='S')  l.fsize = atoi(s.c_str()+1);
    else l.fsize = 0;
  }
}

VMapLab vmap_read_lab_pos(const string & s, double ver){
  VMapLab ret;
  istringstream IN1(s);
  ret.pos = vmap_read_pt(IN1);
  vmap_read_lab_pars(IN1, ret, ver);
  return ret;
}

VMapLfull vmap_read_lbuf(const string & s, double ver){
  VMapLfull ret;
  istringstream IN1(s);
  ret.pos = vmap_read_pt(IN1);
  vmap_read_lab_pars(IN1, ret, ver);
  ret.ref = vmap_read_pt(IN1);
  getline(IN1, ret.text);
  return ret;
}

dLine
read_vmap_points(istream & IN, string & s, size_t & nline){
  dLine ret;
  string key,val;
  if (vmap_get_kv(s, key, val)!=0)
    throw Err() << "wrong call of read_vmap_points()";
  s=val;
  do {
    istringstream IN1(s);
    while (IN1.good()){
      ret.push_back(vmap_read_pt(IN1));
    }
    getline(IN, s); nline++;
  } while (s[0]=='\t');
  return ret;
}


VMapObj
read_vmap_object(istream & IN, string & s, double ver, size_t & nline){
  VMapObj ret;
  string key,val;
  bool read_ahead=false;
  auto s0 = s; // save object header string for error message
  if ((vmap_get_kv(s, key, val)!=0) || (key!="OBJECT"))
    throw Err() << "wrong call of read_vmap_object()";

  istringstream IN1(val);
  IN1 >> setbase(16) >> ret.type >> ws;
  getline(IN1,ret.text);

  bool inv = false;
  while (!IN.eof() || read_ahead){
    if (!read_ahead) { getline(IN, s); nline++;}
    else read_ahead=false;
    auto s1 = s; // save string for error message

    try{
      if (vmap_get_kv(s, key, val)!=0) continue;
        if (ver<3.1 && key=="TEXT"){  // backward comp
        ret.text=val;
        continue;
      }
      if (key=="DIR"){
        inv = (atoi(val.c_str()) == 2);
        continue;
      }
      if (key=="OPT"){
        string k,v;
        if (vmap_get_kv(val, k, v)!=0)
          throw Err() << "bad options";
        ret.opts.put(k,v);
        continue;
      }
      if (key=="COMM"){
        if (ret.comm.size()>0) ret.comm+="\n";
         ret.comm+=val;
        continue;
      }
      if (key=="LABEL"){
        ret.labels.push_back(vmap_read_lab_pos(val, ver));
        continue;
      }
      if (key=="DATA"){
        ret.push_back(read_vmap_points(IN, s, nline));
        read_ahead=true;
        continue;
      }
      break; // end of object
    }
    catch (const Err & e){
      std::cerr << "read_vmap_object[" << nline << "]: "
                << e.str() << ": \"" << s1 << "\"\n";
    }
  }
  if (inv)
    for (auto & i:ret) i.invert();
  return ret;
}

// read vmap native format
VMap
read_vmap(istream & IN){
  VMap ret;
  string s, key, val;
  bool read_ahead=false;
  if (!IN) throw Err() << "can't read VMAP file";
  size_t nline = 0; // line number

  double ver;
  IN >> s >> ver; nline++;
  if (s!="VMAP"){
    cerr << "error: not a VMAP file\n";
    return ret;
  }
  if (ver>CUR_VER){
    cerr << "error: Too new VMAP format. Update mapsoft package.\n";
    return ret;
  }
  if (ver<CUR_VER){
    cerr << "note: reading old VMAP format version: "
         << fixed << setprecision(1) << ver << " < " << CUR_VER << "\n";
  }

  while (IN || read_ahead){

    if (!read_ahead) { getline(IN, s); nline++; }
    else read_ahead=false;
    auto s1=s; // save string for error message

    try {
      if (vmap_get_kv(s, key, val)!=0) continue;

      if (key=="NAME"){
        ret.name=val;
        continue;
      }
      if (key=="RSCALE"){
        ret.rscale=atof(val.c_str());
        continue;
      }
      if (key=="STYLE"){
        ret.style=val;
        continue;
      }
      if (key=="MP_ID"){
        ret.mp_id=atoi(val.c_str());
        continue;
      }
      if (key=="BRD"){
        ret.brd = read_vmap_points(IN, s, nline);
        read_ahead=true;
        continue;
      }
      if (key=="LBUF"){
        ret.lbuf.push_back(vmap_read_lbuf(val, ver));
        continue;
      }
      if (key=="OBJECT"){
        ret.push_back(read_vmap_object(IN, s, ver, nline));
        read_ahead=true;
        continue;
      }
      throw Err() << "unknown key";
    }
    catch (const Err & e){
      std::cerr << "read_vmap[" << nline << "]: "
                << e.str() << ": \"" << s1 << "\"\n";
    }
  }
  return ret;
}

/***************************************/


void write_vmap_line(ostream & OUT, const dLine & L){
  int n=0;
  for (auto &i:L){
    if ((n>0)&&(n%4==0)) OUT << "\n\t";
    else if (n!=0) OUT << " ";
    OUT << int(i.x) << "," << int(i.y);
    n++;
  }
}
// write label position
void write_vmap_lpos(ostream & OUT, const VMapLab & L){
  // coordinates
  OUT << int(L.pos.x) << "," << int(L.pos.y) << " ";
  // alignment (left,right,center)
  switch (L.dir){
    case 0: OUT << 'L'; break;
    case 1: OUT << 'C'; break;
    case 2: OUT << 'R'; break;
  }
  // angle (or H for horizontal labels)
  if (L.hor) OUT << " H";
  else  OUT << " " << setprecision(2) << round(L.ang*100)/100;

  // font size correction
  if (L.fsize) OUT << " S" << L.fsize;
}


// write vmap to ostream
// put vmap to mp
int
write_vmap(ostream & OUT, const VMap & W){

  VMap WS (W);

  // Sort problem: we write rounded values, so order can change!
  // Rounding values before sort:

  for (auto &l:WS.lbuf){
    l.pos=rint(l.pos*vmap_point_scale);
    l.ref=rint(l.ref*vmap_point_scale);
  }
  for (auto &o:WS){
    for (auto &l:o){
      for (auto &p:l){
        p=rint(p*vmap_point_scale);
      }
    }
    for (auto &l:o.labels){
      l.pos=rint(l.pos*vmap_point_scale);
    }
  }
  for (auto &p:WS.brd){
    p=rint(p*vmap_point_scale);
  }

  WS.sort();
  WS.lbuf.sort();

  OUT << "VMAP " << fixed << setprecision(1) << CUR_VER << "\n";
  if (WS.name!="") OUT << "NAME\t" << WS.name << "\n";
  OUT << "RSCALE\t" << int(WS.rscale) << "\n";
  OUT << "STYLE\t" << WS.style << "\n";
  OUT << "MP_ID\t" << WS.mp_id << "\n";
  if (WS.brd.size()>0){
    OUT << "BRD\t";  write_vmap_line(OUT, WS.brd); OUT << "\n";
  }

  // lbuf
  for (auto const &l:WS.lbuf){
    OUT << "LBUF\t"; write_vmap_lpos(OUT, l);
    OUT << " " << int(l.ref.x) << "," << int(l.ref.y);
    if (l.text.size()) OUT << " " << l.text;
    OUT << "\n";
  }

  for (auto &o:WS){
    OUT << "OBJECT\t" << "0x" << setbase(16) << o.type << setbase(10);
    if (o.text != "") OUT << " " << o.text;
    OUT << "\n";

    for (auto const &i:o.opts){
      OUT << "  OPT\t" << i.first << "\t" << i.second << "\n"; // protect \n!
    }

    if (o.comm.size()>0){
      std::istringstream ii(o.comm);
      while (!ii.eof()){
        string l;
        getline(ii, l);
        OUT << "  COMM\t" << l << "\n";
      }
    }

    o.labels.sort();
    for (auto const &i:o.labels){
      OUT << "  LABEL\t";  write_vmap_lpos(OUT, i); OUT << "\n";
    }
    for (auto const &i:o){
      OUT << "  DATA\t"; write_vmap_line(OUT, i); OUT << "\n";
    }
  }

  return 0;
}


VMap read_vmap(const std::string &fname){
  std::ifstream s(fname);
  if (!s) throw Err() << "can't open file: " << fname;
  return read_vmap(s);
}
int write_vmap(const std::string &fname, const VMap & W){
  std::ofstream s(fname);
  if (!s) throw Err() << "can't open file: " << fname;
  return write_vmap(s, W);
}
