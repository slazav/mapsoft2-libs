## MapDB -- BerkleyDB-based storage  for vector maps with fast access and spatial indexing.


db_simple.h  -- DBSimple, a simple BerkleyDB database with integer key and string value
db_geohash.h -- BerkleyDB-based spatial indexing database (analog of
                GeoHashStorage from module/geohash).
db_tools.h   -- tools for BerkleyDB databases

mapdb_obj.h         -- map object
string_pack.h       -- tools for packing/unpacking mapdb_obj

mapdb_storage.h     -- MapDBStorage - interface class for MapDBObj collections
mapdb_storage_mem.h -- MapDBStorageMem - in-memory variant
mapdb_storage_bdb.h -- BerkleyDB variant

mapdb.h     -- MapDB class, map project with links to all components
mapdb_types.h

gobj_mapdb.h

conv_label.h
