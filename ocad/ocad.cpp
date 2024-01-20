#include "ocad.h"
#include "ocad_types.h"
#include "err/err.h"
#include <iostream>

using namespace std;

/********************************************************************/
/// File operations with error handling

void
ocad_seek(FILE *F, const size_t pos, const char * str, bool rel=false){
  if (fseek(F, pos, rel? SEEK_CUR : SEEK_SET)!=0)
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

/********************************************************************/
std::string
ocad_parstr::pack_data(const Opt & opts){
  // Replace all tab and \0 characters with spaces
  Opt opts1(opts); // we can modify it
  for (auto & o:opts1){
    std::replace(o.second.begin(), o.second.end(), '\t', ' ');
    std::replace(o.second.begin(), o.second.end(), '\0', ' ');
  }
  std::ostringstream s;
  s << opts1.get("");
  for (auto & o:opts1)
    if (o.first.size()==1) s << '\t' << o.first << o.second;
  return s.str();
}

Opt
ocad_parstr::unpack_data(const std::string & str){
  Opt ret;
  std::string key="";
  std::string val="";
  for (auto const c : str){
    if (c=='\0') break;
    if (c=='\t') {
      if (val.size()) ret.emplace(key,val);
      val="";
      key="\t";
      continue;
    }
    if (key=="\t"){
      key=c;
      continue;
    }
    val+=c;
  }
  if (val.size()) ret.emplace(key,val);
  return ret;
}

std::vector<ocad_parstr>
ocad_parstr::read(FILE * F, const size_t addr, const int verb){
  std::vector<ocad_parstr> ret;
  auto next = addr;
  while (next){
    ocad_seek(F, next, "reading parameter string index");
    ocad_index_block_<ocad_parstr_index_> idx;
    ocad_read(F, &idx, sizeof(idx), "parameter string index");
    next = idx.next;
    for (size_t i=0; i<256; i++){
      if (idx.data[i].pos==0) continue;
      // read string data
      std::string data(idx.data[i].len, '\0');
      ocad_seek(F, idx.data[i].pos, "reading parameter string");
      ocad_read(F, (void*)data.data(), data.size(), "parameter string");
      ret.emplace_back(idx.data[i].type, idx.data[i].obj, data);

      // dump data
      if (verb>0){
        std::cout << "Parameter string:"
          << " type: "  << ret.rbegin()->type
          << " obj: "   << ret.rbegin()->obj
          << "\n";
        for (const auto & p: *ret.rbegin())
          std::cout << "  " << p.first << " " << p.second << "\n";
      }
    }
  }
  return ret;
}

size_t
ocad_parstr::write(FILE * F, const std::vector<ocad_parstr> & parstrs){
  auto ret = ftell(F);
  size_t num = parstrs.size(); // number of entries
  size_t num_blk = num/256+1; // number of blocks

  for (size_t iblk=0; iblk<num_blk; iblk++){
    size_t addr = ftell(F);
    ocad_index_block_<ocad_parstr_index_> idx_blk;
    ocad_seek(F, addr + sizeof(idx_blk), "writing parstr index");

    // write data, fill the index block
    for (size_t i=0; i<256; i++){
      size_t n = iblk*256 + i;
      if (n >= num) break;

      auto & str = parstrs[n];
      auto & idx = idx_blk.data[i];
      auto data = ocad_parstr::pack_data(str);
      idx.pos  = ftell(F);
      idx.len  = data.size();
      idx.type = str.type;
      idx.obj  = str.obj;
      ocad_write(F, (void*)data.data(), data.size(), "parstr");
    }
    size_t next = ftell(F);
    if (iblk < num_blk-1) idx_blk.next = next;
    ocad_seek(F, addr, "writing parstr index");
    ocad_write(F, &idx_blk, sizeof(idx_blk), "parstr index");
    ocad_seek(F, next, "writing parstr index");
  }
  return ret;
}

/********************************************************************/

iLine
ocad_object::line() const{
  iLine ret;
  for (int i=0; i<coords.size(); i++){
    iPoint p(coords[i].getx(),coords[i].gety());
    if (!coords[i].is_curve())
      ret.push_back(p);
  }
  return ret;
}

void
ocad_object::set_coords(const iLine & l){
  coords.clear();
  coords = vector<ocad_coord>(l.begin(), l.end());
}

iRect
ocad_object::range() const{
  iRect ret;
  for (const auto & c:coords)
    ret.expand(iPoint(c.getx(),c.gety()));
  return ret;
}

/********************************************************************/
void
ocad::read(const char * fn, int verb){

  // clear data
  *this = ocad();

  // open file
  FILE * F = fopen(fn, "r");
  if (!F) throw Err() << "OCAD: can't open file: " << strerror(errno);

  // read header
  ocad_header_ header;
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
  header.dump(std::cout, verb);

  // OCAD 6..8 -- read color info at the end of the header
  if (version<9){
    ocad8_colinfo_ colinfo;
    ocad_read(F, &colinfo, sizeof(colinfo), "color info");
    colinfo_blob = std::string((char *)&colinfo, (char *)&colinfo + sizeof(colinfo));
    colinfo.dump(std::cout, verb);
  }

  // read symbols
  {
    auto next = header.sym_pos;
    while (next){
      ocad_seek(F, next, "reading symbol index");
      ocad_index_block_<ocad_symbol_index_> oi;
      ocad_read(F, &oi, sizeof(oi), "object index");
      next = oi.next;
      for (size_t i=0; i<256; i++){
        if (oi.data[i].pos==0) continue;
        ocad_seek(F, oi.data[i].pos, "reading symbol");
        ocad_symbol s;
        s.version = version;
        size_t size; // blob size

        int pos = ftell(F);

        // OCAD 6..8
        if (version<9){
          ocad8_base_symb_ bs;
          if (fread(&bs, 1, sizeof(bs), F)!=sizeof(bs))
            throw Err() << "can't read symbol";
          size = bs.size;

          // convert symbol number to OCAD9 format XXXY -> XXX00Y
          s.sym = (bs.sym/10)*1000 + bs.sym%10;
          s.type = bs.type;
          // convert type:
          // 2: line or line text -> 2 or 6
          // 5: formatted text or rectangle -> 5 or 7
          if ((s.type==2)&&(bs.stype==1)) s.type = 6;
          if ((s.type==5)&&(bs.stype!=1)) s.type = 7;

          s.extent = bs.extent;
          s.desc = iconv_from_win(str_pas2str(bs.desc,32));

        }

        // OCAD 9
        if (version==9){
          ocad9_base_symb_ bs;
          if (fread(&bs, 1, sizeof(bs), F)!=sizeof(bs))
            throw Err() << "can't read symbol";
          size = bs.size;

          s.sym = bs.sym;
          s.type = bs.type;
          s.extent = bs.extent;
          s.desc = iconv_from_win(str_pas2str(bs.desc,32));
        }

        // read symbol blob
        ocad_seek(F, pos, "symbol blob");
        char *buf = new char [size];
        if (fread(buf, 1, size, F)!=size)
            throw Err() << "can't read symbol blob";
        s.blob = string(buf, buf+size);
        delete [] buf;

        symbols.push_back(s);

        // dump data
        if (verb>0){
          std::cout << "  Symbol:"
            << " sym: " << s.sym/1000 << "." << s.sym%1000
            << " type: " << (int)s.type
            << " extent: " << s.extent
            << " size: " << s.blob.size()
            << " \"" << s.desc
            << "\"\n";
          ;
        }
      }
    }
  }

  // read objects
  if (version<9){
    auto next = header.obj_pos;
    while (next){
      ocad_seek(F, next, "reading object index");
      ocad_index_block_<ocad8_object_index_> oi8;
      ocad_read(F, &oi8, sizeof(oi8), "object index");
      next = oi8.next;
      for (size_t i=0; i<256; i++){
        if (oi8.data[i].pos==0) continue;
        ocad_object obj;
        obj.sym = oi8.data[i].sym;

        size_t size = version<8 ? oi8.data[i].len : oi8.data[i].len*8 + 32;

        // read data
        ocad8_object_ o8;
        ocad_seek(F, oi8.data[i].pos, "reading object");
        ocad_read(F, &o8, sizeof(o8), "object");
        if (o8.sym!=obj.sym) std::cerr
          << "warning: different symbol in index and object: "
          << (int)o8.sym << " != " << (int)obj.sym << "\n";

        if (o8.unicode) {} // TODO: 0 in OCAD6..7, 1 in OCAD8
        obj.type = o8.type;
        obj.ang = o8.ang;

        // deleted object
        if (obj.sym == 0) obj.status=0;

        // convert to OCAD9  XXXY -> XXX00Y
        obj.sym = (obj.sym/10)*1000 + obj.sym%10;

        // convert type:
        // 2: line or line text -> 2 or 6
        // 5: formatted text or rectangle -> 5 or 7
        if ((obj.type==2)&&(o8.nt!=0)) obj.type = 6;
        if ((obj.type==5)&&(o8.nt==0)) obj.type = 7;

        // check length
        if (size != sizeof(o8) + (o8.n + o8.nt) * sizeof(ocad_coord))
          throw Err() << "OCAD: object length does not match coord number!"
                      << " size: " << size << " " << o8.n << " " << o8.nt;

        // coordinates
        if (o8.n){
          obj.coords = std::vector<ocad_coord>(o8.n);
          ocad_read(F, (void*)obj.coords.data(), o8.n*sizeof(ocad_coord), "coordinates");
        }
        // text
        if (o8.nt){
          obj.text = std::string('\0', o8.nt*8);
          ocad_read(F, (void*)obj.text.data(), o8.nt*8, "object text");
          auto n = obj.text.find('\0');
          if (n!=std::string::npos) obj.text.resize(n);
        }

        objects.push_back(obj);

        // dump data
        if (verb>0){
          std::cout << "  Object:"
            << " type: "  << (int)obj.type
            << " sym: "   << (int)obj.sym
            << " ang: "   << (int)obj.ang
            << " l-l: "   << oi8.data[i].lower_left.x << " " << oi8.data[i].lower_left.y
            << " u-r: "   << oi8.data[i].upper_right.x << " " << oi8.data[i].upper_right.y
            << " npts: "   << o8.n
          ;
          if (obj.text.size()) std::cout << " txt: "   << obj.text;
          std::cout << "\n";
          if (verb>1){
            for (const auto & c: obj.coords)
              std::cout << " [" << c.x << "," << c.y << "]";
            std::cout << "\n";
          }
        }
      }
    }
  }

  // read objects
  if (version==9){
    auto next = header.obj_pos;
    while (next){
      ocad_seek(F, next, "reading object index");
      ocad_index_block_<ocad9_object_index_> oi9;
      ocad_read(F, &oi9, sizeof(oi9), "object index");
      next = oi9.next;
      for (size_t i=0; i<256; i++){
        if (oi9.data[i].pos==0) continue;
        ocad_object obj;
        obj.sym         = oi9.data[i].sym;
        obj.type        = oi9.data[i].type;
        obj.status      = oi9.data[i].status;
        obj.viewtype    = oi9.data[i].viewtype;
        obj.col         = oi9.data[i].col;
        obj.implayer    = oi9.data[i].implayer;

        // read data
        ocad9_object_ o9;
        size_t size = oi9.data[i].len*sizeof(ocad_coord) + sizeof(o9);
        ocad_seek(F, oi9.data[i].pos, "reading object");
        ocad_read(F, &o9, sizeof(o9), "object");
        if (o9.sym!=obj.sym) std::cerr
          << "warning: different symbol in index and object: "
          << (int)o9.sym << " != " << (int)obj.sym << "\n";
        if (o9.type!=obj.type) std::cerr
          << "warning: different type in index and object: "
          << (int)o9.type << " != " << (int)obj.type << "\n";

        obj.ang = o9.ang;

        // check length
        if (size < sizeof(o9) + (o9.n + o9.nt) * sizeof(ocad_coord))
          throw Err() << "OCAD: object length does not match coord number!"
                      << " size: " << size << " " << o9.n << " " << o9.nt;

        // coordinates
        if (o9.n){
          obj.coords = std::vector<ocad_coord>(o9.n);
          ocad_read(F, (void*)obj.coords.data(), o9.n*sizeof(ocad_coord), "coordinates");
        }
        // text
        if (o9.nt){
          obj.text = std::string('\0', o9.nt*8);
          ocad_read(F, (void*)obj.text.data(), o9.nt*8, "object text");
          auto n = obj.text.find('\0');
          if (n!=std::string::npos) obj.text.resize(n);
        }
        ocad_seek(F, oi9.data[i].pos + size, "object");
        objects.push_back(obj);

        // dump data
        if (verb>0){
          std::cout << "  Object:"
            << " type: "  << (int)obj.type
            << " sym: "   << (int)obj.sym
            << " ang: "   << (int)obj.ang
            << " l-l: "   << oi9.data[i].lower_left.x << " " << oi9.data[i].lower_left.y
            << " u-r: "   << oi9.data[i].upper_right.x << " " << oi9.data[i].upper_right.y
            << " npts: "   << o9.n
          ;
          if (obj.text.size()) std::cout << " txt: "   << obj.text;
          std::cout << "\n";
          if (verb>1){
            for (const auto & c: obj.coords)
              std::cout << " [" << c.x << "," << c.y << "]";
            std::cout << "\n";
          }
        }
      }
    }
  }

  // read strings
  if (version>7 && header.str_pos)
    parstrs = ocad_parstr::read(F, header.str_pos, verb);

  // read OCAD9 fname field
  if (version>8 && header.fname_pos>0){
    ocad_seek(F, header.fname_pos, "reading fname");
    std::string s((size_t)header.fname_size, ' ');
    ocad_read(F, (void *)s.data(), s.size(), "fname");
    if (s.find('\0')!=std::string::npos) s.resize(s.find('\0'));
    fname = iconv_from_win(s);
    if (verb>0){
      std::cout << "fname: " << fname << "\n";
    }
  }

  // setup blob (OCAD 6,7,8)
  if (version <9 && header.setup_pos != 0) {
    ocad_seek(F, header.setup_pos, "reading setup blob");
    setup_blob = std::string((size_t)header.setup_size, ' ');
    ocad_read(F, (void *)setup_blob.data(), setup_blob.size(), "setup blob");
    if (verb>0) std::cout << "setup size: " << setup_blob.size() << "\n";
  }

  // info blob (OCAD 6,7,8)
  if (version <9 && header.info_pos != 0) {
    ocad_seek(F, header.info_pos, "reading info blob");
    info_blob = std::string((size_t)header.info_size, ' ');
    ocad_read(F, (void *)info_blob.data(), info_blob.size(), "info blob");
    if (verb>0) std::cout << "info size: " << info_blob.size() << "\n";
  }

  // We are converting files to v8 or v9
  if (version<8) version=8;

}

/********************************************************************/
void
ocad::write (const char * fn) const{
  // open file, seek to the header end
  FILE * F = fopen(fn, "w");
  if (!F) throw Err() << "OCAD: can't open file: " << strerror(errno);

  // supported versions 8 and 9
  if (version != 8 && version != 9)
    throw Err() << "OCAD: incorrect version: " << version;

  // create header, start filling it
  ocad_header_ header;
  ocad_seek(F, sizeof(header), "writing");
  header.version = version;
  header.subversion = subversion;
  if (version==8) header.ftype = (type==0)? 2:3;
  if (version==9) header.ftype = (type==0)? 0:1;

  // OCAD8 color info (par string types 9 and 10)
  if (version==8){
    ocad8_colinfo_ colinfo;
    if (colinfo_blob.size() == sizeof(colinfo))
      ocad_write(F, (void *)colinfo_blob.data(), colinfo_blob.size(), "color info");
  }

  // OCAD8: write setup blob
  if (version==8 && setup_blob.size()){
    header.setup_pos = ftell(F);
    header.setup_size = setup_blob.size();
    ocad_write(F, (void *)setup_blob.data(), setup_blob.size(), "setup blob");
  }

  // OCAD8: write info blob
  if (version==8 && info_blob.size()){
    header.info_pos = ftell(F);
    header.info_size = info_blob.size();
    ocad_write(F, (void *)info_blob.data(), info_blob.size(), "info blob");
  }

  // OCAD9: write fname
  if (version==9 && fname.size()){
    string s = iconv_to_win(fname);
    header.fname_pos = ftell(F);
    header.fname_size = s.size();
    ocad_write(F, (void *)s.data(), s.size(), "fname");
  }

  // parameter strings
  // TODO: for OCAD8 skip types 9 and 10 (color info is stored in the header)
  if (parstrs.size())
    header.str_pos = ocad_parstr::write(F, parstrs);

  // symbols
  if (symbols.size()>0){
    header.sym_pos = ftell(F);
    size_t num = symbols.size(); // number of entries
    size_t num_blk = num/256+1; // number of blocks

    for (size_t iblk=0; iblk<num_blk; iblk++){
      size_t addr = ftell(F);
      ocad_index_block_<ocad_symbol_index_> idx_blk;
      ocad_seek(F, addr + sizeof(idx_blk), "writing object index");

      // write data, fill the index block
      for (size_t i=0; i<256; i++){
        size_t n = iblk*256 + i;
        if (n >= num) break;

        auto & obj = symbols[n];
        auto & idx = idx_blk.data[i];
        idx.pos  = ftell(F);
        ocad_write(F, (void *)obj.blob.data(), obj.blob.size(), "symbol blob");
      }
      size_t next = ftell(F);
      if (iblk < num_blk-1) idx_blk.next = next;
      ocad_seek(F, addr, "writing symbol index");
      ocad_write(F, &idx_blk, sizeof(idx_blk), "symbol index");
      ocad_seek(F, next, "writing symbol index");
    }
  }

  // OCAD8 objects
  if (version==8){
    header.obj_pos = ftell(F);

    size_t num = objects.size(); // number of entries
    size_t num_blk = num/256+1; // number of blocks

    for (size_t iblk=0; iblk<num_blk; iblk++){
      size_t addr = ftell(F);
      ocad_index_block_<ocad8_object_index_> idx_blk;
      ocad_seek(F, addr + sizeof(idx_blk), "writing object index");

      // write data, fill the index block
      for (size_t i=0; i<256; i++){
        size_t n = iblk*256 + i;
        if (n >= num) break;

        auto & obj = objects[n];
        auto & idx = idx_blk.data[i];
        ocad8_object_ data;

        idx.pos  = ftell(F);

        // object range
        auto r = object_range(obj);
        idx.lower_left = r.tlc();
        idx.upper_right = r.brc();

        // convert sym from OCAD9  XXXZZY -> XXXY. this may couse collisions!
        // TODO - check symbol collisions before writing OCAD6..8 objects
        data.sym = idx.sym = (obj.sym/1000)*10 + obj.sym%10;

        // convert type:
        // line text 6 -> 2
        // rectangle 7 -> 5
        data.type=obj.type;
        if (obj.type == 6) data.type = 2;
        if (obj.type == 7) data.type = 5;

        data.type = obj.type;
        data.ang = obj.ang;

        // TODO: convert to unicode
        data.unicode = 1;
        data.n = obj.coords.size();
        data.nt = (obj.text.size()+7)/8;

        //  limits:
        //  OCAD 6,7
        //  n+nt <= 2000
        //  nt <= 1024 (8191 characters + terminating zero)
        //  Index.Len := 32 + 8 * n + 8 * nt;
        //  OCAD 8
        //  n + nt <= 32768
        //  nt <= 1024 (8191 characters + terminating zero)
        //  Index.Len := n+nt
        if (data.nt >1024){
          std::cerr << "warning: cutting too long text\n";
          data.nt = 1024;
        }
        if (data.nt + data.n > 32768){
          std::cerr << "warning: cutting too long coordinates\n";
          data.n = 32768-data.nt;
        }

        idx.len = data.n+data.nt;

        ocad_write(F, &data, sizeof(data), "object");
        for (size_t i = 0; i<data.n; i++)
          ocad_write(F, (void*)&obj.coords[i], 8, "object coordinates");

        auto str = obj.text;
        str.resize(data.nt*8);
        ocad_write(F, (void*)str.data(), str.size(), "object text");
      }
      size_t next = ftell(F);
      if (iblk < num_blk-1) idx_blk.next = next;
      ocad_seek(F, addr, "writing object index");
      ocad_write(F, &idx_blk, sizeof(idx_blk), "object index");
      ocad_seek(F, next, "writing object index");
    }
  }

  // OCAD9 objects
  if (version==9){
    header.obj_pos = ftell(F);

    size_t num = objects.size(); // number of entries
    size_t num_blk = num/256+1; // number of blocks

    for (size_t iblk=0; iblk<num_blk; iblk++){
      size_t addr = ftell(F);
      ocad_index_block_<ocad9_object_index_> idx_blk;
      ocad_seek(F, addr + sizeof(idx_blk), "writing object index");

      // write data, fill the index block
      for (size_t i=0; i<256; i++){
        size_t n = iblk*256 + i;
        if (n >= num) break;

        auto & obj = objects[n];
        auto & idx = idx_blk.data[i];
        ocad9_object_ data;

        idx.pos  = ftell(F);

        // object range
        auto r = object_range(obj);
        idx.lower_left = r.tlc();
        idx.upper_right = r.brc();

        data.sym = idx.sym = obj.sym;
        data.type = idx.type = obj.type;
        idx.status = obj.status;
        idx.viewtype = obj.viewtype;
        idx.implayer = obj.implayer;
        idx.col = obj.col;

        data.ang = obj.ang;

        data.n = obj.coords.size();
        data.nt = (obj.text.size()+7)/8;
        idx.len = data.n+data.nt;

        ocad_write(F, &data, sizeof(data), "object");
        for (size_t i = 0; i<data.n; i++)
          ocad_write(F, (void*)&obj.coords[i], 8, "object coordinates");

        auto str = obj.text;
        str.resize(data.nt*8);
        ocad_write(F, (void*)str.data(), str.size(), "object text");
      }
      size_t next = ftell(F);
      if (iblk < num_blk-1) idx_blk.next = next;
      ocad_seek(F, addr, "writing object index");
      ocad_write(F, &idx_blk, sizeof(idx_blk), "object index");
      ocad_seek(F, next, "writing object index");
    }
  }

  // write header
  ocad_seek(F, 0, "writing header");
  ocad_write(F, &header, sizeof(header), "header");
  fclose(F);
}

int
ocad::add_object(int sym, iLine pts, double ang,
            const std::string & text, int cl){
  ocad_object o;

  // known symbol?
  auto s = find_symbol(sym);

  // if existing symbol is used, use it's type
  if (s!=symbols.end()){
    o.sym = sym;
    o.type = s->type;
  }
  else{
    o.sym = -1;
    if ((cl <0) || (cl>3)){
      std::cerr << "warning: wrong class " << cl
                << "passed to ocad::add_object(). Fixed to line.\n";
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

std::vector<ocad_symbol>::const_iterator
ocad::find_symbol(int sym) const{
  std::vector<ocad_symbol>::const_iterator ret;
  for (ret = symbols.begin(); ret!=symbols.end(); ret++)
    if (ret->sym == sym) break;
  return ret;
}

iRect
ocad::object_range(const ocad_object & o) const{
  auto r = o.range();
  auto s = find_symbol(o.sym);
  if (s!=symbols.end()) r.expand(s->extent);
  return r;
}

iRect
ocad::range() const{
  iRect ret;
  for (const auto & o: objects)
    ret.expand(object_range(o));

  return ret;
}

