#include <iomanip>
#include <algorithm>
#include <fstream>
#include <cstring>

#include "iconv/iconv.h"
#include "mp.h"

using namespace std;


/*********************************************************/
void
ms2opt_add_mp(GetOptSet & opts){
  const char *g = "MP";
  opts.add("mp_in_enc", 1,0,g,
    "Override input encoding for MP-files (does not change CodePage setting)");
  opts.add("mp_out_enc", 1,0,g,
    "Override output encoding for MP-files (does not change CodePage setting)");
}

/*********************************************************/
// get line, trim \r
string
getl(istream & f){
  string l;
  getline(f, l);
  if (l.size() && *l.rbegin()=='\r') l.resize(l.size()-1);
  return l;
}

// write comments
void
write_comm(ostream & oo, const string & s){
  if (s.size() == 0) return;
  std::istringstream ii(s);
  while (!ii.eof()) oo << ";" << getl(ii) << "\r\n";
}

// replace \r, \n with spaces (for Name, Title fields)
std::string
protect_name(const string & s){
  string ret = s;
  std::replace(ret.begin(), ret.end(), '\n', ' ');
  std::replace(ret.begin(), ret.end(), '\r', ' ');
  return ret;
}

// icasecmp for std::strings
bool
icasecmp(const string& l, const string& r) {
  if (l.size() != r.size()) return false;
  for (size_t i=0; i<l.size(); i++)
    if (toupper(l[i]) != toupper(r[i])) return false;
  return true;
}

// icasencmp for std::strings
bool
icasencmp(const string& l, const string& r, size_t n) {
  if (l.size()<n || r.size()<n) return false;
  for (size_t i=0; i<n; i++)
    if (toupper(l[i]) != toupper(r[i])) return false;
  return true;
}


