#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <string>
#include <cassert>
#include "ocad_types.h"
#include "iconv/iconv.h"

using namespace std;

void
str_pas2c(char * str, int maxlen){
  int l = str[0];
  if (l>=maxlen){
    cerr << "warning: not a pascal-string?\n";
    l=maxlen-1;
  }
  for (int i=0; i<l; i++) str[i]=str[i+1];
  str[l]='\0';
}

string
str_pas2str(const char * str, int maxlen){
  int l = str[0]+1;
  if (l>maxlen){
    cerr << "warning: not a pascal-string? "
              << "(first byte: " << (int)str[0] << ")\n";
    l=maxlen;
  }
  if (l<1){
    cerr << "warning: not a pascal-string? "
              << "(first byte: " << (int)str[0] << ")\n";
    l=1;
  }
  return string(str+1, str+l);
}

void
str_str2pas(char * pas, const string & str, size_t maxlen){
  if (str.size() > maxlen-1)
    cerr << "warning: cropping string.";

  int size = std::min(str.size(), maxlen-1);
  pas[0] = size;
  for (int i=0; i<size; i++) pas[i+1]=str[i];
}

const IConv iconv_from_uni("UTF-16", "UTF8");
const IConv iconv_from_win("CP1251", "UTF8");
const IConv iconv_to_uni("UTF-8", "UTF-16");
const IConv iconv_to_win("UTF-8", "CP1251");

void
ocad_coord::dump(ostream & s) const{
  s << " " << getx() << "," << gety();
  if (getf()) s << "(" << setbase(16) << getf()
                       << setbase(10) << ")";
}

void
check_v(int * v){
  if ((*v<6)||(*v>9)){
     cerr << "unsupported version: " << *v << ", set to 9\n";
     *v=9;
  }
}
/********************************************************************/

void
ocad_cmyk_::set_int(int c){
  C=(c>>24)&0xFF;
  M=(c>>16)&0xFF;
  Y=(c>>8)&0xFF;
  K=c&0xFF;
}

int
ocad_cmyk_::get_rgb_int() const{
  int R = ((0xFF-C)*(0xFF-K))/0xFF;
  int G = ((0xFF-M)*(0xFF-K))/0xFF;
  int B = ((0xFF-Y)*(0xFF-K))/0xFF;
  return (R<<16) + (G<<8) + B;
}

void
ocad_cmyk_::set_rgb_int(int c){
  int R=(c>>16)&0xFF;
  int G=(c>>8)&0xFF;
  int B=c&0xFF;
  K = std::min(std::min(0xFF-R, 0xFF-G), 0xFF-B);
  if (K==0xFF) {
    C=M=Y=0;
  }
  else {
    C=(0xFF*(0xFF-R-K))/(0xFF-K);
    M=(0xFF*(0xFF-G-K))/(0xFF-K);
    Y=(0xFF*(0xFF-B-K))/(0xFF-K);
  }
}

void 
ocad_cmyk_::dump_hex(ostream & s) const{
  s << "0x" << setw(8) << setfill('0') << setbase(16)
            << get_int()
            << setfill(' ') << setbase(10);
}

void
ocad_cmyk_::dump_rgb_hex(ostream & s) const{
  s << "0x" << setw(6) << setfill('0') << setbase(16)
            << get_rgb_int()
            << setfill(' ') << setbase(10);
}

