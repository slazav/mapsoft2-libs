#ifndef VMAP2TYPES_H
#define VMAP2TYPES_H

#include <string>
#include <list>
#include <map>
#include "geo_data/geo_data.h"
#include "read_words/read_words.h"
#include "fig/fig.h"

/********************************************************************/
#include "getopt/getopt.h"

void ms2opt_add_vmap2t(GetOptSet & opts);  // --type, --define options
/********************************************************************/


// Information about object type.
// Used for converting objects to different vector formats.
class VMap2type {
public:
  std::string name;     // type short name
  std::string comm;     // type description
  std::string fig_mask; // mask for FIG format
  Fig         fig_pic;  // picture for FIG format
  int mp_start, mp_end; // start/end level for MP format
  int    label_type;    // type number for related labels, -1 for none
  float  label_def_scale; // initial scale for labels
  int    label_mkpt;    // reconnact labels to point objects, type number or -1 for none
  VMap2type(): mp_start(0), mp_end(0), label_type(-1),
               label_def_scale(1.0), label_mkpt(-1) {}
};

// All object types
class VMap2types : public std::map<int, VMap2type> {
public:
  // Constuctor: read configuration file
  VMap2types(const Opt & o);

private:
  // load from a file
  void load(const std::string & fname, read_words_defs & defs);
};

#endif
