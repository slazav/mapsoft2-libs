#ifndef OCAD_TYPES_H
#define OCAD_TYPES_H

// OCAD types and low-level structures

#include <string>
#include <stdint.h>
#include <cstring>
#include "iconv/iconv.h"
#include "geom/point.h"

/// convert pascal-string to c-string
/// maxlen includes first byte;
/// NOT USED?
void str_pas2c(char * str, int maxlen);

/// convert pascal-string to std::string
/// maxlen includes first byte;
std::string str_pas2str(const char * str, int maxlen);

/// convert std::string to pascal-string
///  maxlen includes first byte
void str_str2pas(char * pas, const std::string & str, size_t maxlen);

extern const IConv iconv_from_uni;
extern const IConv iconv_from_win;
extern const IConv iconv_to_uni;
extern const IConv iconv_to_win;

/********************************************************************/
/// OCAD6-7-8-9 data types

typedef int32_t  ocad_long;
typedef int32_t  ocad_int;
typedef int16_t  ocad_small;
typedef uint16_t ocad_word;
typedef uint16_t ocad_bool;
typedef uint8_t  ocad_byte;
typedef double   ocad_double;

struct ocad_coord{ // 8 bytes
  ocad_long x;
  ocad_long y;

  ocad_coord(): x(0), y(0) {}
  ocad_coord(const iPoint & p): x(p.x<<8), y(p.y<<8) {}
  int getx() const {return x>>8;}
  int gety() const {return y>>8;}
  int getf() const {return ((x & 0xFF) << 8) + (y & 0xFF);}
  void setx(int v) {x = (x & 0xFF) + (v << 8);}
  void sety(int v) {y = (y & 0xFF) + (v << 8);}

  /// this point is the first curve point
  bool is_curve_f() const {return x & 1;}

  /// this point is the second curve point
  bool is_curve_s() const {return x & 2;}

  /// this point is curve point
  bool is_curve() const {return x & 3;}

/*
  /// for double lines: there is no left line between this point and the next point
  bool is_no_left() const {return fx & 4;}

  /// v9: this point is a area border line gap (?)
  bool v9_is_ablg() const {return fx & 8;}

  /// this point is a corner point
  bool is_corner() const {return fy & 1;}

  /// this point is the first point of a hole in an area
  bool is_hole_f() const {return fy & 2;}

  /// for double lines: there is no right line between this point and the next point
  bool is_no_right() const {return fy & 4;}

  /// OCAD 7, OCAD 9: this point is a dash point (and 8?)
  bool v79_is_dash() const {return fy & 8;}
*/
  void dump(std::ostream & s) const;
};
/********************************************************************/

struct ocad_cmyk_{ // 4 bytes
  ocad_byte C,M,Y,K;   // 0-200?

  /// constructor: zero values
  ocad_cmyk_(): C(0),M(0),Y(0),K(0){}

  /// Get 0xCCMMYYKK.
  int get_int() const {return (C<<24) + (M<<16) + (Y<<8) + K;}

  /// Set from 0xCCMMYYBB.
  void set_int(int c);

  // get 0xRRGGBB -- INCORRECT?
  int get_rgb_int() const;

  // set from 0xRRGGBB -- INCORRECT?
  void set_rgb_int(int c);

  // dump cmyk
  void dump_hex(std::ostream & s) const;

  // dump rgb -- INCORRECT?
  void dump_rgb_hex(std::ostream & s) const;
};

struct ocad_freq_ang_{
  ocad_small f, a;
  ocad_freq_ang_(): f(0), a(0){}
};


#define MAX_COLSEP 24

struct ocad_color_{ // 72 bytes
  ocad_small num;        // this number is used in the symbols when referring a color
  ocad_small r1;
  ocad_cmyk_ color;       // color value
  char name[32];         // description of the color (pascal-string)
  ocad_byte sep_per[32]; // Definition how the color appears in the different.
                         // spot color separations: 0..200: 2 times the separation percentage
                         // 255: the color does not appear in the corresponding color separation

  /// Zero constructor.
  ocad_color_(): num(0), r1(0){
    memset(name, 0, sizeof(name));
    memset(sep_per, 0xFF, sizeof(sep_per));
  }

  /// Convert to ocad string (inaccurate yet)
//  ocad_parstr to_string() const;

  /// Set from ocad string (inaccurate yet)
//  void from_string(const ocad_parstr & s);

  void dump(std::ostream & s, int num_sep) const;
};

