#include "ocad_file.h"
#include "ocad_colors.h"
#include "err/err.h"

//#include "<string_view>"

#include <iostream>

using namespace std;

namespace ocad{

typedef ocad_index<ocad9_object> ocad9_objects;
typedef ocad_index<ocad8_object> ocad8_objects;
typedef ocad_index<ocad9_symbol> ocad9_symbols;
typedef ocad_index<ocad8_symbol> ocad8_symbols;

/************************************************/

void
ocad_seek(FILE *F, const size_t pos, const char * str){
  if (fseek(F, pos, SEEK_SET)!=0)
    throw Err() << "OCAD: can't do seek ("
                << str << ") : " << strerror(errno);
}

void
ocad_read(FILE *F, void *buf, const size_t len, const char * str){
  if (fread(buf, 1, len, F)!=len)
    throw Err() << "OCAD: can't read " << str << ": " << strerror(errno);
}

void
ocad_write(FILE *F, void *buf, const size_t len, const char * str){
  if (fwrite(buf, 1, len, F)!=len)
       throw Err() << "OCAD: can't write " << str << ": " << strerror(errno);
}


/************************************************/
// OCAD 6..9 header structure
struct ocad_header_struct{ // 48 bytes
  ocad_small ocad_mark;   // 3245 (hex 0cad)
  ocad_byte  ftype;       // v6: 0
                          // v7: 7
                          // v8: 2: normal map, 3: course setting
                          // v9: 0: normal map, 1: course setting
  ocad_byte  fstatus;     // not used
  ocad_small version;     // 6..9
  ocad_small subversion;  // (0 for 9.00, 1 for 9.1 etc.)
  ocad_long  sym_pos;     // file position of the first symbol index block
  ocad_long  obj_pos;     // file position of the first index block
  ocad_long  setup_pos;   // OCAD 6/7/8: file position of the setup record
                          // OCAD 9: reserved
  ocad_long  setup_size;  // OCAD 6/7/8: size (in bytes) of the setup record
                          // OCAD 9: reserved
  ocad_long  info_pos;    // OCAD 6/7/8: file position of the file information (0-term string up to 32767
                          // OCAD 9: reserved
  ocad_long  info_size;   // OCAD 6/7/8: size (in bytes) of the file information
                          // OCAD 9: reserved
  ocad_long  str_pos;     // OCAD 8/9: file position of the first string index block
  ocad_long  fname_pos;   // OCAD 9: file position of file name, used for temporary files
                          //         and to recovery the file
  ocad_long  fname_size;  // OCAD 9: size of the file name, used for temporary files only
  ocad_long  r4;          // reserved
  ocad_header_struct(){
    assert(sizeof(*this) == 48);
    memset(this, 0, sizeof(*this));
    ocad_mark=0x0CAD;
  }
};

// Additional part of header for OCAD8 files.
// Information about colors and color separations.
// In OCAD9 all this data is kept in ocad_strings.
struct ocad8_head_add{ // 24 + 256*72 + 32*24 = 19224 bytes
  ocad_small ncolors;     // number of colors defined
  ocad_small ncolsep;     // number of color separations defined
  ocad_freq_ang C,M,Y,K;  // halftone frequency and angle of CMYK colors
  ocad_small r1,r2;
  ocad_colorinfo cols[256];
  ocad_colorsep  seps[32]; /// only 24 used
  ocad8_head_add(){
    assert(sizeof(*this) == 19224);
    memset(this, 0, sizeof(*this));
  }
};

/************************************************/

void
ocad_file::read(const char * fn, int verb){

  // clear some data
  symbols.clear();
  strings.clear();

  // open file
  FILE * F = fopen(fn, "r");
  if (!F) throw Err() << "OCAD: can't open file: " << strerror(errno);

  // read header
  ocad_header_struct header;
  ocad_read(F, &header, sizeof(header), "file header");
  version = header.version;
  subversion = header.subversion;

  // unsupported versions
  if ((version <6) || (version >9))
    throw Err() << "OCAD: incorrect version: " << version;

  // set file type
  if ( ((version == 8) && (header.ftype==3)) ||
       ((version == 9) && (header.ftype==1)) ) type=1;
  else type = 0;

  // dump header
  if (verb>0){
    std::cout
    << "OCAD " << version << "." << subversion << ", "
    << (type?"course setting":"normal map") << "\n"
    ;
  }
  if (verb>1){
    std::cout
    << "data blocks addresses:\n"
    << "  symbols:  " << header.sym_pos << "\n"
    << "  objects:  " << header.obj_pos << "\n"
    << "  strings:  " << header.str_pos << "\n"
    << "  setup v8: " << header.setup_pos << ":" << header.setup_size << "\n"
    << "  info  v8: " << header.info_pos << ":"  << header.info_size << "\n"
    << "  fname v9: " << header.fname_pos << ":" << header.fname_size << "\n"
    ;
  }

  // OCAD 6..8 -- read end of the header, convert to OCAD9 strings
  if (version<9){
    ocad8_head_add shead;
    ocad_read(F, &shead, sizeof(shead), "sheader");

    // convert to strings
    for (int i=0; i<std::min(shead.ncolors, (ocad_small)256); i++)
      strings.push_back(shead.cols[i].to_string());
    for (int i=0; i<std::min(shead.ncolsep, (ocad_small)MAX_COLSEP); i++)
      strings.push_back(shead.seps[i].to_string(i));

    // dump
    if (verb>0){
      std::cout
        << "Symbol header block: "
        << shead.ncolors << " colors, "
        << shead.ncolsep << " color separations\n";
    }
    if (verb>1){
      std::cout
        << "halftone frequency/angle:\n"
        << " C: " << shead.C.f << "/" << shead.C.a << "\n"
        << " M: " << shead.C.f << "/" << shead.C.a << "\n"
        << " Y: " << shead.C.f << "/" << shead.C.a << "\n"
        << " K: " << shead.C.f << "/" << shead.C.a << "\n"
        << "colors:\n";
      for (int i=0; i<std::min(shead.ncolors, (ocad_small)256); i++){
        std::cout << " ";
        shead.cols[i].dump(std::cout, shead.ncolsep);
        std::cout << "\n";
      }
      std::cout << "color separations:\n";
      for (int i=0; i<std::min(shead.ncolsep, (ocad_small)MAX_COLSEP); i++){
        std::cout << " ";
        shead.seps[i].dump(std::cout);
        std::cout << "\n";
      }
    }
  }

  // read symbols
  if (version>8){
    ocad9_symbols s9;
    s9.read(F, header.sym_pos, version);
    for (const auto & i: s9) symbols[i.sym] = i;
    s9.dump(verb);
  }
  else{
    ocad8_symbols s8;
    s8.read(F, header.sym_pos, version);
    ocad8_symbols::const_iterator i;
    for (const auto & i: s8) symbols[i.sym] = i;
    s8.dump(verb);
  }

  // read objects
  if (version>8){
    ocad9_objects o9;
    o9.read(F, header.obj_pos, version);
    objects.insert(objects.end(), o9.begin(), o9.end());
  }
  else {
    ocad8_objects o8;
    o8.read(F, header.obj_pos, version);
    objects.insert(objects.end(), o8.begin(), o8.end());
  }
  objects.dump(verb);

  // read strings
  if (version>7){
    strings.read(F, header.str_pos, version);
    strings.dump(verb);
  }

  // read OCAD9 fname field
  if (version>8 && header.fname_pos>0){
    ocad_seek(F, header.fname_pos, "reading fname");

    std::string s((size_t)header.fname_size, ' ');
    ocad_read(F, (void *)s.data(), s.size(), "fname");
    fname = iconv_from_win(s);

    if (verb>0){
      std::cout << "fname: " << fname << "\n";
    }
  }

  if (version <9) {
    // read setup/info -- TODO
    //setup_pos = h.setup_pos;
    //setup_size = h.setup_size;
    //info_pos = h.info_pos;
    //info_size = h.info_size;
  }
}

void
ocad_file::write (const char * fn) const{
  // open file, seek to the header end
  FILE * F = fopen(fn, "w");
  if (!F) throw Err() << "OCAD: can't open file: " << strerror(errno);
  ocad_seek(F, sizeof(ocad_header_struct), "writing");

  // create header, start filling it
  ocad_header_struct header;
  header.version = version;
  header.subversion = subversion;
  if (version==6) header.ftype=0;
  if (version==7) header.ftype=7;
  if (version==8) header.ftype = (type==0)? 2:3;
  if (version==9) header.ftype = (type==0)? 0:1;

  // OCAD8 end of header
  if (version<9){
    ocad8_head_add shead;
    for (const auto s: strings){
      if (s.type == 9) {
        if (shead.ncolors<256) shead.cols[shead.ncolors++].from_string(s);
        else std::cerr << "OCAD: warning: too many colors\n";
      }
      if (s.type == 10) {
        if (shead.ncolsep<MAX_COLSEP) shead.seps[shead.ncolsep++].from_string(s);
        else std::cerr << "OCAD: warning: too many colsep\n";
      }
    }
    ocad_write(F, &shead, sizeof(shead), "index shead");
  }

  if (version>8){
    ocad9_objects o9;
    o9.insert(o9.end(), objects.begin(), objects.end());
    header.obj_pos = o9.write(F, version);
  }
  else{
    ocad8_objects o8;
    o8.insert(o8.end(), objects.begin(), objects.end());
    header.obj_pos = o8.write(F, version);
  }

  // write strings
  if (version>7) header.str_pos = strings.write(F, version);

  // write symbols
  if (version>8){
    ocad9_symbols s9;
    for (const auto & s:symbols) s9.push_back(s.second);
    header.sym_pos = s9.write(F, version);
  }
  else{
    ocad8_symbols s8;
    for (const auto & s:symbols) s8.push_back(s.second);
    header.sym_pos = s8.write(F, version);
  }

  // OCAD9: write fname
  if (version>8){
    string s = iconv_to_win(fname);
    header.fname_pos = ftell(F);
    header.fname_size = s.size();
    ocad_write(F, (void *)s.data(), s.size(), "fname");
  }

  // TODO -- write setup + info
  if (version<9){
    // header.setup_pos =
    // header.setup_size =
    // header.info_pos =
    // header.info_size =
  }

  // write header
  ocad_seek(F, 0, "writing header");
  ocad_write(F, &header, sizeof(header), "header");

}

void
ocad_file::update_extents(){
  ocad_objects::iterator o;
  for (o=objects.begin(); o!=objects.end(); o++){

    map<int,ocad_symbol>::const_iterator e = symbols.find(o->sym);
    if (e!=symbols.end()){
      o->extent = e->second.extent;
      o->type = e->second.type;
    }
    else{
      o->extent = 0;
      o->type   = 0;
      cerr << "warning: no symbol: "
                 << o->sym << "\n";
    }
  }
}

int
ocad_file::add_object(int sym, iLine pts, double ang,
            const std::string & text, int cl){

  ocad_object o;

  map<int,ocad_symbol>::const_iterator e = symbols.find(sym);
  if (e!=symbols.end()){ // it is not an error!
    o.sym = sym;
    o.type   = e->second.type;
    o.extent = e->second.extent;
  }
  else{
    o.sym = -1;
    if ((cl <0) || (cl>3)){
      std::cerr << "warning: wrong class " << cl
                << "passed to ocad_file::add_object(). Fixed to line.\n";
      cl=2;
    }
    o.type = cl + 1; // 1-pt, 2-line, 3-area
  }

  o.set_coords(pts);
  o.ang = int(ang*10);
  o.text = text;
  objects.push_back(o);

  return 0;
}

iRect
ocad_file::range() const{
  iRect ret;
  for (const auto & o: objects){
    ret.expand(o.range());
  }
  return ret;
}

} // namespace
