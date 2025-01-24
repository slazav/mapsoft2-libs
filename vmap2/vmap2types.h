#ifndef VMAP2TYPES_H
#define VMAP2TYPES_H

#include <string>
#include <list>
#include <map>
#include "geo_data/geo_data.h"
#include "read_words/read_words.h"
#include "fig/fig.h"
#include "vmap2obj.h"

/********************************************************************/
#include "getopt/getopt.h"

void ms2opt_add_vmap2t(GetOptSet & opts);  // --type, --define options
/********************************************************************/


// Information about object type.
// Used for converting objects to different vector formats.
class VMap2type {
public:
  std::string group;     // type group
  std::string name;      // type short name (EN, default)
  std::string name_ru;   // type short name (RU)
  std::string name_fi;   // type short name (FI)
  std::string comm;      // type description
  std::string fig_mask;  // mask for FIG format
  Fig         fig_pic;   // picture for FIG format

  // information for mp/typ
  int mp_start, mp_end; // start/end level for MP format
  std::string typ_xpm;  // xpm (with \n as line separator)
  int typ_order;        // typ drawing order for polygons (1..8)
  int typ_line_width, typ_border_width; // typ line parameters (default 0)

  // Information for updating map labels
  int    label_type;              // type number for related labels, -1 for none
  float  label_def_scale;         // initial scale for labels
  VMap2objAlign  label_def_align; // initial align for labels
  dPoint label_def_mshift;        // initial shift in meters for labels
  int    label_maxnum;            // max number of labels (-1 for inf, -2 for auto).

  int    label_mkpt;    // reconnect labels to point objects, type number or -1 for none
  VMap2type():
    mp_start(0), mp_end(0), typ_order(1), typ_line_width(0), typ_border_width(0),
    label_type(-1), label_def_scale(1.0), label_def_align(VMAP2_ALIGN_SW),
    label_maxnum(-2), label_mkpt(-1) {
  }
};

// All object types
class VMap2types : public std::map<int, VMap2type> {
public:
  // Constuctor: read configuration file
  VMap2types(const Opt & o = Opt());

  // Write typ file.
  // - Codepage is used for converting names, it is also written to TYP header
  // - FID if written to the header if it is not -1.
  // Note that FID and codepage can be fixed after assembling img file with gmt -w -y.
  void write_typ(const std::string & fname, const int codepage, const int FID = -1) const;

private:
  // load from a file
  void load(const std::string & fname, read_words_defs & defs);
};

#endif
