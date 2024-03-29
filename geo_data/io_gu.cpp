#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "opt/opt.h"
#include "err/err.h"
#include "time_fmt/time_fmt.h"
#include "iconv/iconv.h"

#include "geo_data.h"

using namespace std;
string gu_default_enc("KOI8-R");

void read_gu (const string &fname, GeoData & data, const Opt & opts){
  IConv cnv(opts.get("gu_enc", gu_default_enc), "UTF-8");
  bool v = opts.get("verbose", false);
  if (v) cerr << "Reading GarminUtils file: " << fname << endl;

  ifstream s(fname);
  int mode = 0;
  GeoWptList wpt;
  GeoTrk trk;

  while (!s.eof()){
    string l;
    getline(s, l);

    if (l.compare(0, 10, "[waypoints")==0) {
      wpt.clear(); trk.clear(); mode = 1;
      continue;
    }
    if (l.compare(0,  7, "[tracks")==0) {
      wpt.clear(); trk.clear(); mode = 2;
      continue;
    }
    if (l.compare(0,  4, "[end")==0) {
      if (wpt.size()){
        if (v) cerr << "  Reading waypoints: "
                    << "(" << wpt.size() << " points)" << endl;
        data.wpts.push_back(wpt);
      }
      if (trk.size()){
        if (v) cerr << "  Reading track: "
                    << "(" << trk.npts() << " points)" << endl;
        data.trks.push_back(trk);
      }
      wpt.clear(); trk.clear(); mode = 0;
      continue;
    }
    if (mode == 1){
      GeoWpt p;
      int symb, displ;
      char c;
      istringstream s1(l);
      s1 >> p.name >> p.y >> p.x >> symb >> c >> displ;
      if (!s1.eof()) s1 >> ws;
      getline(s1, p.comm);
      if (s1.fail() || !s1.eof() || c != '/')
        throw Err() << "io_gu: can't parse a waypoint: [" << l << "]";
      p.name = cnv(p.name);
      p.comm = cnv(p.comm);
      wpt.push_back(p);
      continue;
    }
    if (mode == 2){
      GeoTpt p;
      string t1,t2,st;
      istringstream s1(l);
      s1 >> p.y >> p.x >> t1 >> t2;
      p.t = parse_utc_time(t1 + " " + t2);
      if (!s1.eof()) s1 >> ws;
      if (!s1.eof()) {
        s1 >> st;
        if (st != "start")
          throw Err() << "io_gu: can't parse a trackpoint: [" << l << "]";
        trk.add_segment();
      }
      if (s1.fail() || !s1.eof())
        throw Err() << "io_gu: can't parse a trackpoint: [" << l << "]";

      trk.add_point(p);
      continue;
    }
  }
}


void write_gu_waypoints(ostream & s, const GeoWptList & wp,
                        const IConv & cnv, const bool v){
  int num = wp.size();
  if (v) cerr << "  Writing waypoints: (" << num << " points)" << endl;
  s << "[waypoints, " << num << " records]\n";
  int symb  = 0;
  int displ = 0;
  for (const auto & p : wp){
    string name = cnv(p.name);
    string comm = cnv(p.comm);
    replace(name.begin(), name.end(), ' ', '_');

    s << left << setw(6) << setfill(' ') << name << " "
      << right << fixed << setprecision(6)
      << setw(10) << p.y << " "
      << setw(11) << p.x << " "
      << setw(5) << symb << "/" << displ << " "
      << comm  << "\n";
  }
  s << "[end transfer, " << num << "/" << num << " records]\n";
  if (s.fail()) throw Err() << "io_gu: Can't write waypoints";
}


void write_gu_track(ostream & s, const GeoTrk & tr, const bool v){
  int num = tr.npts();
  if (v) cerr << "  Writing track: (" << num << " points)" << endl;
  s << "[tracks, " << num << " records]\n";
  for (const auto & seg : tr){
    bool start = true;
    for (const auto & p : seg){
      s << right << fixed << setprecision(6) << setfill(' ')
        << setw(10)<< p.y << " "
        << setw(11)<< p.x << " "
        << setfill('0') << write_fmt_time("%F %T", p.t)
        << ((start)? " start":"") << "\n";
      start = false;
    }
  }
  s << "[end transfer, " << num << "/" << num << " records]\n";
  if (s.fail()) throw Err() << "io_gu: Can't write track";
}


void write_gu (const string &fname, const GeoData & data, const Opt & opts){
  IConv cnv("UTF-8", opts.get("gu_enc", gu_default_enc));
  bool v = opts.get("verbose", false);
  if (v) cerr << "Writing GarminUtils file: " << fname << endl;

  ofstream s(fname);
  s << "[product 00, version 000: MAPSOFT2]\n";
  for (auto wpl: data.wpts)
    write_gu_waypoints(s, wpl, cnv, v);

  for (auto trk : data.trks)
    write_gu_track(s, trk, v);
}
