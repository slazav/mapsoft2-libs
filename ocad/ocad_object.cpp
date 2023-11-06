#include "ocad_object.h"
#include "err/err.h"
#include <cassert>

using namespace std;

namespace ocad{

/********************************************************************/
// OCAD has quite tricky structures for keeping objects and symbols:
// File contains chain of index blocks, each with 256 index structures
// and position of the next block.
// Symbol/Object information is splitted into index structure and main
// part. Position of the main part is recorded in the index structure.

// Index block. Position of the next block + 256 index entries.
// Should work for OCAD8 and OCAD9 objects and symbols.

template <typename B>
struct ocad_index_block{ // 1028 bytes
  ocad_long next;     // file position of the next symbol block or 0
  B index[256];       // index data

  ocad_index_block(): next(0){
    assert(sizeof(*this) == 4+256*sizeof(B));
  }
};

/// OCAD8 object index structure.
struct ocad8_object_index{
  ocad_coord lower_left, upper_right;
  ocad_long pos;  // file position
  ocad_word len;  // OCAD 6 and 7: size of the object in the file in bytes
                  // OCAD 8: number of coordinate pairs. size = 32+8*len
                  // this is reserved length, real length may be shorter
  ocad_small sym; // the symbol number (0 for deleted objects)
  ocad8_object_index(){
    assert(sizeof(*this) == 24);
    memset(this, 0, sizeof(*this));
  }
};

/// OCAD8 object structure
struct ocad8_object{
  ocad_small sym;    // symbol number
  ocad_byte type;    // object type (1-pt, ...)
  ocad_byte unicode; // OCAD 6/7: must be 0, OCAD 8: 1 if the text is Unicode
  ocad_small n;      // number of coordinates
  ocad_small nt;     // number of coordinates used for text
  ocad_small ang;    // Angle, unit is 0.1 degrees (for points, area with structure,.
                     // text, rectangles)
  ocad_small r2;     // reserved
  ocad_long rh;      // reserved for height
  char res[16];
  ocad8_object(){
    assert(sizeof(*this) == 32);
    memset(this, 0, sizeof(*this));
  }
};

/// OCAD9 object index structure
struct ocad9_object_index {
  ocad_coord lower_left;
  ocad_coord upper_right;
  ocad_long pos;   // file position
  ocad_long len;   // number of coordinate pairs. size = 32+8*len
                   // this is reserved length, real length may be shorter
  ocad_long sym;   // the symbol number (0 for deleted objects)
                   // -3 = image object eg AI object
                   // -2 = graphic object
                   // -1 = imported, no symbol assigned or symbol number
  ocad_byte type;  // 1: point
                   // 2: line
                   // 3: area
                   // 4: unformatted text
                   // 5: formatted text
                   // 6: line text
                   // 7: rectangle
  ocad_byte r1;    // reserved
  ocad_byte status;// 0: deleted
                   // 1: normal
                   // 2: hidden
                   // 3: deleted for undo
  ocad_byte viewtype;
                   // 0: normal
                   // 1: course setting object
                   // 2: modified preview object
                   // 3: unmodified preview object
                   // 4: temporary object (symbol editor or control description)
                   // 10: DXF import, GPS import
  ocad_small col;  // symbolized objects: color number
                   // graphic object: color number
                   // image object: CYMK color of object
  ocad_small r2;   // reserved
  ocad_small implayer; //  Layer number of imported objects
  ocad_small r3;

  ocad9_object_index(){
    assert(sizeof(*this) == 40);
    memset(this, 0, sizeof(*this));
  }
};

/// OCAD9 object structure
struct ocad9_object{
  ocad_long  sym; // symbol number or typ
                  // image object= -3 (imported from ai or pdf, no symbol assigned)
                  //  graphic object = -2 (OCAD objects converted to graphics)
                  // imported object = -1 (imported, no symbol assigned)
  ocad_byte type; // object type (1-pt, ...)
  ocad_byte r1;   // reserved
  ocad_small ang; // Angle, unit is 0.1 degrees (for points, area with structure,.
                  // text, rectangles)
  ocad_long n;    // number of coordinates
  ocad_small nt;  // number of coordinates in the Poly array used for
                  // storing text in Unicode for (line text and text
                  // objects) if nText is > 0
                  // for all other objects it is 0
  ocad_small r2;  //
  ocad_long col;  // color number
  ocad_small width; // line width -- not used???
  ocad_small flags; // flages: LineStyle by lines -- not used???
  double r3, r4;
  ocad9_object(){
    assert(sizeof(*this) == 40);
    memset(this, 0, sizeof(*this));
  }
};

/********************************************************************/


ocad_object::ocad_object(): sym(0), type(3), status(1), viewtype(0),
  implayer(0), idx_col(-1), ang(0), col(0), width(0), flags(0),
  extent(0){}

// lib2d functions
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

