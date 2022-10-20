#include "mapdb_storage_io.h"
#include "mapdb_storage_bdb.h"
#include "mapdb_storage_mem.h"
#include "mapdb_types.h"
#include "filename/filename.h"

#include "mp/mp.h"
#include "vmap/vmap.h"
#include "fig_geo/fig_geo.h"

void
ms2opt_add_mapdb_load(GetOptSet & opts){
  ms2opt_add_mp(opts);
  ms2opt_add_fig(opts);
}

void
ms2opt_add_mapdb_save(GetOptSet & opts){
  ms2opt_add_mp(opts);
  ms2opt_add_fig(opts);
}


std::shared_ptr<MapDBStorage>
mapdb_load(const std::string & fname, const MapDBTypeMap & types, const Opt & opts){

  if (file_ext_check(fname, ".mapdb"))
    return std::shared_ptr<MapDBStorage>(new MapDBStorageBDB(fname));

  std::shared_ptr<MapDBStorage> ret(new MapDBStorageMem);
  if (file_ext_check(fname, ".vmap")) {
    VMap vmap = read_vmap(fname);
    for (auto const & o:vmap){
//...
    }
    return ret;
  }

  if (file_ext_check(fname, ".mp")) {
    MP mp;
    read_mp(fname, mp, opts);
    for (auto const & o:mp){
//...
    }
    return ret;
  }

  if (file_ext_check(fname, ".fig")) {
    Fig fig;
    read_fig(fname, fig, opts);
//...
    return ret;
  }

  throw Err() << "Unknown format for vector map objects: " << fname;

}


/*********************************************************************/
// convert mapdb object to vmap
void
mapdb_to_vmap(const MapDBObj & o, const MapBDTypeInfo & t, VMap & vmap){
  VMapObj ret;

  // convert type to vmap format
  switch (o.type>>24){
    case MAPDB_POINT:   ret.type = (o.type & 0xFFFF); break;
    case MAPDB_LINE:    ret.type = (o.type & 0xFFFF) | 0x100000; break;
    case MAPDB_POLYGON: ret.type = (o.type & 0xFFFF) | 0x200000; break;
    case MAPDB_TEXT:    ret.type = (o.type & 0xFFFF) | 0x300000; break;
    default: throw Err() << "mapdb_to_vmap: unknown object class: " << (o.type>>24);
  }

  // name and comment
  ret.text = o.name;
  ret.comm = o.comm;

  // source
  // if (o.tags.size()>0) ret.opts.put("Source", *o.tags.begin());

  // angle (deg->deg)
  // In the old vmap format angle has a strange meaning: atan2(dlat,dlon)
  // (lonlat coordinates have different scales in horizontal and vertical dire
  // We want to convert the angle to real angle from north direction
  // We can do it only if we know coordinates (object has at least one point).
  // Direction of angle is also different (ccw instead of cw)
  if (!std::isnan(o.angle)){
    double a = -o.angle;
    if (o.size()>0 && o[0].size()>0){
      dPoint pt = o.bbox().cnt();
      a = 180/M_PI*atan2(cos(M_PI/180*pt.y)*sin(M_PI/180*a), cos(M_PI/180*a));
    }
    ret.opts.put("Angle", a);
  }

  // set coordinates
  ret.dMultiLine::operator=(o);

  // labels
/*
  for (auto const & li:o.children){
    auto l = get(li);
    VMapLab l1;
    if (l.get_class() != MAPDB_TEXT) continue;
    if (l.size()==0 || l[0].size()==0) continue;
    l1.pos = l[0][0];
    if (std::isnan(l.angle)){
      l1.hor = true;
      l1.ang = 0;
    }
    else {
      l1.hor = false;
      double a = -l.angle;
      l1.ang = 180/M_PI*atan2(cos(M_PI/180*l1.pos.y)*sin(M_PI/180*a), cos(M_PI/180*a));
    }
    switch (l.align) {
      case MAPDB_ALIGN_NW:
      case MAPDB_ALIGN_W:
      case MAPDB_ALIGN_SW:
        l1.dir = 0; break;
      case MAPDB_ALIGN_N:
      case MAPDB_ALIGN_C:
      case MAPDB_ALIGN_S:
        l1.dir = 1; break;
      case MAPDB_ALIGN_NE:
      case MAPDB_ALIGN_E:
      case MAPDB_ALIGN_SE:
        l1.dir = 2; break;
    }
    l1.fsize = 0; // todo: convert scale?

    // Convert scale:
    // In vmap format label scale is written
    // as a correction to font size: -1, 0, +1, etc.
    // Convert it to scaling factor using some
    // "natural" font size.
    if (l.scale!=1.0) l1.fsize = rint(8.0*(l.scale-1));

    ret.labels.push_back(l1);
  }
*/

  vmap.push_back(ret);
}

/*********************************************************************/
// convert mapdb object to mp
void
mapdb_to_mp(const MapDBObj & o, const MapBDTypeInfo & t, MP & mp){
  MPObj ret;
// ...
  mp.push_back(ret);
}

/*********************************************************************/
void
mapdb_save(MapDBStorage & storage, const std::string & fname, const MapDBTypeMap & types, const Opt & opts){

  // no need to save, it's same database
  if (storage.get_fname() == fname) return;

  // save to BDB database
  if (file_ext_check(fname, ".mapdb")){
    MapDBStorageBDB::delete_db(fname); // delete old

    // copy BDB database
    if (storage.get_fname()!=""){
      std::ifstream  src(storage.get_fname(), std::ios::binary);
      std::ofstream  dst(fname, std::ios::binary);
      dst << src.rdbuf();
      return;
    }
    // Copy from memory database to BDB
    else {
      MapDBStorageBDB new_storage(fname, true);
      storage.iter_start();
      while(!storage.iter_end()) {
        auto p = storage.iter_get_next();
        new_storage.put(p.first, p.second);
      }
      return;
    }
  }

  /************************************/
  // -> VMAP
  if (file_ext_check(fname, ".vmap")){
    VMap vmap;
    storage.iter_start();
    while(!storage.iter_end()) {
      auto p = storage.iter_get_next();
      auto t = types.find(p.second.type);
      if (t == types.end())
        throw Err() << "unknown type: " << MapDBObj::print_type(p.second.type);
      mapdb_to_vmap(p.second, t->second, vmap);
    }
    write_vmap(fname, vmap);
    return;
  }

  /************************************/
  // -> MP
  if (file_ext_check(fname, ".mp")){
    MP mp;
    storage.iter_start();
    while(!storage.iter_end()) {
      auto p = storage.iter_get_next();
      auto t = types.find(p.second.type);
      if (t == types.end())
        throw Err() << "unknown type: " << MapDBObj::print_type(p.second.type);
      mapdb_to_mp(p.second, t->second, mp);
    }
    write_mp(fname, mp, opts);
    return;
  }

  /************************************/
  // -> FIG
  if (file_ext_check(fname, ".fig")){
// ...
    storage.iter_start();
    while(!storage.iter_end()) {
      auto p = storage.iter_get_next();
// ...
    }
    return;
  }
  /************************************/

  throw Err() << "unknown file format: " << fname;
}

