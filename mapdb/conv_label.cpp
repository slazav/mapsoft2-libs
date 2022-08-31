#include "conv_label.h"
#include <map>
#include <string>
#include <iostream>

std::map<std::string,std::string> conv_label_map = {
  {"`A",  "\u00C0"}, {"`a",  "\u00E0"},
  {"\'A", "\u00C1"}, {"\'a", "\u00E1"},
  {"^A",  "\u00C3"}, {"^a",  "\u00E2"},
  {"~A",  "\u00C3"}, {"~a",  "\u00E3"},
  {"\"A", "\u00C4"}, {"\"a", "\u00E4"},
  {"AA",  "\u00C5"}, {"aa",  "\u00E5"},
  {"AE",  "\u00C6"}, {"ae",  "\u00E6"},
  {"cC",  "\u00C7"}, {"cc",  "\u00E7"},
  {"`E",  "\u00C8"}, {"\'e", "\u00E8"},
  {"\'E", "\u00C9"}, {"\'e", "\u00E9"},
  {"^E",  "\u00CA"}, {"^e",  "\u00EA"},
  {"\"E", "\u00CB"}, {"\"e", "\u00EB"},
  {"`I",  "\u00CC"}, {"\'i", "\u00EC"},
  {"\'I", "\u00CD"}, {"\'i", "\u00ED"},
  {"^I",  "\u00CE"}, {"^i",  "\u00EE"},
  {"\"I", "\u00CF"}, {"\"i", "\u00EF"},
  {"NN",  "\u00D1"}, {"nn", "\u00F1"},
  {"`O",  "\u00D2"}, {"\'o", "\u00F2"},
  {"\'O", "\u00D3"}, {"\'o", "\u00F3"},
  {"^O",  "\u00D4"}, {"^o",  "\u00F4"},
  {"~O",  "\u00D5"}, {"~o",  "\u00F5"},
  {"\"O", "\u00D6"}, {"\"o", "\u00F6"},
  {"OO",  "\u00D8"}, {"oo",  "\u00F8"},
  {"`U",  "\u00D9"}, {"\'u", "\u00F9"},
  {"\'U", "\u00DA"}, {"\'u", "\u00FA"},
  {"^U",  "\u00DB"}, {"^u",  "\u00FB"},
  {"\"U", "\u00DC"}, {"\"u", "\u00FC"},
  {"\'Y", "\u00DD"}, {"\'y", "\u00FD"},

  {".C",  "\u010A"}, {".c",  "\u010B"},
  {"vC",  "\u010C"}, {"vc",  "\u010D"},
  {"LL",  "\u0141"}, {"ll",  "\u0142"},
  {"\'L", "\u013D"}, {"\'l", "\u013E"},
  {"OE",  "\u0152"}, {"oe",  "\u0153"},
  {"vT",  "\u0164"}, {"\'t",  "\u0165"},
  {"\'Z", "\u0179"}, {"\'z", "\u017A"},
  {".Z",  "\u017B"}, {".z",  "\u017C"},
  {"vZ",  "\u017D"}, {"vz",  "\u017E"}
};

std::string
conv_label(const std::string & str){
  std::string ret;
  for (size_t i = 0; i<str.size(); i++) {
    auto c = str[i];
    if (c!='#') { ret.push_back(c); continue; }
    if (i>=str.size()-1) {
      std::cerr << "conv_label: skipping # at the end: "<< str <<"\n";
      continue;
    }
    i++;
    if (str[i]=='#') {ret+=c; continue;}  // "##"
    if (i>=str.size()-1) {
      std::cerr << "conv_label: skipping # at the end: "<< str <<"\n";
      continue;
    }
    auto ss = str.substr(i,2);
    i++;
    auto ii = conv_label_map.find(ss);
    if (ii==conv_label_map.end()){
      std::cerr << "conv_label: unknown sequence: #" << ss << " in: "<< str <<"\n";
      continue;
    } // unknown
    ret += ii->second;
  }
  return ret;
}
