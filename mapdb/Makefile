MOD_HEADERS := db_tools.h db_geohash.h db_simple.h\
               mapdb_obj.h mapdb_storage.h mapdb_storage_mem.h mapdb_storage_bdb.h\
               mapdb_types.h mapdb.h\
               gobj_mapdb.h conv_label.h\


MOD_SOURCES := db_tools.cpp db_geohash.cpp db_simple.cpp string_pack.cpp\
               mapdb_obj.cpp mapdb_storage_mem.cpp mapdb_storage_bdb.cpp\
               mapdb_types.cpp mapdb.cpp \
               gobj_mapdb.cpp conv_label.cpp\

#               mapdb_vmap.cpp mapdb_mp.cpp\


SIMPLE_TESTS := db_geohash db_simple string_pack\
                mapdb_storage_bdb mapdb_obj
PKG_CONFIG = 'libdb'
CXXFLAGS = -g

include ../Makefile.inc
