#include <fstream>
#include <deque>
#include "filename/filename.h"
#include "iconv/iconv.h"
#include "vmap2types.h"
#include "vmap2obj.h"

void
ms2opt_add_vmap2t(GetOptSet & opts){
  const char *g = "VMAP2";
  opts.add("types",  1, 't', g, "File with type information. Default: /usr/share/mapsoft2/types.cfg");
  opts.add("define", 1, 'D', g, "Define variables for type information file.");
}

VMap2types::VMap2types(const Opt & o){
  std::string file = o.get("types", "/usr/share/mapsoft2/types.cfg");
  if (file == "") return;
  read_words_defs defs(o.get("define", Opt()));
  load(file, defs);
}

void
VMap2types::write_typ(const std::string & fname, const int codepage, const int FID) const {
  std::ofstream ff(fname);
  if (!ff) throw Err() << "vmap2types: can't write file: " << fname;

  // header
  ff << "; Generated by mapsoft\n";
  ff << "[_id]\n";
  ff << "ProductCode=1\n";
  if (FID>=0) ff << "FID=" << FID << "\n";
  if (codepage>=0) ff << "CodePage=" << codepage << "\n";
  ff << "[end]\n\n";

  // convert all names to the typ codepage
  IConv cnv("UTF-8", "CP" + type_to_str(codepage));

  // draw order
  ff << "[_drawOrder]\n";

  for (const auto tt: *this){
    auto cl = VMap2obj::get_class(tt.first);
    if (cl != VMAP2_POLYGON) continue;
    auto tnum = VMap2obj::get_tnum(tt.first);
    auto order = tt.second.typ_order;
    ff << "Type=0x" << std::hex << cl << std::dec << ","
       << tt.second.typ_order
       << "  ; " << cnv(tt.second.name)
       << "\n";
  }
  ff << "[end]\n\n";

  // types
  for (const auto tt: *this){
    auto cl = VMap2obj::get_class(tt.first);
    switch (cl){
      case VMAP2_POINT:   ff << "[_point]\n";   break;
      case VMAP2_LINE:    ff << "[_line]\n";    break;
      case VMAP2_POLYGON: ff << "[_polygon]\n"; break;
      default: continue;
    }


    ff << "Type=0x" << std::hex << cl << std::dec << "\n";
    // could be up to 4 languages
    if (tt.second.name!="")
      ff << "string1=0x00," << cnv(tt.second.name) << "\n";
    if (tt.second.name!="")
      ff << "string2=0x04," << cnv(tt.second.name) << "\n";
    if (tt.second.name_ru!="")
      ff << "string3=0x19," << cnv(tt.second.name_ru) << "\n";
    if (tt.second.name_fi!="")
      ff << "string4=0x06," << cnv(tt.second.name_fi) << "\n";
    if (cl==VMAP2_LINE){
      if (tt.second.typ_line_width != 0)
        ff << "LineWidth="   << tt.second.typ_line_width << "\n";
      if (tt.second.typ_border_width != 0)
        ff << "BorderWidth=" << tt.second.typ_border_width << "\n";
    }
    if (tt.second.typ_xpm!=""){
      ff << "[dayxpm]=" << tt.second.typ_xpm;
    }
    ff << "[end]\n\n";
  }

}

void
VMap2types::print_type_list(const bool full) const{
  for (const auto & t: *this){
    if (full) {
      print_type_info(t.first);
      std::cout << "\n";
    }
    else {
      std::cout << VMap2obj::print_type(t.first) << "\n";
    }
  }
}

void
VMap2types::print_type_info(const uint32_t type) const{
  auto t = find(type);
  if (t == end()) throw Err() << "Unknown type: " << type;
  std::cout << "type:      " << VMap2obj::print_type(t->first) << "\n";
  std::cout << "group:     " << t->second.group    << "\n";
  std::cout << "name:      " << t->second.name     << "\n";
  std::cout << "name_ru:   " << t->second.name_ru  << "\n";
  std::cout << "comm:      " << t->second.comm     << "\n";
  std::cout << "fig_mask:  " << t->second.fig_mask << "\n";

  std::cout << "mp_start:  " << t->second.mp_start  << "\n";
  std::cout << "mp_end:    " << t->second.mp_end    << "\n";
  std::cout << "typ_order:        " << t->second.typ_order << "\n";
  std::cout << "typ_line_width:   " << t->second.typ_line_width << "\n";
  std::cout << "typ_border_width: " << t->second.typ_border_width << "\n";

  // Information for updating map labels
  std::cout << "label_type: " << t->second.label_type << "\n";
  std::cout << "label_def_scale:  " << t->second.label_def_scale << "\n";
  std::cout << "label_def_align:  " << t->second.label_def_align << "\n";
  std::cout << "label_def_mshift: " << t->second.label_def_mshift << "\n";
  std::cout << "label_maxnum:     " << t->second.label_maxnum << "\n";
}


