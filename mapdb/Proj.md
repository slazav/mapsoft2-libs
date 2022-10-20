## MapDB project

(see `mapdb/mapdb.h`)

Vector Map Project - a text file with links to all map components:

- object collections (pages), vmap- mp-, mapdb- files,
- map border, track in GPX or other format,
- type declarations, human-readable text file,
- render type declarations, human-readable text file,

#### file structure

MapDB project is a standard configuration file read by
mapsoft2/read_words library. Comments and basic quating is supported.
File consists of a number of lines:

Commands:

- `define <key> <value>`    -- define a variable
- `map_name <name>`         -- set map name
- `border_file <file name>` -- read border from a track file
- `page_nom <file name> [<name>]` -- page with a soviet map name
- `page_box <file name> <box> [<name>]` -- page with a soviet map name
- `render_conf <file name>` -- render configuration file
- `types_conf <file name>`  -- types configuration file

#### operations with MapDB projects

...


## Object collections

Objects can be stored in a few formats:

- MP - a standard Polish MP format. See `mp/`.

- FIG - xfig format (with mapsoft geo reference). Used for editing maps.
  See `fig/`, `fig_geo`, `fig_opt`.

- VMAP - format of mapsoft1. See `vmap/`. Some fields in the header (map
  scale, map name, etc.) are not used, all this information should be in
  the main project file.

- MAPDB - format of mapsoft2 (see `mapdb/mapdb_storage*.h`). Objects are
  stored in a database with spatial indexing. There is BerkleyDB and
  in-memory (STL) variants.

- OCAD - it was partially supported in mapsoft1, no plans for mapsoft2 yet.

My plan is to use MAPDB in-memory storage internally and have possibility
to use any format (except OCAD) for importing/exporting objects. To keep
maps on github I will still use VMAP (modified?)


## Type configuration

(see `mapdb/mapdb_types.h`)

A text file with information about object types.

