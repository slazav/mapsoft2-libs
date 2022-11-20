#ifndef VMAP2IO_H
#define VMAP2IO_H

#include <string>
#include <vector>

#include "mp/mp.h"
#include "fig_geo/fig_geo.h"
#include "vmap/vmap.h"

#include "vmap2.h"
#include "vmap2obj.h"
#include "vmap2types.h"

/********************************************************************/
#include "getopt/getopt.h"

// add VMAP2I, VMAP2O, VMAP2IO, VMAP2T groups of options
void ms2opt_add_vmap2i(GetOptSet & opts);  // options for reading files
void ms2opt_add_vmap2o(GetOptSet & opts);  // options for writing files
void ms2opt_add_vmap2io(GetOptSet & opts); // common options for reading/writing
void ms2opt_add_vmap2t(GetOptSet & opts);  // --type option (does not used directly)

/********************************************************************/

// Read files (format is determined from file extension), add all
// objects to vmap2
void vmap2_import(const std::vector<std::string> & files,
                  const VMap2types & types,
                  VMap2 & vmap2,
                  const Opt & opts);

// Write vmap2 to a different format
void vmap2_export(VMap2 & vmap2, const VMap2types & types,
                 const std::string & ofile, const Opt & opts);

/********************************************************************/
// Decoding different formats.

void vmap_to_vmap2(const std::string & ifile, const VMap2types & types,
                   VMap2 & vmap2, const Opt & opts);

void fig_to_vmap2(const std::string & ifile, const VMap2types & types,
                  VMap2 & vmap2, const Opt & opts);

void mp_to_vmap2(const std::string & ifile, const VMap2types & types,
                 VMap2 & vmap2, const Opt & opts);


/********************************************************************/
// Encoding different formats.

// Convert vmap2 objects to vmap format and add to vmap storage.
// <types> parameter is not used.
void vmap2_to_vmap(VMap2 & vmap2, const VMap2types & types,
                   const std::string & ofile, const Opt & opts);

// Convert vmap2 objects to mp format and add to mp storage
// <types> parameter is optional, it's used for mp_start/mp_stop vaules.
void vmap2_to_mp(VMap2 & vmap2, const VMap2types & types,
                 const std::string & ofile, const Opt & opts);

// Convert vmap2 objects to fig format and add to Fig
void vmap2_to_fig(VMap2 & vmap2, const VMap2types & types,
                  const std::string & ofile, const Opt & opts);


#endif