/*
ocad_parstr
ocad_colorinfo::to_string() const{
  ocad_parstr ret;
  ret.type = 9;
  ret.put("", std::string(name+1, name+1+name[0]));
  ret.put("n", num);
  ret.put("c", (int)color.C);
  ret.put("m", (int)color.M);
  ret.put("y", (int)color.Y);
  ret.put("k", (int)color.K);
  ret.put("o", "0");   // overprint
  ret.put("t", "100"); // transparency
  // ... other fields?
  return ret;
}

void
ocad_colorinfo::from_string(const ocad_parstr & s){
  if (s.type!=9) return;
  *this = ocad_colorinfo();
  auto str = s.get("");
  str.resize(31);
  name[0] = str.size();
  strncpy(name+1, str.c_str(), 31);
  num = s.get<int>("n");
  color.C = s.get<int>("c");
  color.M = s.get<int>("m");
  color.Y = s.get<int>("y");
  color.K = s.get<int>("k");
  //... other fields?
}
*/
/*
ocad_parstr
ocad_colorsep::to_string(int n) const{
  ocad_parstr ret;
  ret.type = 10;
  ret.put("", std::string(name+1, name+1+name[0]));
  ret.put("v", "1");
  ret.put("n", n);
  ret.put("f", raster.f);
  ret.put("a", raster.a/10.0);
  ret.put("c", color.C/2.0);
  ret.put("m", color.M/2.0);
  ret.put("y", color.Y/2.0);
  ret.put("k", color.K/2.0);
  return ret;
}

void
ocad_colorsep::from_string(const ocad_parstr & s){
  if (s.type!=10) return;
  *this = ocad_colorsep();
  auto str = s.get("");
  str.resize(15);
  name[0] = str.size();
  strncpy(name+1, str.c_str(), 15);
  color.C = s.get<int>("c"); // *2?
  color.M = s.get<int>("m"); // *2?
  color.Y = s.get<int>("y"); // *2?
  color.K = s.get<int>("k"); // *2?
  raster.f =s.get<int>("f");
  raster.a =s.get<int>("a"); // *10?
  //... other fields?
  // problem with number!
}
*/

/********************************************************************/

void
ocad_color_::dump(ostream & s, int num_sep) const{
  s << std::setw(3) << num << " CMYK: ";
  color.dump_hex(s);

  s << " SEP: [" << setfill('0') << setbase(16);
  for (int i=0; i<num_sep; i++){
     if (i>=MAX_COLSEP) break;
     s << " " << setw(2) << (int)sep_per[i];
  }
  s << setfill(' ') << setbase(10) << "]";
  s << " \"" << str_pas2str(name, 32) << "\"";
}

void
ocad_colsep_::dump(ostream & s) const{
  s << " " << setw(4) << raster.f << "/" << raster.a << " ";
  color.dump_hex(s);
  s << " \"" << str_pas2str(name, 16) << "\"";
}

void
ocad_header_::dump(std::ostream & s, const int verb){
  if (verb<1) return;
  std::cout
  << "OCAD " << version << "." << subversion << "\n"
  << "  type: " << ftype << "\n"
  ;
  if (verb<2) return;
  std::cout
  << "data blocks addresses:\n"
  << "  symbols:  " << sym_pos << "\n"
  << "  objects:  " << obj_pos << "\n"
  << "  strings:  " << str_pos << "\n"
  << "  setup v8: " << setup_pos << ":" << setup_size << "\n"
  << "  info  v8: " << info_pos << ":"  << info_size << "\n"
  << "  fname v9: " << fname_pos << ":" << fname_size << "\n"
  ;
}

void
ocad8_colinfo_::dump(std::ostream & s, const int verb){
  if (verb<1) return;
  std::cout
    << "Symbol header block: "
    << ncolors << " colors, "
    << ncolsep << " color separations\n";
  std::cout
    << "halftone frequency/angle:\n"
    << " C: " << C.f << "/" << C.a << "\n"
    << " M: " << M.f << "/" << M.a << "\n"
    << " Y: " << Y.f << "/" << Y.a << "\n"
    << " K: " << K.f << "/" << K.a << "\n"
    << "colors:\n";
  if (verb<2) return;
  for (int i=0; i<ncolors; i++){
    if (i>255) break;
    std::cout << " ";
    cols[i].dump(std::cout, ncolsep);
    std::cout << "\n";
  }
  std::cout << "color separations:\n";
  for (int i=0; i<ncolsep; i++){
    if (i>31) break;
    std::cout << " ";
    seps[i].dump(std::cout);
    std::cout << "\n";
  }
}