  ret.expand((int)extent);
  return ret;
}

void
ocad_object::dump(int verb) const{
 if (verb<1) return;
  cout << "object: " << sym/1000 << "." << sym%1000
    << " type: " << (int)type
    << " col: " << idx_col << " - " << col;
  if (status!=1) cout << " status: " << (int)status;
  if (viewtype)  cout << " viewtype: " << (int)viewtype;
  if (implayer)  cout << " implayer: " << implayer;
  if (ang)       cout << " ang: " << ang/10.0;
  if (coords.size()) cout << " points: " << coords.size();
  if (text.length()) cout << " text: \""  << text << "\"";
  cout << "\n";
  if (verb<2) return;
  if (coords.size()){
    cout << "  coords: ";
    for (size_t i=0; i<coords.size(); i++) coords[i].dump(cout);
    cout << "\n";
  }
}



ocad_coord
ocad_object::LLC() const{
  if (coords.size() == 0) return ocad_coord();
  ocad_coord ret;
  ret.setx(coords[0].getx());
  ret.sety(coords[0].gety());
  for (size_t i=0; i<coords.size(); i++){
    if (coords[i].getx() < ret.getx()) ret.setx(coords[i].getx());
    if (coords[i].gety() < ret.gety()) ret.sety(coords[i].gety());
  }
  ret.setx(ret.getx()-extent);
  ret.sety(ret.gety()-extent);
  return ret;
}

ocad_coord
ocad_object::URC() const{
  if (coords.size() == 0) return ocad_coord();
  ocad_coord ret;
  ret.setx(coords[0].getx());
  ret.sety(coords[0].gety());
  for (size_t i=0; i<coords.size(); i++){
    if (coords[i].getx() > ret.getx()) ret.setx(coords[i].getx());
    if (coords[i].gety() > ret.gety()) ret.sety(coords[i].gety());
  }
  ret.setx(ret.getx()+extent);
  ret.sety(ret.gety()+extent);
  return ret;
}

int
ocad_object::txt_blocks(const string & txt) const{
  if (txt.length() == 0 ) return 0;
  return (txt.length()+2)/8+1;
}

void
ocad_object::write_text(const string & txt, FILE *F, int limit) const {
  size_t n=txt_blocks(txt);
  if (limit>=0) n=std::min((size_t)limit, n); // TODO: this can break unicode letters!
  if (n){
    char *buf = new char [n*8];
    memset(buf, 0, n*8);
    for (size_t i=0; i<txt.length(); i++) buf[i] = txt[i];
    if (fwrite(buf, 1, n*8, F)!=n*8)
      throw Err() << "can't write object text";
    delete [] buf;
  }
}

void
ocad_object::write_coords(FILE *F, int limit) const{
  size_t n = coords.size();
  if (limit>=0) n=std::min((size_t)limit, n);
  if (n){
    ocad_coord * buf = new ocad_coord[n];
    memset(buf, 0, n*8);
    for (size_t i=0; i<n; i++)  buf[i] = coords[i];
    if (fwrite(buf, sizeof(ocad_coord), n, F)!=n)
      throw Err() << "can't write object coordinates";
    delete [] buf;
  }
}

void
ocad_object::read_coords(FILE *F, size_t n){
  if (n){
    ocad_coord * buf = new ocad_coord[n];
    if (fread(buf, sizeof(ocad_coord), n, F)!=n)
      throw Err() << "can't read object coordinates";
    coords = vector<ocad_coord>(buf, buf+n);
    delete [] buf;
  }
}

void
ocad_object::read_text(FILE *F, size_t n){
  if (n){
    char *buf = new char [n*8];
    if (fread(buf, 1, n*8, F)!=n*8)
      throw Err() << "can't read object text";
    for (size_t i=0; i<n*8-1; i++){
      if ((buf[i]==0) && (buf[i+1]==0)) break; // 0x0000-terminated string
      text.push_back(buf[i]);
    }
    delete [] buf;
  }
}


} // namespace
