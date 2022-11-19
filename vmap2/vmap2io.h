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

// add VMAP2IO group of options
void ms2opt_add_vmap2io(GetOptSet & opts);

/********************************************************************/

/* Convert vector map objects between different formats.
   Read map objects from a number of input files and save
   to the output file. Format is determined from file extension.
   Old content of the ouput file is deleted.
*/
void vmap2_convert(const std::vector<std::string> & ifiles, const Opt & opts);


/*
 Convert vector map objects between different formats.
  - Read output file (or open database)
  - Cleanup old data according to options
  - Read objects from all input files
  - Save result in the output file (or close database)
  - Input/Output format is determined from file extension

 Options (see ms2opt_add_vmap2io()):
  --out,-o -- output file, should be non-empty
  --types,-t -- type information file
  --headers -- Use non-object information from output file
               (mp headers, fig reference and objects, etc.),
               1 (default) or 0
  --old_objects -- Keep old objects, 0 (default) or 1
  --old_labels  -- Keep old labels,  0 (default) or 1
  --new_objects -- Copy new objects, 1 (default) or 0
  --new_labels  -- Copy new labels,  1 (default) or 0
*/
void vmap2_merge(const std::vector<std::string> & ifiles, const Opt & opts);

/********************************************************************/
// Decoding different formats.

// Read files (format is determined from file extension), add all
// objects to vmap2 file
void any_to_vmap2(const std::vector<std::string> & files, const VMap2types & types,
                 VMap2 & vmap2, const Opt & opts);

// Convert vmap objects to vmap2 format.
// <types> parameter is needed to calculate label types.
void vmap_to_vmap2(const VMap & vmap, const VMap2types & types, VMap2 & vmap2);

void fig_to_vmap2(const Fig & fig, const VMap2types & types, VMap2 & vmap2);

void mp_to_vmap2(const MP & mp, const VMap2types & types, VMap2 & vmap2);


/********************************************************************/
// Encoding different formats.
void vmap2_to_any(VMap2 & vmap2, const VMap2types & types,
                 const std::string & ofile, const Opt & opts);

// Convert vmap2 objects to vmap format and add to vmap storage.
// <types> parameter is not used.
void vmap2_to_vmap(VMap2 & vmap2, const VMap2types & types, VMap & vmap);

// Convert vmap2 objects to mp format and add to mp storage
// <types> parameter is optional, it's used for mp_start/mp_stop vaules.
void vmap2_to_mp(VMap2 & vmap2, const VMap2types & types, MP & mp);

// Convert vmap2 objects to fig format and add to Fig
void vmap2_to_fig(VMap2 & vmap2, const VMap2types & types, Fig & fig);


#endif
