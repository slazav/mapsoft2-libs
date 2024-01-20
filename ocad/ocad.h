#ifndef OCAD_FILE_H
#define OCAD_FILE_H

#include <cstdio>
#include <map>
#include <algorithm>
#include <iostream>

#include "geom/line.h"
#include "geom/rect.h"
#include "opt/opt.h"

#include "ocad_types.h"

/********************************************************************/
/// OCAD8/9 parameter string. It can contain many
/// various information (setup, colors, geo-reference etc.).
/// Each string contains type, object (object number starting with 1, or 0),
/// and code(char)+value pairs, separated by tab character '\t'.

/// Here I just keep parameter string data as a binary blob.
/// If needed it can be parsed or assembled separately for each parameter type.

struct ocad_parstr : Opt {
  ocad_long type; // parameter type
  ocad_long obj;  // object number 1.. or 0 for no object
  ocad_parstr(): type(0), obj(0) {}

  // construct from a string
  ocad_parstr(const ocad_long type, const ocad_long obj,
              const std::string & str): type(type), obj(obj) {
     Opt::operator=(unpack_data(str));
  }

  // Pack a data string from options (empty key and 1-char keys are used)
  static std::string pack_data(const Opt & opts);

  // Unpack a parameter string data
  static Opt unpack_data(const std::string & str);

  static std::vector<ocad_parstr> read(FILE * F,  const size_t addr, const int verb);

  static size_t write(FILE * F, const std::vector<ocad_parstr> & parstrs);

};

/********************************************************************/
/// This is common structure for all ocad symbols.
/// Now symbols are kept as blobs which are not
/// compatable for different versions.

struct ocad_symbol{

  ocad_long sym;    // symbol number
  ocad_byte type;   // symbol type
  ocad_long extent; // extent size

  std::string desc; // symbol description

  ocad_long version;
  std::string blob;

  /// Empty constructor.
  ocad_symbol():
    sym(0), type(0), extent(0), version(0){}
};

/********************************************************************/

struct ocad_object{

  ocad_long sym; // the symbol number (0 for deleted objects)
                 // -3 = image object eg AI object
                 // -2 = graphic object
                 // -1 = imported, no symbol assigned or symbol number
                 // ocad_short type in OCAD6..8

  ocad_byte type;  // 1: point
                   // 2: line
                   // 3: area
                   // 4: unformatted text
                   // 5: formatted text
                   // 6: line text
                   // 7: rectangle

  ocad_byte status;// 0: deleted
                   // 1: normal
                   // 2: hidden
                   // 3: deleted for undo

  ocad_small col;  // (OCAD9)
                   // symbolized objects: color number
                   // graphic object: color number
                   // image object: CYMK color of object

  ocad_small ang;  // Angle, unit is 0.1 degrees (for points, area with structure,.
                   // text, rectangles)

  ocad_byte viewtype; // (OCAD9)
                   // 0: normal
                   // 1: course setting object
                   // 2: modified preview object
                   // 3: unmodified preview object
                   // 4: temporary object (symbol editor or control description)
                   // 10: DXF import, GPS import

  ocad_small implayer; //  Layer number of imported objects (OCAD9)

  // TODO: color, width, flags

  std::vector<ocad_coord> coords;
  std::string text;

  ocad_object(): sym(0), type(1), status(1), col(0), ang(0),
                 viewtype(0), implayer(0){}

  /// Convert to iLine.
  /// TODO -- correct holes processing!
  iLine line() const;

  /// Set points.
  /// For creating objects use ocad_file::add_object()!
  void set_coords(const iLine & l);

  /// Get range (without symbol size added).
  iRect range() const;

};

/********************************************************************/


/// It is a structure with all passible data blocks.
/// It can be read or write from files with
/// different versions. It is not possible to
/// read and write different versions yet
/// (the main problem is different symbol format)

struct ocad{

  // all possible data blocks
  int version, subversion, type;
  std::vector<ocad_object> objects;
  std::vector<ocad_parstr> parstrs;
  std::vector<ocad_symbol> symbols;
  std::string     fname;      // v 9
  std::string     setup_blob; // v 6 7 8
  std::string     info_blob;  // v 6 7 8
  std::string     colinfo_blob;  // v 6 7 8

  // read ocad 6,7,8,9, cast to ocad 9 structures -- todo
  // verb = 0 - silent
  // verb = 1 - some info
  // verb = 2 - dump all known data from file
  void read(const char * fn, int verb=0);

  /// Write to file.
  void write (const char * fn) const;

  // find a symbol
  std::vector<ocad_symbol>::const_iterator
    find_symbol(int sym) const;

  /// Add new object.
  /// Class parameter is used only for unknown symbols,
  /// 0 - POI, 1 - POLYLINE, 2 - POLYGON, as in vmap
  int add_object(int sym, iLine pts, double ang=0,
                 const std::string & text = std::string(),
                 int cl=1);

  // coordinate range of an objects (with symbol size added)
  iRect object_range(const ocad_object & o) const;

  // coordinate range of all objects (with symbol sizes)
  iRect range() const;

};

#endif