struct ocad_colsep_{ // 24 bytes
  char name[16];         // name of the color separation (pascal-string!)
  ocad_cmyk_ color;      // 0 in OCAD 6,
                         // CMYK value of the separation in OCAD 7.
                         // This value is only used in  the AI (Adobe Illustrator) export
  ocad_freq_ang_ raster;  // the halfton frequency and angle

  ocad_colsep_(){ memset(name, 0, sizeof(name)); }

  /// Convert to ocad string (inaccurate yet)
//  ocad_parstr to_string(int n) const;

  /// Set from ocad string (inaccurate yet)
//  void from_string(const ocad_parstr & s);

  // Dump color separation to s.
  void dump(std::ostream & s) const;
};

/********************************************************************/
// OCAD 6..9 header structure
struct ocad_header_{ // 48 bytes
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
  ocad_long  info_pos;    // OCAD 6/7/8: file position of the file information (0-term string up to 3
                          // OCAD 9: reserved
  ocad_long  info_size;   // OCAD 6/7/8: size (in bytes) of the file information
                          // OCAD 9: reserved
  ocad_long  str_pos;     // OCAD 8/9: file position of the first string index block
  ocad_long  fname_pos;   // OCAD 9: file position of file name, used for temporary files
                          //         and to recovery the file
  ocad_long  fname_size;  // OCAD 9: size of the file name, used for temporary files only
  ocad_long  r4;          // reserved
  ocad_header_(){
    memset(this, 0, sizeof(*this));
    ocad_mark=0x0CAD;
  }
  void dump(std::ostream & s, const int verb);
};

// Additional part of header for OCAD8 files.
// Information about colors and color separations.
// In OCAD9 all this data is kept in ocad_strings.
struct ocad8_colinfo_{ // 24 + 256*72 + 32*24 = 19224 bytes
  ocad_small ncolors;     // number of colors defined
  ocad_small ncolsep;     // number of color separations defined
  ocad_freq_ang_ C,M,Y,K; // halftone frequency and angle of CMYK colors
  ocad_small r1,r2;
  ocad_color_  cols[256];
  ocad_colsep_ seps[32]; /// only 24 used
  ocad8_colinfo_(){ memset(this, 0, sizeof(*this)); }
  void dump(std::ostream & s, const int verb);
};

/********************************************************************/
// OCAD has quite tricky structures for keeping objects, symbols,
// and parameter strings:
// File contains chain of index blocks, each with 256 index structures
// and position of the next block.
// Symbol/Object information is splitted into index structure and main
// part. Position of the main part is recorded in the index structure.

// Index block. Position of the next block + 256 index entries.
// Should work for OCAD8 and OCAD9 objects and symbols.

template <typename T>
struct ocad_index_block_{ // 4 + 256*sizeof(T)
  ocad_long next;     // file position of the next symbol block or 0
  T data[256];        // index data

  ocad_index_block_(): next(0){}
};

/// Parameter string index structure (version > 7)
struct ocad_parstr_index_{
  ocad_long pos;  // file position of string
  ocad_long len;  // length reserved for the string
  ocad_long type; // string typ number, if < 0 then deleted
  ocad_long obj;  // number of the object from 1 (or 0)
  ocad_parstr_index_(): pos(0), len(0), type(0), obj(0){}
};

// symbol index structure
struct ocad_symbol_index_{
  ocad_long pos;
  ocad_symbol_index_(): pos(0){}
};

/// OCAD8 object index structure.
struct ocad8_object_index_{
  ocad_coord lower_left;
  ocad_coord upper_right;
  ocad_long pos;  // file position
  ocad_word len;  // OCAD 6 and 7: size of the object in the file in bytes
                  // OCAD 8: number of coordinate pairs. size = 32+8*len
                  // this is reserved length, real length may be shorter
  ocad_small sym; // the symbol number (0 for deleted objects)
  ocad8_object_index_(){ memset(this, 0, sizeof(*this)); }
};

/// OCAD8 object structure
struct ocad8_object_{
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
  ocad8_object_(){ memset(this, 0, sizeof(*this)); }
};

// OCAD9 object index structure
struct ocad9_object_index_ {
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

  ocad9_object_index_(){memset(this, 0, sizeof(*this));}
};


/// OCAD9 object structure
struct ocad9_object_{
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
  ocad9_object_(){ memset(this, 0, sizeof(*this)); }
};

