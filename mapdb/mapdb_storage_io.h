#ifndef MAPDB_STORAGE_IO_H
#define MAPDB_STORAGE_IO_H

#include <memory>
#include <string>
#include "mapdb_storage.h"
#include "mapdb_types.h"

/*********************************************************************/
// Loading/saving MapDBStorage.

// Open BDB database or create in-memory database and import objects from a file.
// File format is determined by its extension: .mp, .vmap, .fig, .mapdb
std::shared_ptr<MapDBStorage> mapdb_load(const std::string & fname, const MapDBTypeMap & types, const Opt & opts);

// Export storage to a file. File format is determined by its extension.
// Do nothing if it's BDB database with same name.
void mapdb_save(MapDBStorage & storage, const std::string & fname, const MapDBTypeMap & types, const Opt & opts);

#endif