/*********************************************************/
void read_mp(istream & f, MP & data, const Opt & opts){

  MPObj o;
  dLine pts;

  if (!f) throw Err() << "can't read MP file";

  // overwrite header
  MP tmp;
  tmp.std::list<MPObj>::operator=(data); // copy objects if any
  data = tmp;

  // comments and empty lines
  string l = getl(f);
  while (l.size()==0 || l[0]==';'){
    if (l.size()>0){
      if (data.Comment.size()>0) data.Comment += '\n';
      data.Comment += l.substr(1);
    }
    l = getl(f);
  }

  // [IMG ID]
  if (!icasecmp(l,"[IMG ID]"))
    throw Err() << "read_mp: MP header expected: [" << l << "]";
  while (1){
    l = getl(f);
    if (f.eof()) throw Err() << "read_mp: unclosed MP header";
    if (icasencmp(l,"[END",4)) break;
    if (l.size() && l[0]==';') continue;

    // TODO: key=value pairs
    size_t p = l.find('=');
    if (p == string::npos)
      throw Err() << "read_mp: key=pair value expected: [" << l << "]";
    string key=l.substr(0,p);
    string val=l.substr(p+1);

    // TODO: non-wgs datums?
    if (icasecmp(key, "ID")) {
      data.ID=str_to_type<int>(val);
      continue;
    }
    if (icasecmp(key, "Name")) {
      data.Name=val;
      continue;
    }
    if (icasecmp(key, "LblCoding")) {
      data.LblCoding = str_to_type<int>(val);
      if (data.LblCoding!=6 && data.LblCoding!=9 &&
          data.LblCoding!=10) throw Err() <<
            "ead_mp: bad LblCoding setting" << data.LblCoding;
      continue;
    }
    if (icasecmp(key, "Codepage")) {
      data.Codepage = val;
      continue;
    }
    if (icasecmp(key, "Elevation")) {
      data.Elevation = val.size()?val[0]:'M';
      if (data.Elevation != 'M' && data.Elevation != 'F')
        throw Err() << "mp_read: unsupported Elevation: " << data.Elevation;
      continue;
    }
    if (icasecmp(key, "TreSize")) {
      data.TreSize = str_to_type<int>(val);
      continue;
    }
    if (icasecmp(key, "RgnLimit")) {
      data.RgnLimit = str_to_type<int>(val);
      continue;
    }
    if (icasecmp(key, "PreProcess")) {
      data.PreProcess = val.size()?val[0]:'F';
      if (data.PreProcess != 'F' && data.PreProcess != 'G' &&
          data.PreProcess != 'N') throw Err() <<
            "mp_read: unsupported PreProcess: " << data.PreProcess;
      continue;
    }
    if (icasecmp(key, "Levels"))   {
       int n = str_to_type<int>(val);
       if (n<2 || n>10) throw Err() << "read_mp: Levels setting 2..10 is expected: [" << l << "]";
       data.Levels = vector<int>(n, 0);
       continue;
    }
    if (icasencmp(key, "Level", 5)){
      int n = str_to_type<int>(key.substr(5));
      if (n<0 || n>=(int)data.Levels.size())
        throw Err() << "read_mp: wrong level number: [" << l << "]";
      data.Levels[n]=str_to_type<int>(val);
      if (n>0 && data.Levels[n]>=data.Levels[n-1])
        throw Err() << "read_mp: Level" << n
                    << " should be less then Level" << n-1;
       continue;
    }

    if (icasencmp(key, "Zoom", 4)){
      continue;
    }

    data.Opts.put(key, val);
  }

  if (*data.Levels.rbegin() == 0)
    throw Err() << "read_mp: not all levels are set in the header";

  // make encoding converter, convert name and comments in the header
  // override codepage from options
  std::string enc = "UTF-8"; // default
  if (data.Codepage != "")   enc = "CP" + data.Codepage; // from file
  if (opts.exists("mp_in_enc")) enc = opts.get("mp_in_enc"); // from opts

  IConv cnv(enc, "UTF-8");
  data.Name = cnv(data.Name);
  for (auto & o: data.Opts) o.second = cnv(o.second);
  data.Comment = cnv(data.Comment);

  int mode=0;
  // mode=0 -- between objects
  // mode=1 -- skip
  // mode=2 -- point, polyline, polygon
  string comm;
  bool inv = false; // for handling Direction field
  while (!f.eof()) {
    l = getl(f);
    if (l.size()==0) continue;
    // skip all declarations and unsupported objects
    if (icasecmp(l,"[COUNTRIES]")   || icasecmp(l,"[REGIONS]") ||
        icasecmp(l,"[CITIES]")      || icasecmp(l,"[CHART INFO]") ||
        icasecmp(l,"[DICTIOANRY]")  || icasecmp(l,"[BACKGROUND]") ||
        icasecmp(l,"[HIGHWAYS]")    || icasecmp(l,"[ZIPCODES]") ||
        icasecmp(l,"[DEFINITIONS]") || icasecmp(l,"[PLT]") ||
        icasecmp(l,"[WPT]")         || icasecmp(l,"[DBX]") ||
        icasecmp(l,"[SHP]")         || icasecmp(l,"[FILE]")) {mode=1; continue;}

    // supported objects
    if (icasecmp(l,"[POI]") || icasecmp(l,"[RNG10]") || icasecmp(l,"[RNG20]")) {
      mode=2;
      o = MPObj();
      o.Class = MP_POINT;
      o.Comment = comm;
      continue;
    }
    if (icasecmp(l,"[POLYLINE]") || icasecmp(l,"[RNG40]")) {
      mode=2;
      o = MPObj();
      o.Class = MP_LINE;
      o.Comment = comm;
      continue;
    }
    if (icasecmp(l,"[POLYGON]") || icasecmp(l,"[RNG80]")) {
      mode=2;
      o = MPObj();
      o.Class = MP_POLYGON;
      o.Comment = comm;
      continue;
    }

    // end of object, known or unknown
    if (mode>0 && icasencmp(l,"[END",4)){
      if (mode>1){
         data.push_back(o);
      }
      mode=0; inv=false;
      comm.clear();
      continue;
    }

    // comments between objects
    if (mode==0 && l[0]==';') {
      if (comm.size()>0) comm += '\n';
      comm+=cnv(l.substr(1));
      continue;
    }

    // nothing else should be between objects
    if (mode==0)
      throw Err() << "read_mp: unexpected line between objects: [" << l << "]";

    // skip unknown things
    if (mode==1) continue;

    // find key=value pairs
    size_t p = l.find('=');
    if (p == string::npos)
      throw Err() << "read_mp: key=pair value expected: [" << l << "]";
    string key=l.substr(0,p);
    string val=l.substr(p+1);

    // object parameters and data
    if (icasecmp(key, "Type")){
      o.Type=str_to_type<int>(val);
      continue;
    }
    if (icasecmp(key, "Label")){
      o.Label=cnv(val);
      continue;
    }
    if (icasecmp(key, "EndLevel")){
      o.EndLevel=str_to_type<int>(val);
      continue;
    }
    if (icasecmp(key, "Direction")){
      // Direction field is a non-standard field used in
      // in mapsoft1. Invert lines if Direction==2
      inv = (str_to_type<int>(val) == 2);
      continue;
    }
    if (icasencmp(key, "Data", 4) ||
        icasencmp(key, "Origin", 6)){
      int n = str_to_type<int>(key.substr( key[0]=='D'? 4:6) );
      if (n<0 || n>=(int)data.Levels.size()-1)
        throw Err() << "read_mp: wrong level number: [" << l << "]";
      o.Data.resize(data.Levels.size()-1);
      // read coordinates
      dLine ll;
      std::istringstream ss(val);
      char c1,c2,c3;
      double v1,v2;
      while (!ss.eof()) {
        ss >> c1 >> v1 >> c2 >> v2 >> c3;
        if (!ss || c1!='(' || c2!=',' || c3!=')')
          throw Err() << "read_mp: bad Data line:\n"
                         " [" << l << "]";
        ll.push_back(dPoint(v2,v1));
        ss >> c1;
        if (!ss.eof() && c1!=',')
          throw Err() << "read_mp: bad Data line:\n"
                         " [" << l << "]";
      }
      if (inv) ll.invert();
      o.Data[n].push_back(ll);
      continue;
    }

  }
}