void
VMap2types::load(const std::string & fname, read_words_defs & defs){

  std::ifstream ff(fname);
  if (!ff) throw Err() << "can't open file: " << fname;
  auto path = file_get_prefix(fname);

  int line_num[2] = {0,0}; // line counter for read_words
  int type = -1;           // current type
  std::deque<bool> ifs;  // for if/endif commands

  while (1){
    auto vs = read_words(ff, line_num, false);
    if (vs.size()==0) break;

    try{
      if (read_words_stdcmds(vs, defs, ifs)){
        type=-1; continue; }

      // include command
      if (vs[0] == "include"){
        if (vs.size()!=2) throw Err() << "include: filename expected";
        auto fn = vs[1];
        // should not happend, but lets check before accessing fn[0]:
        if (fn.size() == 0) throw Err() << "include: empty filename";

        if (fn[0] != '/') fn = path + fn;
        load(fn, defs);
        type=-1;
        continue;
      }

      // type <type> -- add description of an object type
      if (vs[0] == "type") {
        if (vs.size()!=2) throw Err() << "type: argument expected: <type>";
        type = VMap2obj::make_type(vs[1]);
        emplace(type, VMap2type());
        continue;
      }

      // + <feature> <args> ... -- define feature of a point, a line or an area object
      if (vs[0] == "+") {
        if (type == -1)  throw Err() << "+ expected after point, line, or area command";
        auto o = find(type);
        if (o==end())    throw Err() << "can't find object type" << type;
        if (vs.size()<2) throw Err() << "+: at least one argument expected";

        // features
        if (vs[1] == "group"){
          if (vs.size()!=3) throw Err() << "+ group: argument expected: <group>";
          o->second.group = vs[2];
          continue;
        }
        if (vs[1] == "name"){
          if (vs.size()!=3) throw Err() << "+ name: argument expected: <name>";
          o->second.name = vs[2];
          continue;
        }
        if (vs[1] == "name_ru"){
          if (vs.size()!=3) throw Err() << "+ name_ru: argument expected: <name>";
          o->second.name_ru = vs[2];
          continue;
        }
        if (vs[1] == "name_fi"){
          if (vs.size()!=3) throw Err() << "+ name_ru: argument expected: <name>";
          o->second.name_ru = vs[2];
          continue;
        }
        if (vs[1] == "comm"){
          if (vs.size()!=3) throw Err() << "+ comm: argument expected: <description>";
          o->second.comm = vs[2];
          continue;
        }
        if (vs[1] == "fig_mask"){
          if (vs.size()!=3) throw Err() << "+ fig_mask: argument expected: <fig mask>";
          o->second.fig_mask = vs[2];
          continue;
        }
        if (vs[1] == "fig_pic"){
          if (vs.size()!=3 && vs.size()!=4) throw Err() << "+ fig_pic: argument expected: <fig path> [<scale>]";
          read_fig(path + vs[2], o->second.fig_pic);
          if (vs.size()==4) o->second.fig_pic *= str_to_type<double>(vs[3]);
          continue;
        }
        if (vs[1] == "mp_start"){
          if (vs.size()!=3) throw Err() << "+ mp_start: argument expected: <start level>";
          o->second.mp_start = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "mp_end"){
          if (vs.size()!=3) throw Err() << "+ mp_end: arguments expected: <end level>";
          o->second.mp_end = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "typ_order"){
          if (vs.size()!=3) throw Err() << "+ typ_order: arguments expected: <number 1..8>";
          o->second.typ_order = str_to_type<int>(vs[2]);
          if (o->second.typ_order < 1 || o->second.typ_order > 8)
            throw Err() << "+ typ_order: number 1..8 expected";
          continue;
        }
        if (vs[1] == "typ_line_width"){
          if (vs.size()!=3) throw Err() << "+ typ_line_width: arguments expected: <number 1..8>";
          o->second.typ_line_width = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "typ_border_width"){
          if (vs.size()!=3) throw Err() << "+ typ_border_width: arguments expected: <number 1..8>";
          o->second.typ_border_width = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "typ_xpm"){
          if (vs.size()!=3) throw Err() << "+ typ_xpm: arguments expected: <file name>";
          // very basic xpm file reading: just select everyting enclosed in "" from each string
          std::ifstream ff(vs[2]);
          if (!ff) throw Err() << "+ typ_xpm: can't read XPM file: " << vs[2];
          o->second.typ_xpm="";
          std::string line;
          while (std::getline (ff,line)){
            auto n1 = line.find('"'), n2 = line.rfind('"');
            if (n1!=line.npos && n2!=line.npos && n2>n1)
              o->second.typ_xpm += line.substr(n1,n2+1) + '\n';
          }
          continue;
        }
        if (vs[1] == "label_type"){
          if (vs.size()!=3) throw Err() << "+ label_type: argument expected: <type number or -1>";
          o->second.label_type = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "label_def_scale"){
          if (vs.size()!=3) throw Err() << "+ label_def_scale: argument expected: <scale>";
          o->second.label_def_scale = str_to_type<float>(vs[2]);
          continue;
        }
        if (vs[1] == "label_def_align"){
          if (vs.size()!=3) throw Err() << "+ label_def_align: argument expected: <align, W,C,E>";
          o->second.label_def_align = VMap2obj::parse_align(vs[2]);
          continue;
        }
        if (vs[1] == "label_def_mshift"){
          if (vs.size()!=4) throw Err() << "+ label_def_mshift: 2 arguments expected: <dx> <dy>";
          o->second.label_def_mshift = dPoint(str_to_type<double>(vs[2]), str_to_type<double>(vs[3]));
          continue;
        }
        if (vs[1] == "label_mkpt"){
          if (vs.size()!=3) throw Err() << "+ label_mkpt: argument expected: <type number or -1>";
          o->second.label_mkpt = str_to_type<int>(vs[2]);
          continue;
        }
        if (vs[1] == "label_maxnum"){
          if (vs.size()!=3) throw Err() << "+ label_maxnum: argument expected: <number, -1 for inf, -2 for auto>";
          o->second.label_maxnum = str_to_type<int>(vs[2]);
          continue;
        }

        throw Err() << "unknown feature: " << vs[1];
      }
      throw Err() << "unknown command: " << vs[0];

    }
    catch (Err & e) {
      throw Err() << fname << ":" << line_num[0] << ": " << e.str();
    }

  }
}