// OCAD8 base symbol structure.
// Someting wrong with this structure documentation
// (the tail is shifted by 1byte)
struct ocad8_base_symb_{
  ocad_small size;   // size of the symbol in bytes (depends on the type and the number of subsymbols
  ocad_small sym;    // symbol number (1010 for 101.0)
  ocad_small type;   // object type
                     // 1: point, 2: line or line text, 3: area, 4: text, 5: rectangle
  ocad_byte stype;   // symbol type
                     // 1: line text and text, 2: other
  ocad_byte flags;   // OCAD 6/7: must be 0
                     // OCAD 8: 1: not oriented to north 2: icon is compressed
  ocad_small extent; // how much the rendered symbols can reach outside the
                     // coordinates of an object with this symbol
  ocad_bool selected;// symbol is selected in the symbol box
  ocad_byte status;  // status of the symbol: 0: Normal, 1: Protected, 2: Hidden
  ocad_small tool;   // v7,v8 lines: Preferred drawing tool
                        //         0: off
                        //         1: Curve mode
                        //         2: Ellipse mode
                        //         3: Circle mode
                        //         4: Rectangular mode
                        //         5: Straight line mode
                        //         6: Freehand mode
  ocad_byte r2;
  ocad_long pos;     // file position, not used in the file, only when loaded in memory
  ocad_byte colors[32]; // used colors (256bit). color 0 - lowest bit of the first byte
  char desc[32];        // description of the symbol (c-string?!)
  ocad_byte icon[264];  // in v8 can by uncompressed (16-bit colors) or compressed (256 color palette

  ocad8_base_symb_(){ memset(this, 0, sizeof(*this)); }
} __attribute__((packed));


// OCAD9 base symbol structure.
// Someting wrong with this structure in documentation
// (the tail is shifted by 1byte).
struct ocad9_base_symb_{
  ocad_long size;   // size of the symbol in bytes (depends on the type and the number of subsymbols)
                    // Coordinates following the symbol are included.
  ocad_long sym;    // symbol number (101000 for 101.0, 101005 for 101.5, 101123 for 101.123)
  ocad_byte type;   // object type
                     // 1: point, 2: line, 3: area, 4: text
                     // 6: line text, 7: rectangle
  ocad_byte flags;   // 1: rotatable symbol (not oriented to north)
                     // 4: belongs to favorites
  ocad_bool selected;// symbol is selected in the symbol box
  ocad_byte status;  // status of the symbol: 0: Normal, 1: Protected, 2: Hidden
  ocad_byte tool;    // Preferred drawing tool
                        //         0: off
                        //         1: Curve mode
                        //         2: Ellipse mode
                        //         3: Circle mode
                        //         4: Rectangular mode
                        //         5: Straight line mode
                        //         6: Freehand mode
  ocad_byte csmode;  // Course setting mode:
                        //   0: Not used for course setting
                        //   1: course symbol
                        //   2: control description symbol
  ocad_byte sctype;  // Course setting object type
                       //   0: Start symbol (Point symbol)
                       //   1: Control symbol (Point symbol)
                       //   2: Finish symbol (Point symbol)
                       //   3: Marked route (Line symbol)
                       //   4: Control description symbol (Point symbol)
                       //   5: Course Titel (Text symbol)
                       //   6: Start Number (Text symbol)
                       //   7: Variant (Text symbol)
                       //   8: Text block (Text symbol)
//  ocad_byte csflags; // Course setting control description flags
                       //   a combination of the flags
                       //   32: available in column C
                       //   16: available in column D
                       //   8: available in column E
                       //   4: available in column F
                       //   2: available in column G
                       //   1: available in column H
  ocad_long extent;  // Extent how much the rendered symbols can reach outside the
                     // coordinates of an object with this symbol. For a point
                     // object it tells how far away from the coordinates of the
                     // object anything of the point symbol can appear.
  ocad_long pos;     // used internally
  ocad_small group;  // Group ID in the symbol tree. Lower and higher 8 bit are
                     // used for 2 different symbol trees.
  ocad_small ncolors; // Number of colors of the symbol max. 14 ///???
                      //   -1: the number of colors is > 14
  ocad_small colors[14]; // number of colors of the symbol
  char desc[32];        // description of the symbol (c-string?!)
  ocad_byte icon[484];  //  256 color palette (icon 22x22 pixels)

  ocad9_base_symb_(){ memset(this, 0, sizeof(*this)); }
} __attribute__((packed));

#endif