void write_mp(ostream & out, const MP & data, const Opt & opts){

  // converting some fields from UTF8 to MP codepage
  string enc = "UTF-8";
  if (data.Codepage != "")   enc = "CP" + data.Codepage; // from data
  if (opts.exists("mp_out_enc")) enc = opts.get("mp_out_enc"); // from opts
  IConv cnv("UTF-8", enc);

  write_comm(out, cnv(data.Comment));

  if (data.LblCoding!=6 && data.LblCoding!=9 && data.LblCoding!=10)
    throw Err() << "mp_write: unsupported LblCoding: " << data.LblCoding;

  if (data.Elevation != 'M' && data.Elevation != 'F')
    throw Err() << "mp_write: unsupported Elevation: " << data.Elevation;

  if (data.PreProcess != 'F' && data.PreProcess != 'G' &&
      data.PreProcess != 'N') throw Err() <<
            "mp_write: unsupported PreProcess: " << data.PreProcess;

  out << setprecision(6) << fixed
      << "[IMG ID]\r\n"
      << "ID=" << data.ID << "\r\n"
      << "Name="       << protect_name(cnv(data.Name)) << "\r\n"
      << "LblCoding="  << data.LblCoding  << "\r\n"
      << "Codepage="   << data.Codepage   << "\r\n"
      << "Elevation="  << data.Elevation  << "\r\n"
      << "TreSize=" << data.TreSize       << "\r\n"
      << "RgnLimit=" << data.RgnLimit     << "\r\n"
      << "PreProcess=" << data.PreProcess << "\r\n";

  if (data.Levels.size()>10 || data.Levels.size()<2)
    throw Err() << "mp_write: Levels setting 2..10 exected instead of " << data.Levels.size();
  out << "Levels=" << data.Levels.size() << "\r\n";
  for (size_t i=0; i<data.Levels.size(); i++)
    out << "Level" << i << "=" << data.Levels[i] << "\r\n";
  for (size_t i=0; i<data.Levels.size(); i++)
    out << "Zoom" << i << "=" << i << "\r\n";

  // other options
  for (auto o:data.Opts)  out << o.first << "=" << cnv(o.second) << "\r\n";

  out << "[END-IMG ID]\r\n";

  for (auto obj:data){
    write_comm(out, cnv(obj.Comment));

    switch (obj.Class) {
      case MP_POINT:   out << "[POI]\r\n"; break;
      case MP_LINE:    out << "[POLYLINE]\r\n"; break;
      case MP_POLYGON: out << "[POLYGON]\r\n"; break;
      default: throw Err() << "mp_write: unknown object class: " << obj.Class;
    }
    out << "Type=0x"     << setbase(16) << obj.Type << setbase(10) << "\r\n";
    if (obj.Label != "")   out << "Label=" << protect_name(cnv(obj.Label)) << "\r\n";
    if (obj.EndLevel  != 0) out << "EndLevel=" << obj.EndLevel << "\r\n";

    // other options
    for (auto o:obj.Opts)  out << o.first << "=" << protect_name(cnv(o.second)) << "\r\n";

    // data
    if (obj.Data.size() > data.Levels.size()-1)
      throw Err() << "write_mp: too large level in data: " << obj.Data.size();

    for (size_t i=0; i<obj.Data.size(); i++){
      for (auto l:obj.Data[i]){
        out << "Data" << i << "=";
        for (size_t j=0; j<l.size(); j++){
          out << ((j!=0)?",":"")
              << "(" << l[j].y << "," << l[j].x << ")";
        }
        out << "\r\n";
      }
    }

    out << "[END]\r\n\r\n";
  }
}

